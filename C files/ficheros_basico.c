// Jaume, Marc, Ruben
#include "ficheros_basico.h"

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
    void *buffer[BLOCKSIZE];

    if(memset(buffer, 0, sizeof(buffer)) == NULL){
        fprintf(stderr, "Error while setting memory\n");
        return -1;
    }

    for (int i = SB.posPrimerBloqueMB; i <= SB.posUltimoBloqueMB; i++){
        if(bwrite(i, buffer) == -1){
            fprintf(stderr, "Error while writing\n");
            return -1;
        }

    }

    for(int i = 0; i < SB.posPrimerBloqueDatos; i++){
        if(reservar_bloque() == -1){
            fprintf(stderr,"Error while reserving a block\n");
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
 * Using:   bwrite, memset
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
            fprintf(stderr, "Error while writing\n");
            return -1;
        }   
        if(memset(inodos, '0', sizeof(inodo_t)*8) == NULL){
            fprintf(stderr, "Error while setting memory\n");
            return -1;
        }
    }
    return 0;
}

/*
 * Escribe el valor indicado por el parámetro bit en un bit del MB determinado por nbloque 
 * 
 * Input:   nbloque     => posixión de escritura
 *          bit         => Valor a esribir
 * Output:  0 uppon success. -1 otherwise
 * Using:   bread, bwrite
 */
int escribir_bit(unsigned int nbloque, unsigned int bit){
    int posbyte = (nbloque / 8) % BLOCKSIZE;
    int posbit = nbloque % 8;
    int nbloqueMB = (nbloque/8) / BLOCKSIZE;
    int nbloqueabs =  SB.posPrimerBloqueMB + nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE]; 
    unsigned char mascara = 128;
    mascara >>= posbit; // Desplazamiento de bits a la derecha
    
    if(bread(nbloqueabs, bufferMB) == -1){ // Leemos bloque
        fprintf(stderr, "Error while reading\n");
        return -1;
    }  
    
    if(bit == 1){
        bufferMB[posbyte] |= mascara; // Operador OR
    }else{
        bufferMB[posbyte] &= ~mascara; // Operador AND y NOT
    }
    
    if(bwrite(nbloqueabs, bufferMB) == -1){ // Escribimos bloque
        fprintf(stderr, "Error while writing\n");
        return -1;
    }
    return 0;
}

/*
 * Lee el bit del MB determinado por nbloque 
 * 
 * Input:   nbloque     => Posición de lectura
 * Output:  Valor leido
 * Using:   bread
 */
char leer_bit(unsigned int nbloque){
    int posbyte = nbloque / 8; 
    int posbit = nbloque % 8;
    int nbloqueMB = posbyte / BLOCKSIZE ;
    int nbloqueabs =  SB.posPrimerBloqueMB + nbloqueMB;
    
    unsigned char bufferMB[BLOCKSIZE]; 
    posbyte = posbyte % BLOCKSIZE;
    unsigned char mascara = 128;

    if(bread(nbloqueabs, bufferMB) == -1){ // Leemos bloque
        fprintf(stderr, "Error while reading\n");
        return -1;
    }  
    mascara >>= posbit; // Desplazamiento de bits a la derecha
    mascara &= bufferMB[posbyte]; // Operador and para bits
    mascara >>= (7 - posbit);   // Desplazamiento de bits a la derecha
    
    // Print debug
    // printf("[leer_bit(%d) -> posbyte: %d, posbit: %d, nbloqueMB: %d, nbloqueabs: %d]\n", nbloque/BLOCKSIZE, posbyte, posbit, nbloqueMB, nbloqueabs);

    if(mascara == 0){
        // printf("leer_bit(%d) = 0\n", nbloque/BLOCKSIZE);
        return 0;
    }else{
        // printf("leer_bit(%d) = 1\n", nbloque/BLOCKSIZE);
        return 1;
    }
}

/*
 * Encuentra el primer bloque libre, consultando el MB, lo ocupa y devuelve su posición.
 * 
 * Input:   none
 * Output:  Posición de memoria reservada
 * Using:   memset, bread, escribir_bit, bwrite,  
 */
int reservar_bloque(){ 
    if(bread(posSB, &SB) == -1){
        fprintf(stderr, "Error while reading\n");
        return -1;
    }

    if(SB.cantBloquesLibres == 0){ 
        fprintf(stderr, "Error: Bloques has reached maximum capacity\n");  
        return -1; 
    }

    int foundNotOccupied= 0;
    int posBloqueMB = SB.posPrimerBloqueMB;
    unsigned int nbloque;
    int posbyte = 0;
    int posbit = 0;
    unsigned char bufferMB[BLOCKSIZE]; 
    unsigned char auxBufferMB[BLOCKSIZE]; 
    unsigned char mascara = 128;

    if(memset(auxBufferMB, 255, BLOCKSIZE) == NULL){
        fprintf(stderr, "Error while setting memory\n");
        return -1;
    }

     // parse a for
    while(foundNotOccupied == 0){ // Localizamos bloque desocupado
        if(posBloqueMB > SB.posUltimoBloqueMB){ //esto deberia evitar leer memoria ilegal
            fprintf(stderr, "targeted block not found, aborting\n");
            return -1;
        }

        if(bread(posBloqueMB, bufferMB) == -1){ // Leemos bloque
            fprintf(stderr, "Error while writing\n");
            return -1;
        }

        if(memcmp(bufferMB, auxBufferMB, BLOCKSIZE) != 0){ // Vemos si el bloque esta desocupado
            foundNotOccupied = 1;
            break;
        }

        posBloqueMB++;
    }

    for(int i = 0; i < BLOCKSIZE; i++ ){ // Localizamos byte desocupado
        if(bufferMB[i] != 255){ // Si el byte no esta a 1 (ocupado)
            posbyte = i;
            break;
        }
    }
   
    while(bufferMB[posbyte] & mascara){  // Localizamos el bit
        bufferMB[posbyte] <<= 1;
        posbit++;
    }

    nbloque = ((posBloqueMB - SB.posPrimerBloqueMB)*BLOCKSIZE + posbyte)*8 + posbit;
    if(escribir_bit(nbloque, 1) == -1){ 
        fprintf(stderr, "Error while writing a bit\n");
        return -1;
    }

    SB.cantBloquesLibres--;  // Actualizamos la cantidad de bloques libres 
 
    if(bwrite(posSB, &SB) == -1){
        fprintf(stderr, "Error while writing\n");
        return -1;
    }
    
    return nbloque;
}

/*
 * Libera un bloque determinado por nbloque
 * 
 * Input:   nbloque     => posicion de memoria a liberar
 * Output:  Nº del bloque liberado, -1 si ha encontrado un error
 * Using:   none 
 */    
int liberar_bloque(unsigned int nbloque){

    if(escribir_bit(nbloque, 0) == -1){
        fprintf(stderr, "Error while writing a bit\n");
        return -1;
    }

    if(bread(0, &SB) == -1){
        fprintf(stderr, "Error while reading\n");
        return -1;
    }

    SB.cantBloquesLibres++;

    if(bwrite(posSB, &SB) == -1){
        fprintf(stderr, "Error while writing\n");
        return -1;
    }

    return nbloque;
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

    int posBloque = ninodo / (BLOCKSIZE/INODOSIZE); // Numero de veces a moverse
    int posInodo = ninodo % (BLOCKSIZE/INODOSIZE); //Posición del inodo dentro del bloque 
    inodo_t inodos [BLOCKSIZE/INODOSIZE];

    if(bread(SB.posPrimerBloqueAI + posBloque, inodos) == -1){ //Leer el bloque de inodos correspondiente 
        fprintf(stderr,"Error while reading\n");
        return -1;
    }

    inodos[posInodo] = inodo;
    if(bwrite(SB.posPrimerBloqueAI + posBloque, inodos) == -1){
        fprintf(stderr, "Error while writing\n");
        return -1;
    }
    return 0;
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
    if(bread(posSB, &SB) == -1){
        fprintf(stderr, "Error while reading SB\n");
        return -1;
    }

    int posBloque = ninodo / (BLOCKSIZE/INODOSIZE); // Numero de veces a moverse
    int posInodo = ninodo % (BLOCKSIZE/INODOSIZE); // Posición del inodo dentro del bloque 
    inodo_t inodos [BLOCKSIZE/INODOSIZE];

    if(bread(SB.posPrimerBloqueAI + posBloque, inodos) == -1){ //Leer el bloque de inodos correspondiente 
        fprintf(stderr,"Error while reading\n");
        return -1;
    }

    *inodo = inodos[posInodo]; //

    return 0;
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

    if(SB.cantInodosLibres == 0){ 
        fprintf(stderr, "Error: Inodos has reached maximum capacity\n");  
        return -1; 
    }
    
    // Encontrar el primer inodo libre
    int posInodoReservado; // Variable auxiliar para guardar la posicion
    inodo_t inodoReservado; // Variable para inicializar el inodo

    if(bread(posSB, &SB) == -1){  // Leer el SuperBloque para tener los valores actuales 
        fprintf(stderr, "Error while reading SB\n");
        return -1;
    }

    posInodoReservado = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre++;  // Actualizamos la posicion del primer inodo libre

    inodoReservado.tipo = tipo;
    inodoReservado.permisos = permisos;
    inodoReservado.nlinks = 1;
    inodoReservado.tamEnBytesLog = 0;

    inodoReservado.atime = time(NULL);  // Esto da warning
    inodoReservado.ctime = time(NULL);
    inodoReservado.mtime = time(NULL);

    inodoReservado.numBloquesOcupados = 0;  
    memset(inodoReservado.punterosDirectos, 0, sizeof(unsigned int) * 12);   // Llenamos de 0's todo el array 
    memset(inodoReservado.punterosIndirectos, 0, sizeof(unsigned int) * 3);
    // Reserva el inodo inicializado
    escribir_inodo(posInodoReservado, inodoReservado);

    // Actualiza la lista de nodos libres
    SB.cantInodosLibres--;  // Disminuimos la cantidad de inodos que tenemos 
    if(bwrite(posSB, &SB) == -1){
        fprintf(stderr, "Error while writing\n");
        return -1;
    }


    // Devolver la posición del inodo reservado
    return posInodoReservado;
}

/*
 * Obtener el rango de punteros en el que se sitúa el bloque lógico 
 * que buscamos (0:D, 1:I0, 2:I1, 3:I2), y obtenemos además la 
 * dirección almacenada en el puntero correspondiente del inodo
 * 
 * Input:   *inodo      => inodo de referencia
 *          nblogico    => nº bloque logico 
 *          *ptr        => puntero que apunta al bloque correspondiente dentro del rango
 * Output:  Dirección almacenada en el puntero correspondiente del inodo
 * Using:   none 
 */ 
int obtener_nRangoBL(inodo_t *inodo, unsigned int nblogico, unsigned int *ptr){
    if(nblogico < DIRECTOS){
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;

    }else if(nblogico < INDIRECTOS0){
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    }else if(nblogico < INDIRECTOS1){
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    }else if (nblogico < INDIRECTOS2){
        *ptr = inodo->punterosIndirectos[2];
        
        return 3;
    }else{
        ptr = 0;
        fprintf(stderr, "Bloque lógico fuera de rango");
        return -1;
    }
}

/*
 * Obtener los índices de los bloques de punteros
 * 
 * Input:   nblogico        => número bloque lógico
 *          nivel_punteros  => nivel del puntero interesado
 * Output:  Indice del bloque pasado por parametro 
 * Using:   none 
 */ 
int obtener_indice(int nblogico, int nivel_punteros){
    if(nblogico < DIRECTOS){
        return nblogico;
    }else if(nblogico < INDIRECTOS0){
        return nblogico - DIRECTOS;
    }else if(nblogico < INDIRECTOS1){
        if(nivel_punteros == 2){
            return (nblogico - INDIRECTOS0)/NPUNTEROS;
        }else if(nivel_punteros == 1){
            return (nblogico - INDIRECTOS0)%NPUNTEROS;
        }
    }else if(nblogico < INDIRECTOS2){
        if(nivel_punteros == 3){
            return (nblogico - INDIRECTOS1)/(NPUNTEROS*NPUNTEROS);
        }else if(nivel_punteros == 2){
            return ((nblogico - INDIRECTOS1)%(NPUNTEROS*NPUNTEROS))/NPUNTEROS;
        }else if(nivel_punteros == 1){
            return ((nblogico - INDIRECTOS1)%(NPUNTEROS*NPUNTEROS))%NPUNTEROS;
        }
    }

    fprintf(stderr, "Error while obtaining index\n");
    return -1; 
}


/*
 * Esta función se encarga de obtener el nº de bloque físico correspondiente a un 
 * bloque lógico determinado del inodo indicado.
 * 
 * Input:   ninodo    => inodo del que se traducirá
 *          nblogico  => número bloque lógico
 *          reservar  => 0: solo consulta; 1: consultar y en caso de inexistencia de bloque, reservar
 * Output:  nº  de bloque físico  correspondiente a nblogico
 * Using:   none 
 */ 
int traducir_bloque_inodo(int ninodo, int nblogico, char reservar){
    inodo_t inodo;
    int ptr_ant, salvar_inodo, nRangoBL, nivel_punteros, indice;
    int buffer[NPUNTEROS];
    unsigned int ptr;

    leer_inodo(ninodo, &inodo);

    ptr = 0;
    ptr_ant = 0;
    salvar_inodo = 0;

    nRangoBL = obtener_nRangoBL(&inodo, nblogico, &ptr);
    nivel_punteros = nRangoBL;

    while(nivel_punteros>0){
        if(ptr == 0){
            if(reservar != 0){ // Estamos en el nivel esperado
                salvar_inodo = 1;
                ptr = reservar_bloque(); 
                inodo.numBloquesOcupados++;
                inodo.ctime = time(NULL);

                if(nivel_punteros == nRangoBL){
                    //printf("[traducir_bloque_inodo()→ inodo.punterosIndirectos[%i] = %i (reservado BF %i para punteros_nivel%i)]\n", nRangoBL - 1, ptr, ptr, nivel_punteros);
                    inodo.punterosIndirectos[nRangoBL-1] = ptr; // IMPRIMIR PARA TEST
                }else{
                    buffer[indice] = ptr; 
                    //printf("[traducir_bloque_inodo()→ inodo.punteros_nivel%i[%i] = %i (reservado BF %i para punteros_nivel%i)]\n", nivel_punteros, indice, ptr, ptr, nivel_punteros);
                    if(bwrite(ptr_ant, buffer) == -1){
                        fprintf(stderr, "Error while writing\n");
                        return -1;
                    }
                }
            }else{
                return -1;
            }
        }
        
        if(bread(ptr, buffer) == -1){
            fprintf(stderr,"Error while reading\n");
            return -1;
        }
        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr;
        ptr = buffer[indice]; // PRINT
        nivel_punteros--;
    }
    if(ptr == 0){  // Punteros directos 
        if(reservar != 0){
            salvar_inodo = 1;
            ptr = reservar_bloque();
            inodo.numBloquesOcupados++;
            inodo.ctime = time(NULL);
            if(nRangoBL == 0){
               // printf("[traducir_bloque_inodo()→ inodo.punterosDirectos[%i] = %i (reservado BF %i para BL %i)]\n", nblogico, ptr, ptr, nblogico);
                inodo.punterosDirectos[nblogico] = ptr; //IMPRIMIR TEST
                
            }else{ // print nRangoBL
                buffer[indice] = ptr; //IMPRIMIR TEST
               // printf("[traducir_bloque_inodo()→ inodo.punteros_nivel1[%i] = %i (reservado BF %i para BL %i)]\n", indice, ptr, ptr, nblogico);
                if(bwrite(ptr_ant, buffer) == -1){
                    fprintf(stderr, "Error while writing\n");
                    return -1;
                }
            }
            
            
        } else {
            return -1;
        }
    }

    if (salvar_inodo == 1){
        escribir_inodo(ninodo, inodo);  
    }
    return (int) ptr; 
}

int liberar_inodo(unsigned int ninodo){
    inodo_t inodo;

    if(bread(posSB, &SB) == -1){  // Leer el SuperBloque para tener los valores actuales 
        fprintf(stderr, "Error while reading SB\n");
        return -1;
    }
    leer_inodo(ninodo, &inodo);//leemos el inodo que se quiere liberar
    /* Liberamos el inodo en cuestión */

    inodo.numBloquesOcupados -= liberar_bloques_inodo(0, &inodo);
    if(inodo.numBloquesOcupados != 0){
        return -1;
    }

    inodo.tipo = 'l';
    inodo.tamEnBytesLog = 0;
    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre = ninodo; //dejamos como libre el inodo correspondiente en el superbloque
    SB.cantInodosLibres++;

    escribir_inodo(ninodo, inodo);

    if(bwrite(posSB, &SB) == -1){
        fprintf(stderr, "Error while writing\n");
        return -1;
    }

    return ninodo;
    /* Notas para tener en cuenta:
        - El superbloque se lee cuando se invoca "leer_inodo", asi que en principio no hace falta
        hacerlo de nuevo en este método.
        - SB.posPrimerInodoLibre es una variable que se incrementa cuando se reserva un inodo, por lo que
        es lógico pensar que se decrementa cuando se libera algun inodo. En el pdf se comenta que se tiene
        que actualizar una lista enlazada SB.posPrimerInodoLibre, algo que no he visto en el código. Con esto
        quiero decir que puede ser un posible fallo.
    */
}

int liberar_bloques_inodo(unsigned int primerBL, inodo_t *inodo){
    unsigned int nivel_punteros, indice, ptr, nBL, ultimoBL;
    int nRangoBL;
    unsigned int bloques_punteros[3][NPUNTEROS];
    unsigned char bufAux_punteros[BLOCKSIZE];
    int ptr_nivel[3];
    int indices[3];
    int liberados;

    liberados = 0;
    if(inodo -> tamEnBytesLog == 0){
        fprintf(stderr, "File is empty\n");
        return 0;
    }

    if(inodo -> tamEnBytesLog%BLOCKSIZE == 0){
        ultimoBL = inodo -> tamEnBytesLog/BLOCKSIZE -1;
    }else{
        ultimoBL = inodo -> tamEnBytesLog/BLOCKSIZE;
    }

    memset(bufAux_punteros, 0, BLOCKSIZE);
    ptr = 0;

    fprintf(stderr, "[liberar_bloques_inodo() → primer BL: %i, último BL: %i]\n",
    primerBL, ultimoBL);

    for (nBL = primerBL ; nBL <= ultimoBL ; nBL++){
        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr); // &inodo por utilizar inodo_t
        if(nRangoBL < 0){
            return -1;
        }

        nivel_punteros = nRangoBL;

        while((ptr > 0) && (nivel_punteros > 0)){
            indice = obtener_indice(nBL, nivel_punteros);
            if((indice == 0) || (nBL == primerBL)){
                if(bread(ptr, bloques_punteros[nivel_punteros-1]) == -1){ // Leemos bloque
                    fprintf(stderr, "Error while reading\n");
                    return -1;
                }  
            }

            ptr_nivel[nivel_punteros - 1] = ptr;
            indices[nivel_punteros - 1] = indice;
            ptr = bloques_punteros[nivel_punteros - 1][indice];
            nivel_punteros--;
        }

        if(ptr > 0 ){
            liberar_bloque(ptr);
            liberados++;

            fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %i de datos para BL: %i]\n",
            ptr, nBL);

            if(nRangoBL == 0){
                inodo -> punterosDirectos[nBL] = 0;
            }else{
                nivel_punteros = 1;
                while(nivel_punteros <= nRangoBL ){
                    indice = indices[nivel_punteros - 1];
                    bloques_punteros[nivel_punteros - 1][indice] = 0;
                    ptr = ptr_nivel[nivel_punteros -1];

                    if(memcmp(bloques_punteros[nivel_punteros - 1], bufAux_punteros, BLOCKSIZE) == 0){
                        liberar_bloque(ptr);
                        liberados++;

                        fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %i de punteros_nivel%i correspondiente al BL: %i]\n",
                        ptr, nivel_punteros + 1, nBL);

                        if(nivel_punteros == nRangoBL){
                            inodo -> punterosIndirectos[nRangoBL - 1] = 0;
                        }
                        nivel_punteros++;
                    }else{
                        if(bwrite(ptr, bloques_punteros[nivel_punteros-1]) == -1){ // Leemos bloque
                            fprintf(stderr, "Error while writing\n");
                            return -1;
                        } 
                        
                        nivel_punteros = nRangoBL + 1;
                    }
                }
            }
        }
    }

    fprintf(stderr, "[liberar_bloques_inodo() → total bloques liberados: %i]\n",
    liberados);
    
    return liberados;
}