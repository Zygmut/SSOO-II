#include "ficheros_basico.h"

#define posSB 0
#define tamSB 1

superbloque_t SB;

/*
 * Calcular el tamaño del mapa de bits
 * 
 * Input:   ammount of blocks user wants at the MB
 * Output:  Actual ammount of blocks in MB
 */
int tamMB(unsigned int nbloques){
    if((nbloques/8)%BLOCKSIZE != 0){
        return ((nbloques/8)/BLOCKSIZE) + 1;
    }else{
        return ((nbloques/8)/BLOCKSIZE);
    }
}

/*
 * Calcular el tamaño del array de Inodos
 * 
 * Input:   ammount of blocks user wants at the AI
 * Output:  Actual ammount of blocks in AI
 */
int tamAI(unsigned int ninodos){
    if((ninodos*INODOSIZE)%BLOCKSIZE != 0){
        return ((ninodos*INODOSIZE)/BLOCKSIZE) + 1;
    }else{
        return ((ninodos*INODOSIZE)/BLOCKSIZE);
    }
}

/*
 * Inicializar el Super Bloque
 * 
 * Input:   nbloques -> MB blocks
 *          ninodos  -> Inode blocks
 * Output:  
 * Using:   open 
 */
int initSB(unsigned int nbloques, unsigned int ninodos){
    
    SB.posPrimerBloqueMB = posSB + tamSB;
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;

    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;

    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques - 1;

    SB.posInodoRaiz = 0;
    SB.posPrimerInodoLibre = 0;
    SB.cantBloquesLibres = nbloques;
    SB.cantInodosLibres = ninodos;

    SB.totBloques = nbloques;
    SB.totInodos = ninodos; 
    
    return bwrite(posSB, &SB);
}

/*
 * Inicializar el Mapa de Bits
 * 
 * Input:   none
 * Output:  0 if success. -1 otherwise
 * Using:   bwrite, memset
 */
int initMB(){
    unsigned char buffer[BLOCKSIZE];  
    if(memset(buffer,'0',sizeof(buffer)) == NULL){
        return -1;
    }

    for (int i = SB.posPrimerBloqueMB; i < SB.posUltimoBloqueMB; i++){
        if(bwrite(i, buffer) == -1){
            return -1;
        }
        if(memset(buffer,'0',sizeof(buffer)) == NULL){
            return -1;
        }
    }
    return 0;
}

/*
 * Inicializar el Bloque de Inodos
 * 
 * Input:   none
 * Output:  0 uppon success. -1 otherwise
 * Using:   none 
 */
int initAI(){
    inodo_t inodos [BLOCKSIZE/INODOSIZE];
    int indexInodos = SB.posPrimerInodoLibre + 1; // Cantidad de Inodos
    int lastInodo = 0 ;                               // Es un booleano pero no me deja declarar un booleano (no existen las booleanas en C, tienes que usar una libreria para eso)
    
    for(int i = SB.posPrimerBloqueAI; (i <= SB.posUltimoBloqueAI) && (lastInodo == 0); i++){ //Recorrido de todos los inodos
        
        for(int j = 0; j < (BLOCKSIZE/INODOSIZE); j++){ //Recorrido de cada uno de los inodos
            inodos[j].tipo = 'l'; //Tipo l: Libre

            if(indexInodos < SB.totInodos){
                inodos[j].punterosDirectos[0] = indexInodos; // Declaramos la conexion con el siguiente inodo
                indexInodos++;
            
            }else{ //Estamos al final
                inodos[j].punterosDirectos[0] = UINT_MAX;
                lastInodo = 1;
                break;
            }
        }
        
        if(bwrite(i, inodos) == -1){
            return -1;
        }   
        if(memset(inodos, '0', sizeof(inodo_t)) == NULL){
            return -1;
        }
    }
    return 0;

    /*
     * Escribe el valor indicado por el parámetro bit en un bit del MB determinado por nbloque 
     * 
     * Input:   nbloque     => posixión de escritura
     *          bit         => Valor a esribir
     * Output:  0 uppon success. -1 otherwise
     * Using:   none 
     */
    int escribir_bit(unsigned int nbloque, unsigned int bit){

        int posbyte = nbloque / 8;
        int posbit = nbloque % 8;
        int nbloqueMB = posbyte / BLOCKSIZE;
        int nbloqueabs =  SB.posPrimerBloqueMB + nbloqueMB;

        unsigned char bufferMB[BLOCKSIZE]; 
        posbyte = posbyte % BLOCKSIZE;
        unsigned char mascara = 128;

        mascara >>= posbit; //desplazamiento de bits a la derecha

        if(bread(nbloqueabs, bufferMB) == -1){ //leemos bloque
            return -1;
        }  

        if(bit == 1){
            bufferMB[posbyte] | = mascara; //operador OR
        }
        else{
            bufferMB[posbyte] & = ~mascara; //operador AND y NOT
        }

        if(bwrite(nbloqueabs, bufferMB) == -1){ //escribimos bloque
            return -1;
        }
   
        return 0;
    }

    /*
     * Lee el bit del MB determinado por nbloque 
     * 
     * Input:   nbloque     => Posición de lectura
     * Output:  Valor leido
     * Using:   none 
     */
    char leer_bit(unsigned int nbloque){
        int posbyte = nbloque / 8;
        int posbit = nbloque % 8;
        int nbloqueMB = posbyte / BLOCKSIZE;
        int nbloqueabs =  SB.posPrimerBloqueMB + nbloqueMB;
        
        unsigned char bufferMB[BLOCKSIZE]; 
        posbyte = posbyte % BLOCKSIZE;
        unsigned char mascara = 128;

        if(bread(nbloqueabs, bufferMB) == -1){ //leemos bloque
            return -1;
        }  

        mascara >>= posbit; //desplazamiento de bits a la derecha
        mascara &= bufferMB[posbyte]; //operador and para bits
        mascara >>= (7 - posbit);   //desplazamiento de bits a la derecha

        if(mascara == 0){
            return 0;
        }
        else{
            return 1;
        }

    }

    /*
     * Encuentra el primer bloque libre, consultando el MB, lo ocupa y devuelve su posición.
     * 
     * Input:   none
     * Output:  Posición de memoria reservada
     * Using:   none 
     */
    int reservar_bloque(){
        if(SB.cantInodosLibres == 0){
            return -1; // ---------------------------------supongo que seria -1 no?-----------------------------------------------------
        }

        bool foundNotOccupied= false;

        int posBloqueMB = SB.posPrimerBloqueMB;
        unsigned int nbloque;
        int posbyte = 0;
        int posbit = 0;

        unsigned char bufferMB[BLOCKSIZE]; 
        unsigned char auxBufferMB[BLOCKSIZE]; 

        unsigned char mascara = 128;

        memset(auxBufferMB,255,BLOCKSIZE);
        //localizamos bloque desocupado
        while(!foundNotOccupied){
            if(bread(posBloqueMB, bufferMB) == -1){ //leemos bloque
                return -1;
            } 
            if(memcmp(bufferMB,auxBufferMB,BLOCKSIZE) != 0){//vemos si el bloque esta desocupado
                foundNotOccupied = true;
                break;
            } 
            posBloqueMB++;
        }
        //localizamos byte desocupado
        for(int i = 0; i < BLOCKSIZE; i++ ){
            if(bufferMB[i] != 255){ //si el byte no esta a 1 (ocupado)
                posbyte = i;
                break;
            }
        }
        //localizamos el bit
        while(bufferMB[posbyte] & mascara){
            bufferMB <<= 1;
            posbit++;
        }

        nbloque = ((posBloqueMB - SB.posPrimerBloqueMB)*BLOCKSIZE+posbyte)*8+posbit;

        if(escribir_bit(nbloque, 1) == -1){ 
            return -1;
        }
        sb.cantInodosLibres--;
        memset(auxBufferMB,0,BLOCKSIZE);
        bwrite(posSB, auxBufferMB);//-----------------------------------not really sure about this one, penultimo circulo de la funcion 3), grabamos buffer de 0s.....------
        return nbloque;
    }

    /*
     * Libera un bloque determinado por nbloque
     * 
     * Input:   nbloque     => posicion de memoria a liberar
     * Output:  Nº del bloque liberado
     * Using:   none 
     */    
    int liberar_bloque(unsigned int nbloque){

        return NULL;
    }

    /*
     * Escribe el contenido de una variable de tipo inodo_t en un determinado inodo del array de inodos
     * 
     * Input:   ninodo      => Nº del inodo a modificar
     *          inodo       => Valor de escritura del nuevo inodo
     * Output:  0 uppon success, 1 otherwise
     * Using:   none 
     */  
    int escribir_inodo(unsigned int ninodo, inodo_t inodo){
    
        return NULL;
    }

    /*
     * Lee un determinado inodo del array de inodos para volcarlo en una variable de tipo struct inodo pasada por referencia.
     * 
     * Input:   ninodo      => Nº del inodo a leer
     *          *inodo      => Inodo en el que se volcara los valores leidos
     * Output:  0 uppon success, 1 otherwise
     * Using:   none 
     */ 
    int leer_inodo(unsigned int ninodo, inodo_t *inodo){

        return NULL;
    }

    /*
     * Encuentra el primer inodo libre, lo reserva, devuelve su número y actualiza la lista enlazada de inodos libres.
     * 
     * Input:   tipo      => Tipo de nodo a encontrar
     *          permisos  => Permisos del inodo
     * Output:  Nº del inodo encontrado
     * Using:   none 
     */ 
    int reservar_inodo(unsigned char tipo, unsigned char permisos){

        return NULL;
    }
}