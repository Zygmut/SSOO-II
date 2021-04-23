/* Esto va dentro de ficheros_basico.h */
int liberar_inodo(unsigned int ninodo);
int liberar_bloques_inodo(unsigned int primerBL, inodo_t inodo);

/* Esto va dentro del ficheros_basico.c */

/* modulo de antipetamiento porque me ponen nervioso los errores */
#include "../C files/ficheros_basico.h"
#include "../C files/ficheros_basico.c"
/* Fin del modulo antipetamiento, porque yo lo valgo */

int liberar_inodo(unsigned int ninodo){
    inodo_t inodo;

    leer_inodo(ninodo, &inodo);//leemos el inodo que se quiere liberar
    /* Liberamos el inodo en cuestión */
    inodo.numBloquesOcupados = inodo.numBloquesOcupados - liberar_bloques_inodo(0, inodo);
    inodo.tipo = 'l';
    inodo.tamEnBytesLog = 0;
    SB.posPrimerInodoLibre--; //dejamos como libre el inodo correspondiente en el superbloque
    SB.cantInodosLibres++;
    escribir_inodo(ninodo, inodo);
    bwrite(0, &SB);

    /* Notas para tener en cuenta:
        - El superbloque se lee cuando se invoca "leer_inodo", asi que en principio no hace falta
        hacerlo de nuevo en este método.
        - SB.posPrimerInodoLibre es una variable que se incrementa cuando se reserva un inodo, por lo que
        es lógico pensar que se decrementa cuando se libera algun inodo. En el pdf se comenta que se tiene
        que actualizar una lista enlazada SB.posPrimerInodoLibre, algo que no he visto en el código. Con esto
        quiero decir que puede ser un posible fallo.
    */
}

int liberar_bloques_inodo(unsigned int primerBL, inodo_t inodo){
    unsigned char bufAux_punteros[BLOCKSIZE]; //1024 bytes
    unsigned int bloque_punteros[NPUNTEROS]; //1024 bytes
    unsigned int ultimoBL, nivel_punteros, indice, ptr, nbl;

    if (inodo.tamEnBytesLog == 0){ //el inodo esta vacio
        return 0;
    }

    if (inodo.tamEnBytesLog % BLOCKSIZE == 0){
        ultimoBL = inodo.tamEnBytesLog/BLOCKSIZE - 1;
    } else {
        ultimoBL = inodo.tamEnBytesLog/BLOCKSIZE;
    }
    memset(bufAux_punteros, 0, BLOCKSIZE);
    ptr = 0;

    for (nbl = primerBL ; nbl <= ultimoBL ; nbl++){
        
    }
}