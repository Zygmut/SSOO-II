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
    unsigned char buffer[BLOCKSIZE];
    int posBlock = SB.posPrimerBloqueDatos;  
    if (bread(posSB, &SB) == -1){
        fprintf(stderr, "Erro while reading\n");
        return -1;
    }

    if(memset(buffer,'0',sizeof(buffer)) == NULL){
        fprintf(stderr, "Error while setting memory\n");
        return -1;
    }


    for (int i = 0; i <= SB.cantBloquesLibres; i++){
        
        if(bwrite(posBlock, buffer) == -1){
            fprintf(stderr, "Error while writting\n");
            return -1;
        }
        posBlock++;

        if(memset(buffer,'0',sizeof(buffer)) == NULL){
            fprintf(stderr, "Error while setting memory\n");
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
            fprintf(stderr, "Error while writting\n");
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
        fprintf(stderr, "Error while writting\n");
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
    printf("[leer_bit(%d) -> posbyte: %d, posbit: %d, nbloqueMB: %d, nbloqueabs: %d]\n", nbloque/BLOCKSIZE, posbyte, posbit, nbloqueMB, nbloqueabs);

    if(mascara == 0){
        printf("leer_bit(%d) = 0\n", nbloque/BLOCKSIZE);
        return 0;
    }else{
        printf("leer_bit(%d) = 1\n", nbloque/BLOCKSIZE);
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
        if(bread(posBloqueMB, bufferMB) == -1){ // Leemos bloque
            fprintf(stderr, "Error while writting\n");
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
        fprintf(stderr, "Error while writting a bit\n");
        return -1;
    }

    SB.cantBloquesLibres--;  // Actualizamos la cantidad de bloques libres 

    if(memset(auxBufferMB, 0, BLOCKSIZE) == NULL){
        fprintf(stderr, "Error while setting memory\n");
        return -1;
    }

    /* if(cmp(nbloque, auxBufferMB) == -1){ // Not really sure about this one (penultimo circulo de la funcion 3), grabamos buffer de 0s
        fprintf(stderr, "Error while writting\n");
        return -1;
    } */   
 
    if(bwrite(posSB, &SB) == -1){
        fprintf(stderr, "Error while writting\n");
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
        fprintf(stderr, "Error while writting a bit\n");
        return -1;
    }

    if(bread(0, &SB) == -1){
        fprintf(stderr, "Error while reading\n");
        return -1;
    }

    SB.cantBloquesLibres++;

    if(bwrite(posSB, &SB) == -1){
        fprintf(stderr, "Error while writting\n");
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
    if(bread(0, &SB) == -1){
        fprintf(stderr, "Error while reading SB\n");
        return -1;
    }

    int posBloque = ninodo / (BLOCKSIZE/INODOSIZE); // Numero de veces a moverse
    int posInodo = ninodo % (BLOCKSIZE/INODOSIZE); //Posición del inodo dentro del bloque 
    inodo_t inodos [BLOCKSIZE/INODOSIZE];

    if(bread(SB.posPrimerBloqueAI + posBloque, inodos) == -1){ //Leer el bloque de inodos correspondiente 
        fprintf(stderr,"Error while reading\n");
        return -1;
    }
    inodos[posInodo] = inodo;

    if(bwrite(SB.posPrimerBloqueAI + posBloque, inodos) == -1){
        fprintf(stderr, "Error while writting\n");
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
        fprintf(stderr, "Error while writting\n");
        return -1;
    }

    // Devolver la posición del inodo reservado
    return posInodoReservado;
}