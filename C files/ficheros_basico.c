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
                // printf("Inodos [%d]: %d\n ", j, inodos[j].punterosDirectos[0]);
            }else{ //Estamos al final
                inodos[j].punterosDirectos[0] = UINT_MAX;
                lastInodo = 1;
                // printf("Ultimo inodo [%d]: %d\n ", j, inodos[j].punterosDirectos[0]);
                break;
            }
        }
        
        if(bwrite(i, inodos) == -1){
            return -1;
        }// No hace falta reiniciar el valor ya que el segundo for se encarga de "remplazar" cada inodo
        if(memset(inodos, '0', sizeof(inodo_t)) == NULL){
            return -1;
        }
    }
    return 0;

}