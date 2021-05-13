#include "directorios.h"

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo){
    int siDir = 0; //me encanta que no hayan booleanas en C, es muy triste 
                    //Alexa play end - Clowncore
    const char barra = '/';
    if(camino[0] == barra ){        
        
        for (int i = 1 ; (i < strlen(camino)) && (siDir==0); i++) { // Iteramos por todo el path
            if(camino[i] != barra){
                inicial[i-1] = camino[i];
            }else{
                siDir = 1;
                strcpy(final,camino+i);
            }
        }

        //si dir
        if(siDir == 1){
            strcpy(tipo, "d");
            return 1;
        } else { //no hay dir luego es un fichero
            //copiamos camino en inicial sin primera /
            strncpy(inicial, camino + 1, sizeof(char) * strlen(camino) - 1); //+1 para evitar la primera '/'
            strcpy(final, " ");
            strcpy(tipo, "f");
            return 0;
        }   
        
    }
    else{
        // fprintf(stderr, "ERROR en extraer_camino, no empieza por /");
        return ERROR_CAMINO_INCORRECTO;
    }

    return 0;
}
 int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos){
    //reservamos variables
    superbloque_t SB;
    struct entrada entrada;
    inodo_t inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];

    char tipo;
    int cant_entradas_inodo,num_entrada_inodo;
    bread(posSB, &SB);
    
    //camino_parcial es "/"
    if(camino_parcial[0] == '/' && strlen(camino_parcial) == 1){
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return 0;
    }

    if (extraer_camino(camino_parcial,inicial,final,&tipo) == -1){
        return ERROR_CAMINO_INCORRECTO;
    }
    fprintf(stderr, "[buscar_entrada() → inicial: %s, final: %s, reserva: %d] \n", inicial, final, reservar);

    leer_inodo(*p_inodo_dir,&inodo_dir);
    if((inodo_dir.permisos & 4) != 4){
        return ERROR_PERMISO_LECTURA;
    }

    //calculamos número de entradas a inodo
    struct entrada buf_entradas[BLOCKSIZE / sizeof(struct entrada)];
    memset(buf_entradas, 0, sizeof(buf_entradas)); //Init a 0's
    int b_leidos = 0;

    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    num_entrada_inodo = 0;
    
    if(cant_entradas_inodo > 0 ){
        b_leidos += mi_read_f(*p_inodo_dir, &buf_entradas, b_leidos, BLOCKSIZE);

        while((num_entrada_inodo < cant_entradas_inodo) && (strcmp(inicial,buf_entradas[num_entrada_inodo].nombre) != 0)){
            
            num_entrada_inodo++;
            if((num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))) == 0){
                b_leidos += mi_read_f(*p_inodo_dir,&buf_entradas,b_leidos,BLOCKSIZE);
            }
        }
    }
    
    if(((strcmp(inicial,buf_entradas[num_entrada_inodo].nombre)) != 0)){ //strcmp be? maybe !=?
    
        switch (reservar) {
            
        case 0: 
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
        case 1:
            if(inodo_dir.tipo == 'f'){
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;

            }

            if((inodo_dir.permisos & 2) != 2){
                return  ERROR_PERMISO_ESCRITURA;
            }else {
                strcpy(entrada.nombre,inicial);
                if(tipo == 'd'){
                    if(strcmp(final,"/") == 0){
                        entrada.ninodo = reservar_inodo('d',6);
                        fprintf(stderr, "[buscar_entrada() → reservado inodo: %d tipo 'd' con permisos %u para: %s]\n", entrada.ninodo, permisos, entrada.nombre);
                    }else{
                        // printf(" ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO (BUSCAR_ENTRADA, DIRECTORIOS.C)");
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                    
                }else{
                    entrada.ninodo = reservar_inodo('f',6);
                    fprintf(stderr, "[buscar_entrada() → reservado inodo: %d tipo 'f' con permisos %u para: %s]\n", entrada.ninodo, permisos, entrada.nombre);
                }

               
                 printf("[buscar_entrada()-> creada entrada: %s inodo: %d] \n", inicial, entrada.ninodo);
                if( mi_write_f(*p_inodo_dir, &entrada, inodo_dir.tamEnBytesLog,sizeof(struct entrada)) == -1){
                   
                    if(entrada.ninodo != -1){
                        fprintf(stderr, "[buscar_entrada() → liberado inodo %i, reservado a %s\n", num_entrada_inodo, inicial);
                        liberar_inodo(entrada.ninodo);
                    }

                    return EXIT_FAILURE;
                }
               
            
            }
        }
    
     }
    if ((strcmp(final, "/")) == 0){
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar=1)){
            // printf("ERROR_ENTRADA_YA_EXISTENTE, (BUSCAR_ENTRADA, DIRECTORIOS.C)");
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        *p_inodo = buf_entradas[num_entrada_inodo].ninodo;
        *p_entrada = num_entrada_inodo;
        return EXIT_SUCCESS;
    }
    else{
        *p_inodo_dir = buf_entradas[num_entrada_inodo].ninodo;
        return buscar_entrada(final,p_inodo_dir,p_inodo,p_entrada,reservar,permisos);
    }
}

void mostrar_error_buscar_entrada(int error) {
   // fprintf(stderr, "Error: %d\n", error);
   switch (error) {
   case -1: fprintf(stderr, "Error: Camino incorrecto.\n"); break;
   case -2: fprintf(stderr, "Error: Permiso denegado de lectura.\n"); break;
   case -3: fprintf(stderr, "Error: No existe el archivo o el directorio.\n"); break;
   case -4: fprintf(stderr, "Error: No existe algún directorio intermedio.\n"); break;
   case -5: fprintf(stderr, "Error: Permiso denegado de escritura.\n"); break;
   case -6: fprintf(stderr, "Error: El archivo ya existe.\n"); break;
   case -7: fprintf(stderr, "Error: No es un directorio.\n"); break;
   }
}
