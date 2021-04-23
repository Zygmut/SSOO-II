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
    unsigned int ultimoBL, ptr, nbl;
    int nRangoBL, nivel_punteros, indice;
    int ptr_nivel[3];
    int indices[3];
    unsigned int bloques_punteros[3][NPUNTEROS];
    int liberados;

    if (inodo.tamEnBytesLog == 0){ //el inodo esta vacio
        return 0;
    }

    if (inodo.tamEnBytesLog % BLOCKSIZE == 0){ //encontramos el ultimo bloque logico
        ultimoBL = inodo.tamEnBytesLog/BLOCKSIZE - 1;
    } else {
        ultimoBL = inodo.tamEnBytesLog/BLOCKSIZE;
    }
    memset(bufAux_punteros, 0, BLOCKSIZE);
    ptr = 0;

    for (nbl = primerBL ; nbl <= ultimoBL ; nbl++){
        if ((nRangoBL = obtener_nRangoBL(&inodo, nbl, &ptr)) < 0) {
            return - 1;
        }
        nivel_punteros = nRangoBL;
        while ((ptr > 0) && (nivel_punteros > 0)){
            indice = obtener_indice(nbl, nivel_punteros);
            if((indice == 0) || (nbl = primerBL)){
                bread(ptr, bloque_punteros[nivel_punteros - 1]);//puede dar fallo por ptr
            }
            ptr_nivel[nivel_punteros -1] = ptr;
            indices[nivel_punteros - 1] = indice;
            ptr = bloques_punteros[nivel_punteros - 1][indice];
            nivel_punteros--;
        }

        if (ptr > 0){
            liberar_bloque(ptr); //puede dar fallo por ptr
            liberados++;
            if (nRangoBL == 0){
                inodo.punterosDirectos[nbl] = 0;
            } else {
                nivel_punteros = 1;
                while (nivel_punteros <= nRangoBL){
                    indice = indices[nivel_punteros - 1];
                    bloques_punteros[nivel_punteros -1][indice] = 0;
                    ptr = ptr_nivel[nivel_punteros - 1];
                    if (memcmp(bloque_punteros[nivel_punteros - 1],bufAux_punteros,BLOCKSIZE) == 0){
                        liberar_bloque(ptr);
                        liberados++;
                        //aqui dice que hay que mejorar el algoritmo para saltar bloques que no hace falta explorar
                        //pero ahora mismo ni zorra.
                        if(nivel_punteros = nRangoBL){
                            inodo.punterosIndirectos[nRangoBL - 1] = 0;
                        }
                        nivel_punteros++;
                    } else {
                        bwrite(ptr, bloques_punteros[nivel_punteros - 1]); //puede dar fallo por ptr
                        nivel_punteros = nRangoBL + 1;
                        //dice que aqui tenemos que salir del bucle porque
                        //no es necesario liberar los bloques de niveles superiores
                        //de los que cuelga
                    }
                }
            }
        }

    }
    return liberados;
}