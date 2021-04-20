#include "ficheros.h"
/*
 * Escribe el contenido procedente de un buffer de memoria, buf_original, de tamaño nbytes, en un fichero/directorio
 * 
 * Input:   ninodo             => Nº del inodo a leer
 *          *buf_original      => ---
            offset             => ---
            nbytes             => ---
 
 * Output:  cantidad de bytes escritos realmente
 * Using:   none 
 */ 
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){
    inodo_t inodo;
    int bytes_escritos = 0;
    leer_inodo(ninodo, &inodo);
    if ((inodo.permisos & 2) == 2){
        int primerBL = offset / BLOCKSIZE;
        int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
        int desp1 = offset % BLOCKSIZE;
        int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
        unsigned char *buf_bloque[BLOCKSIZE];
        int nbfisico = traducir_bloque_inodo(ninodo,primerBL,1);

        //cabe en un solo bloque
        if(primerBL == ultimoBL){
            if(bread(nbfisico,buf_bloque) == -1){
                fprintf(stderr, "Error while reading\n");
                return -1;
            }
            memcpy(buf_bloque + desp1, buf_original, nbytes);
            bytes_escritos += nbytes;
            if(bwrite(nbfisico,buf_bloque) == -1){
                fprintf(stderr, "Error while writing\n");
                return -1;
            }
        }
        //no cabe en un solo bloque
        else{
             //primer bloque lógico
            if(bread(nbfisico,buf_bloque) == -1){
               fprintf(stderr, "Error while reading\n");
               return -1;
            }
            memcpy(buf_bloque+desp1,buf_original,BLOCKSIZE-desp1);
            bytes_escritos+=BLOCKSIZE-desp1;
            if(bwrite(nbfisico,buf_bloque) == -1){
                fprintf(stderr, "Error while writing\n");
                return -1;
            }

            //bloques lógicos intermedios
            for(int i = primerBL+1; i != ultimoBL; i++){
                nbfisico = traducir_bloque_inodo(ninodo,i,1);
                bytes_escritos+=bwrite(nbfisico,buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE);
            }

            //último bloque lógico
            nbfisico = traducir_bloque_inodo(ninodo,ultimoBL,1);
            if(bread(nbfisico,buf_bloque) == -1){
               fprintf(stderr, "Error while reading\n");
               return -1;
            }
            memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);
            bytes_escritos += desp2 + 1;
            if(bwrite(nbfisico,buf_bloque) == -1){
                fprintf(stderr, "Error while writing\n");
                return -1;
            }

        }
        bytes_escritos += offset; // not really sure about this one thou
        //actualizamos inodo
        leer_inodo(ninodo,&inodo);
        if(inodo.tamEnBytesLog < bytes_escritos){
            inodo.tamEnBytesLog = bytes_escritos;
            inodo.ctime = time(NULL);
        }
        inodo.mtime = time(NULL);
        escribir_inodo(ninodo, inodo);
        return bytes_escritos;

    }else{
        fprintf(stderr, "Inodo[%d] doesn't have writing privileges\n", ninodo);
        return -1;
    }
}

/*
 * Lee información de un fichero/directorio correspondiente al nº de inodo, ninodo, pasado como argumento y la almacena en un buffer de memoria
 * 
 * Input:   ninodo          => Nº del inodo a leer
 *          *buf_original   => buffer de memoria 
 *          offset          => posición inicia de lectura con respecto al inodo (en bytes)
 *          nbytes          => numero de bytes a leer
 * Output:  0 uppon success, 1 otherwise
 * Using:   none 
 */ 
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes){
    inodo_t inodo;
    int bytes_leidos = 0;
    leer_inodo(ninodo, &inodo);
    
    if((inodo.permisos & 4) == 4 ){ //Check permisos de lectura inodo leido
        if(offset >= inodo.tamEnBytesLog){
            return bytes_leidos; // return 0;
        }
        if((offset + nbytes)>= inodo.tamEnBytesLog){
            bytes_leidos = inodo.tamEnBytesLog-offset;
        }

        int primerBL = offset / BLOCKSIZE;
        int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
        int desp1 = offset % BLOCKSIZE;
        int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
        unsigned char *buf_bloque[BLOCKSIZE];
        int nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        
        // Todo cabe en un solo bloque
        if(primerBL == ultimoBL){ 
            if(nbfisico != -1){ 
                if(bread(nbfisico, buf_bloque) == -1){
                    fprintf(stderr, "Error while reading\n");
                    return -1;
                }

                if(desp2 == 0){ // Estamos leyendo el ultimo bloque 
                    memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
                    bytes_leidos += BLOCKSIZE - desp1;
                }else{
                    memcpy(buf_original, buf_bloque + desp1, desp2 - desp1 + 1);
                    bytes_leidos += desp2 - desp1 + 1;
                }

            }else{ //No hay bloque fisiso reservado, acumulamos bytes
                bytes_leidos += BLOCKSIZE ;
            }
        }else{

            // Primer bloque
            desp2 = offset + nbytes - 1; //modificamos el desplazamiento
            
            if(nbfisico != -1){
                if(bread(nbfisico, buf_bloque) == -1){
                    fprintf(stderr, "Error while reading \n");
                    return -1;
                }

                memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1); // Leemos el primer bloque 
                bytes_leidos += BLOCKSIZE - desp1;
            }else{ // No esta el bloque reservado
                bytes_leidos += BLOCKSIZE;
            }

            // Bloques intermedios
            for(int i = primerBL + 1; i != ultimoBL; i ++){ 
                nbfisico = traducir_bloque_inodo(ninodo, i, 0);
                if(nbfisico != -1){
                    bytes_leidos += bread(nbfisico, buf_bloque); //Leemos el bloque y augmentamos la cantidad de bytes leidos
                    memcpy(buf_original, buf_bloque, BLOCKSIZE); // leemos todo el bloque 
                }else{
                    bytes_leidos += BLOCKSIZE;
                }
            }

            // Ultimo bloque
            desp2 = desp2 % BLOCKSIZE;
            nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 0);
            if(nbfisico != -1){  
                if(bread(nbfisico, buf_bloque) == -1){
                    fprintf(stderr, "Error while reading\n");
                    return -1;
                }
                memcpy (buf_original + (nbytes - desp2 - 1), buf_bloque, desp2 + 1);
                bytes_leidos += desp2 + 1;
            }else{ // No esta el bloque reservado
                bytes_leidos += BLOCKSIZE;
            }
        }
        
        inodo.atime = time(NULL);
        escribir_inodo(ninodo, inodo);
        return bytes_leidos;
        

    }else{
        fprintf(stderr, "Inodo[%d] doesn't have reading privileges\n", ninodo);
        return -1;
    }
}

/*
 * Modifica los permisos del inodo posicionado en ninodo
 * 
 * Input:   ninodo      => Nº del inodo a leer
 *          permisos    => nuevo permiso del inodo
 * Output:  0 uppon success, -1 otherwise
 * Using:   none 
 */ 
int mi_chmod_f(unsigned int ninodo, unsigned char permisos){
    inodo_t inodo;
    leer_inodo(ninodo, &inodo);
    inodo.permisos = permisos;
    inodo.ctime = time(NULL);
    escribir_inodo(ninodo, inodo);
    return 0;
}

/*
 * Devuelve la metainformación de un fichero/directorio (correspondiente al nº de inodo pasado como argumento)
 * 
 * Input:   ninodo      => Nº del inodo a leer
 *          *p_stat      => STAT
 * Output:  0 uppon success, -1 otherwise 
 * Using:   none 
 */ 
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat){
    inodo_t inodo;
    leer_inodo(ninodo, &inodo);
    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;

    p_stat->atime = inodo.atime;
    p_stat->ctime = inodo.ctime;
    p_stat->mtime = inodo.mtime;
    
    p_stat->nlinks = inodo.nlinks;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;

    return 0;
}