#include "ficheros_basico.h"

// Declaraciones de funciones
int printSB();
int printMB();
int printInodoList();
int printInodo(unsigned int ninodo);

struct tm *ts;
char atime [80];
char mtime [80];
char Ctime [80];


// Declaraciones de variables globales
superbloque_t SB;
inodo_t inodos [BLOCKSIZE/INODOSIZE];

int main(int argc, char **argsv){
    if(argc == 2){
        if(bmount(argsv[1]) == -1){
            fprintf(stderr, "Error while mounting\n");
            return -1;
        }
        
        printSB();
        // printInodoList();       
        printMB();

        // Reservar y liberar un bloque
        printf("RESERVA Y LIBERACION DE UN BLOQUE\n");
        int res = reservar_bloque();
       
        
        printf("nBloque reservado : %d\n", res);
        if(bread(posSB, &SB) == -1){
            fprintf(stderr, "Error while reading SB\n");
        return -1;
        }
        printf("Cantidad de bloques libres: %d\n", SB.cantBloquesLibres);

        printf("liberar el bloque [ %d ]: %d \n", res, liberar_bloque(res));
        
        if(bread(posSB, &SB) == -1){
            fprintf(stderr, "Error while reading SB\n");
        return -1;
        }
        printf("Cantidad de bloques libres: %d\n", SB.cantBloquesLibres);
        printf("\n");

        // Printeo de inodo raiz
        printf("DATOS DEL INODO RAIZ\n");
        printInodo(SB.posInodoRaiz); //por el culo te la hinco


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
    return 0;
}

int printSB(){
    if(bread(posSB, &SB) == -1){
        fprintf(stderr, "Error while reading\n");
        return -1;
    }

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

    return 0;
}

int printInodoList(){
    printf("RECORRIDO LISTA ENLAZADA DE INODOS LIBRES:\n");
    
    int indexInodos = SB.posPrimerInodoLibre + 1; // Cantidad de Inodos
    int lastInodo = 0 ; 
    for(int i = SB.posPrimerBloqueAI; (i <= SB.posUltimoBloqueAI) && (lastInodo == 0); i++){ //Recorrido de todos los inodos
        if(bread(i, inodos) == -1){
            fprintf(stderr, "Error while reading");
            return -1;
        }
        for(int j = 0; j < (BLOCKSIZE/INODOSIZE); j++){ //Recorrido de cada uno de los inodos
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

    return 0;
}

int printMB(){ // solo imprime el priemr y ultimo bit de cada bloque

    printf("VALORES DEL MB\n");
    for(int i = SB.posPrimerBloqueMB; i <= SB.posUltimoBloqueMB; i++){  
        leer_bit(i*BLOCKSIZE);  // Leer el primer bit 1024/8 -> 128  1024%8 -> 0
        printf("\n");
        leer_bit((i*BLOCKSIZE)+ BLOCKSIZE-1);
        printf("\n");
    }
    printf("\n");

    return 0;
}

int printInodo(unsigned int ninodo ){
    inodo_t inodo;
    
    leer_inodo(ninodo, &inodo);     // Mirar este & si peta
    
    ts = localtime(&inodo.atime);   // TOOO BAAAD!
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(Ctime, sizeof(Ctime),"%a %Y-%m-%d %H:%M:%S", ts);
    
    printf("id: %d\n", ninodo);
    printf("tipo: %c\n", inodo.tipo);
    printf("permisos: %i\n", inodo.permisos);
    printf("atime: %s\n", atime);
    printf("mtime: %s\n", mtime);
    printf("Ctime: %s\n", Ctime);
    printf("nlinks: %u\n", inodo.nlinks);
    printf("tamEnBytesLog: %u\n", inodo.tamEnBytesLog);
    printf("numBloquesOcupados: %u\n", inodo.numBloquesOcupados);
    return 0;
}