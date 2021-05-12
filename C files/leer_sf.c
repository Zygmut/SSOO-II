// Jaume, Marc, Ruben
#include "directorios.h"

// Declaraciones de funciones
int printSB();
int printMB();
int printInodoList();
int printInodo(unsigned int ninodo);
void mostrar_buscar_entrada(char *camino, char reservar);
int test_lvl_7();

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
        // printMB();
        // block_test();
        // printInodo(SB.posInodoRaiz);

        // Test traducir_bloque_inodo()
        test_lvl_7();
        if(bumount() == -1){
        fprintf(stderr, "Error while unmounting\n");
        return -1;
    }
    }else{
        fprintf(stderr, "Not enough arguments\n");
    }
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

int printMB(){ // solo imprime el primer y ultimo bit de cada bloque 
    // Si se quiere ver este debug, es necesario descomentar las lineas de debug de leer_bit

    printf("VALORES DEL MB\n");
    for(int i = SB.posPrimerBloqueMB; i <= SB.posUltimoBloqueMB; i++){  
        leer_bit(i*BLOCKSIZE);  
        printf("\n");
        leer_bit((i*BLOCKSIZE)+ BLOCKSIZE-1);
        printf("\n");
    }
    printf("\n");

    return 0;
}

int printInodo(unsigned int ninodo){
    inodo_t inodo;
    printf("DATOS DEL INODO RESERVADO %d\n", ninodo);
    leer_inodo(ninodo, &inodo);     // Mirar este & si peta
    
    ts = localtime(&inodo.atime);   // TOOO BAAAD!
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(Ctime, sizeof(Ctime),"%a %Y-%m-%d %H:%M:%S", ts);
    
    printf("tipo: %c\n", inodo.tipo);
    printf("permisos: %i\n", inodo.permisos);
    printf("atime: %s\n", atime);
    printf("mtime: %s\n", mtime);
    printf("Ctime: %s\n", Ctime);
    printf("nlinks: %u\n", inodo.nlinks);
    printf("tamEnBytesLog: %u\n", inodo.tamEnBytesLog);
    printf("numBloquesOcupados: %u\n", inodo.numBloquesOcupados);
    printf("contenido punterosDirectos: ");
    for(int i = 0; i < 11; i++){
        printf("%d | ", inodos->punterosDirectos[i]);
    }
    printf("%d", inodos->punterosDirectos[12]);
    printf("\n");
    return 0;
}

int block_test(){
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
    return 0;
}

void mostrar_buscar_entrada(char *camino, char reservar){
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;

    printf("\ncamino: %s, reservar: %d\n", camino, reservar);

    if((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6)) < 0){
        mostrar_error_buscar_entrada(error);
    }
    printf("\n");


}

int test_lvl_7(){
    mostrar_buscar_entrada("pruebas/", 1); //ERROR_CAMINO_INCORRECTO
    mostrar_buscar_entrada("/pruebas/", 0); //ERROR_NO_EXISTE_ENTRADA_CONSULTA
    mostrar_buscar_entrada("/pruebas/docs/", 1); //ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO
    mostrar_buscar_entrada("/pruebas/", 1); // creamos /pruebas/
    mostrar_buscar_entrada("/pruebas/docs/", 1); //creamos /pruebas/docs/
    mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/docs/doc1/doc11", 1);  
    //ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO
    mostrar_buscar_entrada("/pruebas/", 1); //ERROR_ENTRADA_YA_EXISTENTE
    mostrar_buscar_entrada("/pruebas/docs/doc1", 0); //consultamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/casos/", 1); //creamos /pruebas/casos/
    mostrar_buscar_entrada("/pruebas/docs/doc2", 1); //creamos /pruebas/docs/doc2
    
    return 0;
}