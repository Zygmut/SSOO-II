#include "ficheros_basico.h"

int main(int argc, char **argsv){
    if(argc == 2){
        if(bmount(argsv[1]) == -1){
            fprintf(stderr, "Error while mounting\n");
            return -1;
        }
        superbloque_t SB;
        inodo_t inodos [BLOCKSIZE/INODOSIZE];
        
        bread(0, &SB); 
        printf("DATOS DEL SUPERBLOQUE\n");
        printf("posPrimerBloqueMB: %i\n",SB.posPrimerBloqueMB);
        printf("posUltimoBloqueMB: %i\n", SB.posUltimoBloqueMB);
        printf("posPrimerBloqueAI: %i\n",SB.posPrimerBloqueAI);
        printf("posUltimoBloqueAI: %i\n", SB.posUltimoBloqueAI);
        printf("posPrimerBloqueDatos: %i\n", SB.posPrimerBloqueDatos);
        printf("posUltimoBloqueDatos: %i\n", SB.posUltimoBloqueDatos);
        printf("posInodoRaíz: %i\n", SB.posInodoRaiz);
        printf("posPrimerInodoLibre: %i\n", SB.posPrimerInodoLibre);
        printf("cantBloquesLibres: %i\n", SB.cantBloquesLibres);
        printf("cantInodosLibres: %i\n", SB.cantInodosLibres);
        printf("totBloques: %i\n", SB.totBloques);
        printf("totInodos: %i\n\n", SB.totInodos);
        printf("sizeof struct superbloque: %li\n", sizeof(superbloque_t));
        printf("sizeof struct inodo: %li\n\n", sizeof(inodo_t));
        printf("RECORRIDO LISTA ENLAZADA DE INODOS LIBRES:\n");
        
        int indexInodos = SB.posPrimerInodoLibre + 1; // Cantidad de Inodos
        int lastInodo = 0 ; 
        for(int i = SB.posPrimerBloqueAI; (i <= SB.posUltimoBloqueAI) && (lastInodo == 0); i++){ //Recorrido de todos los inodos
            for(int j = 0; j < (BLOCKSIZE/INODOSIZE); j++){ //Recorrido de cada uno de los inodos
                inodos[j].tipo = 'l'; //Tipo l: Libre

                if(indexInodos < SB.totInodos){
                    inodos[j].punterosDirectos[0] = indexInodos; // Declaramos la conexion con el siguiente inodo
                    indexInodos++;
                    printf("%d ", inodos[j].punterosDirectos[0]);
                }else{ //Estamos al final
                    inodos[j].punterosDirectos[0] = UINT_MAX;
                    lastInodo = 1;
                    printf("%d ", inodos[j].punterosDirectos[0]);
                    break;
                }
            }
        }
        printf("\n");
        if(bumount() == -1){
            fprintf(stderr, "Error while unmounting\n");
            return -1;
        }
        
    }else{
        fprintf(stderr, "Not enough arguments\n");
    }
    //mecomes loshu evos osti a
    //putob ul lying
    //por honga
    //Adelaida nos hace hacer codigo poco eficiente
}