// Jaume Julia Vallespir, Ruben Palmer Perez, Marc Torres Torres
#include "directorios.h"
#define TAMFILA 100

static struct UltimaEntrada UltimaEntradaEscritura;

/*
 * Dada una cadena de caracteres separa su contenido en inicial y final
 * 
 * Input:   *camino  => Path a extraer
 *          *inicial => Porcion de camino comprendida entre los dos primeros '/'
 *          *final   => Porcion de camino comprendida a partir del segundo '/', este ultimo incluido
 *          *tipo    => tipo
 * 
 * Output: -1 if error, 0 otherwise 
 */ 
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo){
    //El camino ha de empezar por /
    if (camino[0] == '/'){

        if (!strchr(camino + 1, '/')){ //Si solo tiene la / inicial, es un fichero
            strcpy(inicial, camino + 1);
            *tipo = 'f';
            strcpy(final,"");
           
        }
        else{   //Sino, es un directorio
        strncpy(inicial, camino + 1, strchr(camino + 1, '/')- camino - 1);
        *tipo = 'd';
        strcpy(final, strchr(camino + 1, '/'));
        }  
    } 
    else{
        return -1;
    }

    return 0;
}




/*
 * Esta función buscará una determinada entrada entre las entradas del inodo correspondiente a su directorio
 * padre. 
 * 
 * Input:   *camino_parcial  => Cadena de caracteres "path"
 *          *p_inodo_dir     => nº del inodo "padre"
 *          *p_inodo         => nº del inodo al que esta asociado el nombre de la entrada buscada
 *          *p_entrada       => nº de la entrada dentro del inodo *p_inodo_dir que lo contiene
 *          reservar         => 0 si se quiere consultar, 1 si en caso de no existir, reservar dichos inodos
 *          permisos         => permisos 
 * 
 * Output: -1 if error, 0 otherwise 
 */ 
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos){
    //reservamos variables
    superbloque_t SB;
    struct entrada entrada;
    inodo_t inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    memset(inicial, 0, sizeof(inicial));
    char final[strlen(camino_parcial)+1];
    memset(final, 0, strlen(camino_parcial)+1);

    char tipo;
    int cant_entradas_inodo,num_entrada_inodo;
    if(bread(posSB, &SB) == -1){  // Leer el SuperBloque para tener los valores actuales 
        fprintf(stderr, "Error while reading SB\n");
        return -1;
    }
    
    //camino_parcial es "/"
    if(camino_parcial[0] == '/' && strlen(camino_parcial) == 1){
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return 0;
    }

    if (extraer_camino(camino_parcial,inicial,final,&tipo) == -1){
        return ERROR_CAMINO_INCORRECTO;
    }
    //fprintf(stderr, "[buscar_entrada() → inicial: %s, final: %s, reserva: %d] \n", inicial, final, reservar);

    leer_inodo(*p_inodo_dir,&inodo_dir);
    if((inodo_dir.permisos & 4) != 4){
        return ERROR_PERMISO_LECTURA;
    }

    //calculamos número de entradas a inodo
    struct entrada buf_entradas[BLOCKSIZE / sizeof(struct entrada)];
    memset(buf_entradas, 0, sizeof(buf_entradas)); //Init a 0's
    int b_leidos = 0;
    memset(entrada.nombre, 0, sizeof(entrada.nombre));
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    num_entrada_inodo = 0;
    
    if(cant_entradas_inodo > 0 ){
        b_leidos += mi_read_f(*p_inodo_dir, buf_entradas, b_leidos, BLOCKSIZE);
      
        while((num_entrada_inodo < cant_entradas_inodo) && (strcmp(inicial,buf_entradas[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))].nombre) != 0)){
            
            num_entrada_inodo++;
           // fprintf(stderr,"num_inodo: %i\tcantidad_entradas: %i\n", num_entrada_inodo, cant_entradas_inodo);
            if((num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))) == 0){
                memset(buf_entradas, 0, sizeof(buf_entradas)); //Limpiamos el buffer
                b_leidos += mi_read_f(*p_inodo_dir,buf_entradas,b_leidos,BLOCKSIZE);
            }
        }
        memcpy(&entrada, &buf_entradas[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))], sizeof(struct entrada)); 

    }
    
   // fprintf(stderr, "nombre : %s\n", buf_entradas[num_entrada_inodo].nombre);
    if((strcmp(inicial, buf_entradas[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))].nombre)) != 0){ //strcmp be? maybe !=?
    
        switch (reservar) {
            
        case 0: 
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
        case 1:
            if(inodo_dir.tipo == 'f'){
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;

            }

            if((inodo_dir.permisos & 2) != 2){
                return  ERROR_PERMISO_ESCRITURA;
            } else {
                strcpy(entrada.nombre,inicial);
                if(tipo == 'd'){
                    if(strcmp(final,"/") == 0){
                        entrada.ninodo = reservar_inodo('d',permisos);
                       // fprintf(stderr, "[buscar_entrada() → reservado inodo: %d tipo 'd' con permisos %u para: %s]\n", entrada.ninodo, permisos, entrada.nombre);
                    } else {
                        // printf(" ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO (BUSCAR_ENTRADA, DIRECTORIOS.C)");
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                    
                }else{
                    entrada.ninodo = reservar_inodo('f',permisos);
                    //fprintf(stderr, "[buscar_entrada() → reservado inodo: %d tipo 'f' con permisos %u para: %s]\n", entrada.ninodo, permisos, entrada.nombre);
                }

               
               // printf("[buscar_entrada() -> creada entrada: %s inodo: %d] \n", inicial, entrada.ninodo);
                //if( mi_write_f(*p_inodo_dir, &entrada, inodo_dir.tamEnBytesLog,sizeof(struct entrada)) == -1){
                if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == -1) {
                  
                    if(entrada.ninodo != -1){
                       // fprintf(stderr, "[buscar_entrada() → liberado inodo %i, reservado a %s\n", num_entrada_inodo, inicial);
                        liberar_inodo(entrada.ninodo);
                    }

                    return -1;//EXIT_FAILURE;
                }
            }
        }
    
    }
    if (((strcmp(final, "/")) == 0) || ((strcmp(final, "")) == 0) ){   // Tambien tenemos en cuenta si es un fichero (tipo == 'f') -> (strcmp(final, "")) == 0) 
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar==1)){
            // printf("ERROR_ENTRADA_YA_EXISTENTE, (BUSCAR_ENTRADA, DIRECTORIOS.C)");
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
       
            //*p_inodo = entrada.ninodo;
            *p_inodo = entrada.ninodo;//buf_entradas[num_entrada_inodo].ninodo;
        
        //fprintf(stderr,"------%d------ p-inodo 2",p_inodo);
        *p_entrada = num_entrada_inodo;
        return 0;//esto no se si da el resultado esperado.
        // Quizás tendria que poner "return 0"
    }else{
        *p_inodo_dir = entrada.ninodo;//buf_entradas[num_entrada_inodo].ninodo;
        
        return buscar_entrada(final,p_inodo_dir,p_inodo,p_entrada,reservar,permisos);
    }
}

/*
 * Esta función actua como un "switch" para mostrar los diferentes posibles errores  
 * 
 * Input:   error => nº del error en cuestion
 * Output: String del error pasado por parametro
 */ 
void mostrar_error_buscar_entrada(int error) {
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

/*
 * Esta función nos crea un archivo en un path pasado por parámetros con unos permisos dados
 * 
 * Input:   *camino     => "path" a crear el archivo
 *          permisos    => permisos 
 * 
 * Output: -1 if error, 0 otherwise 
 */ 
int mi_creat(const char *camino, unsigned char permisos){
    mi_waitSem();
    superbloque_t SB;
    
    if(bread(posSB, &SB) == -1){  // Leer el SuperBloque para tener los valores actuales 
        fprintf(stderr, "Error while reading SB\n");
        mi_signalSem();
        return -1;
    }

    unsigned int p_entrada = 0,p_inodo_dir = 0,p_inodo = 0; //asumimos que es 0 por simplicidad
    //unsigned int p_inodo = SB.posInodoRaiz;
    char reservar = 1;

    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, permisos); //Permisos lectura va bien
    if(error < 0){ // la entrada no ha sido la esperada
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return -1;
    }
    mi_signalSem();
    return 0;
}

/*
 * Esta función guarda en un buffer los contenidos de un directorio o un fichero
 * 
 * Input:   *camino     => "path" a leer
 *          *buffer     => buffer a guardar los contenidos
 *          *tipo       => utilidad para ficheros 
 * 
 * Output: -1 if error, 0 otherwise 
 */ 
int mi_dir(const char *camino, char *buffer, char *tipo){
    struct tm *tm;
    superbloque_t SB;

    if(bread(posSB, &SB) == -1){  // Leer el SuperBloque para tener los valores actuales 
        fprintf(stderr, "Error while reading SB\n");
        return -1;
    }

    inodo_t inodo;
    int error;
    unsigned int p_inodo_dir, p_inodo;
    p_inodo_dir = SB.posInodoRaiz; p_inodo = SB.posInodoRaiz;
    unsigned int p_entrada = 0;
    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo,&p_entrada, 0, 4); //Permisos lectura va bien
    if(error < 0){ // la entrada no ha sido la esperada
        mostrar_error_buscar_entrada(error);
        return -1;
    }

    leer_inodo(p_inodo, &inodo);

    if((inodo.permisos & 4) != 4){ // Checkeamos si tiene permisos de lectura
        fprintf(stderr, "Inodo doesn't have reading privileges\n");
        return -1;
    }

    char tmp[100]; // Para el tiempo
    char tamEnBytes[10]; // buffer de capacidad 10 ya que es el valor maximo de un unsigned int
    struct entrada entrada;
    char nomTipo[2]; // para el nombre del archivo
    if(camino[(strlen(camino))-1] == '/'){ // En el caso que sea un directorio
        leer_inodo(p_inodo, &inodo);
        *tipo = inodo.tipo;
        // Datos validos, creamos el buffer de salida
        int offset = 0;
        struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
        int cantEntradas = inodo.tamEnBytesLog / sizeof(struct entrada);
        
        printf("Total : %i\n", cantEntradas);
        offset += mi_read_f(p_inodo, entradas, offset, BLOCKSIZE);
        
        for(int i = 0; i < cantEntradas; i++){
            // Leemos todos los inodos / entradas
            leer_inodo(entradas[i % (BLOCKSIZE / sizeof(struct entrada))].ninodo, &inodo); //Leemos el inodo correspndiente
            
            //Tipo
            nomTipo[0] = inodo.tipo;
            nomTipo[1] = '\0';
            strcat(buffer, nomTipo);
            strcat(buffer, "\t");

            //Permisos
            strcat(buffer, ((inodo.permisos & 4) == 4) ? "r" : "-");
            strcat(buffer, ((inodo.permisos & 2) == 2) ? "w" : "-");
            strcat(buffer, ((inodo.permisos & 1) == 1) ? "x" : "-");
            strcat(buffer, "\t");
            
            //mTime
            tm = localtime(&inodo.mtime);
            sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,  tm->tm_sec);
            strcat(buffer, tmp);
            strcat(buffer, "\t");

            //Tamaño
            sprintf(tamEnBytes, "%d", inodo.tamEnBytesLog);
            strcat(buffer, tamEnBytes);
            strcat(buffer, "\t");

            //Nombre

            strcat(buffer, entradas[i % (BLOCKSIZE / sizeof(struct entrada))].nombre);
            while((strlen(buffer) % TAMFILA) != 0){
                strcat(buffer, " ");
            }

            strcat(buffer, "\n"); //Preparamos el string para la siguiente entrada

            if(offset % (BLOCKSIZE/sizeof(struct entrada)) == 0){ 
                offset += mi_read_f(p_inodo, entradas, offset, BLOCKSIZE);
            }
        }
    }else{ // No es un directorio, es un archivo
        mi_read_f(p_inodo_dir, &entrada, sizeof(struct entrada) * p_entrada, sizeof(struct entrada));
        leer_inodo(entrada.ninodo, &inodo);
        *tipo = inodo.tipo;

        nomTipo[0] = inodo.tipo;
        nomTipo[1] = '\0';
        strcat(buffer, nomTipo);
        strcat(buffer, "\t");
        
        //Permisos
        strcat(buffer, ((inodo.permisos & 4) == 4) ? "r" : "-");
        strcat(buffer, ((inodo.permisos & 2) == 2) ? "w" : "-");
        strcat(buffer, ((inodo.permisos & 1) == 1) ? "x" : "-");
        strcat(buffer, "\t");
        
        //mTime
        tm = localtime(&inodo.mtime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,  tm->tm_sec);
        strcat(buffer, tmp);
        strcat(buffer, "\t");

        //Tamaño
        sprintf(tamEnBytes, "%d", inodo.tamEnBytesLog);
        strcat(buffer, tamEnBytes);
        strcat(buffer, "\t");

        //Nombre
        strcat(buffer, entrada.nombre);

        while((strlen(buffer) % TAMFILA) != 0){
            strcat(buffer, " ");
        }
        
        strcat(buffer, "\n"); //Preparamos el string para la siguiente entrada
    }
    return 0;

}

/*
 * Esta función permite modificar los permisos de un archivo
 * 
 * Input:   *camino     => "path" a cambiar los permisos
 *          permisos    => permisos a usar  
 * 
 * Output: -1 if error, 0 otherwise 
 */ 
int mi_chmod(const char *camino, unsigned char permisos){
    superbloque_t SB;

    if(bread(posSB, &SB) == -1){  // Leer el SuperBloque para tener los valores actuales 
        fprintf(stderr, "Error while reading SB\n");
        return -1;
    }
    int error;
    unsigned int p_inodo_dir, p_inodo, p_entrada = 0;
    
    p_inodo_dir = SB.posInodoRaiz; 
    p_inodo = SB.posInodoRaiz;
    
    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos);
    if(error < 0){ // la entrada no ha sido la esperada
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    
    mi_chmod_f(p_inodo, permisos); // funcion de ficheros
    return 0;
}

/*
 * Esta función nos permite visualizar los datos del inodo asociado al "path pasado por parametro"
 * 
 * Input:   *camino     => "path" a leer
 *          *p_stat     => estructura donde guardaremos los datos 
 * 
 * Output: -1 if error, 0 otherwise 
 */
int mi_stat(const char *camino, struct STAT *p_stat){
    superbloque_t SB;

    if(bread(posSB, &SB) == -1){  // Leer el SuperBloque para tener los valores actuales 
        fprintf(stderr, "Error while reading SB\n");
        return -1;
    }
    int error;
    unsigned int p_inodo_dir, p_inodo,p_entrada = 0;

    p_inodo_dir = SB.posInodoRaiz;
    p_inodo = SB.posInodoRaiz;
    
    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4); // Permisos de lectura, no queremos modificar nada
    if(error < 0){ // la entrada no ha sido la esperada
        mostrar_error_buscar_entrada(error);
        return -1;
    }

    mi_stat_f(p_inodo, p_stat);
    return p_inodo;
}

/*
 * Esta función nos permite escribir en un fichero
 * 
 * Input:   *camino     => "path" a escribir
 *          *buffer     => buffer a escribir
 *          offset      => offset de escritura 
 *          nbytes      => tamaño 
 * 
 * Output: nº bytes escritos
 */ 
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes){
    unsigned int p_inodo_dir=0,p_inodo=0,p_entrada=0;

    if(strcmp(UltimaEntradaEscritura.camino, camino) == 0){ //vemos si es escritura sobre el mismo inodo
       p_inodo = UltimaEntradaEscritura.p_inodo;
    }else{
        int error;
        if( (error = buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,0,2)) < -1){ // Permisos 2 par escritura 
            mostrar_error_buscar_entrada(error); //este no es
            return -1;
        }

        strcpy(UltimaEntradaEscritura.camino, camino);
        UltimaEntradaEscritura.p_inodo = p_inodo;
    }
    return mi_write_f(p_inodo, buf, offset, nbytes);
}

/*
 * Esta función nos permite leer el contenido de un fichero
 * 
 * Input:   *camino     => "path" a leer
 *          *buffer     => buffer a leer
 *          offset      => offset de lectura 
 *          nbytes      => tamaño 
 * 
 * Output: nº bytes leidos
 */ 
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes){
    unsigned int p_inodo_dir=0,p_inodo=0,p_entrada=0;

    if(strcmp(camino, UltimaEntradaEscritura.camino) == 0){ //vemos si es escritura sobre el mismo inodo
        p_inodo = UltimaEntradaEscritura.p_inodo;
    }else{
        int error;
        if((error = buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,0,4)) < 0){ // Permisos 4 para lectura
            mostrar_error_buscar_entrada(error);    
            return -1;
        }

        strcpy(UltimaEntradaEscritura.camino, camino);
        UltimaEntradaEscritura.p_inodo = p_inodo;
    }
    return mi_read_f(p_inodo, buf, offset, nbytes);
}

/*
 * Esta función nos permite crear un enlace de directorio camino2 al inodo especificado por otra entrada de 
 * directorio, camino1 a un fichero
 * 
 * Input:   *camino1     => "path maestro"
 *          *camino2     => "path esclavo"
 * 
 * Output: -1 if error, 0 otherwise
 */ 
int mi_link(const char *camino1, const char *camino2){
    mi_waitSem();
    if((camino1[strlen(camino1)-1] == '/') || (camino2[strlen(camino2)-1] == '/')){ // Ambos son ficheros
        fprintf(stderr, "Ambos caminos deben ser ficheros\n");
        mi_signalSem();
        return -1;
    }

    superbloque_t SB;    
    if(bread(posSB, &SB) == -1){  // Leer el SuperBloque para tener los valores actuales 
        fprintf(stderr, "Error while reading SB\n");
        mi_signalSem();
        return -1;
    }
    inodo_t inodo;
    unsigned int p_entrada1 = 0, p_inodo_dir1, p_inodo1 = 0; 
    unsigned int p_entrada2 = 0, p_inodo_dir2, p_inodo2 = 0; 

    p_inodo_dir1 = p_inodo_dir2 = SB.posInodoRaiz; 

    int error1 = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 4); //Permisos lectura va bien
    if(error1 < 0){ // camino 1 no existe
        mostrar_error_buscar_entrada(error1); 
        mi_signalSem();
        return -1;
    }

     leer_inodo(p_inodo1, &inodo);

    if (inodo.tipo != 'f'){
        fprintf(stderr, "mi_link: %s ha de ser un fichero\n", camino1);
        mi_signalSem();
        return -1;
    }
    if ((inodo.permisos & 4) != 4){
        fprintf(stderr, "mi_link: %s no tiene permisos de lectura\n", camino1);
        mi_signalSem();
        return ERROR_PERMISO_LECTURA;
    }

    int error2 = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6); //Permisos lectura va bien
    if(error2 < 0){ //Algun error inesperado que hará fallar el link
        mostrar_error_buscar_entrada(error2);
        mi_signalSem();
        return -1;
    }
    fprintf(stderr,"-----------%d-------- INODO2",p_inodo2);
    struct entrada entrada;
    if(p_inodo2 != 0){
        liberar_inodo(p_inodo2);
    }
    mi_read_f(p_inodo_dir2, &entrada, (p_entrada2 * sizeof(struct entrada)), sizeof(struct entrada));
    entrada.ninodo = p_inodo1; // Asociamos p_entrada2 con p_inodo1
    
    
    mi_write_f(p_inodo_dir2, &entrada, (p_entrada2 * sizeof(struct entrada)), sizeof(struct entrada));
    //liberar_inodo(entrada.ninodo);
    //liberar_inodo(p_inodo2);
    //liberar_inodo(p_inodo2);
    
    //leer_inodo(p_inodo2, &inodo);
    //inodo.numBloquesOcupados = 0;
    //escribir_inodo(p_inodo2, inodo);
    //liberar_inodo(p_inodo2);
                     
   
    
    inodo.nlinks++;
    inodo.ctime = time(NULL);
    escribir_inodo(p_inodo1, inodo);
    mi_signalSem();
    return 0;
}

/*
 * Esta función nos permite borrar la entrada de directorio especificada por parametro
 * 
 * Input:   *camino1     => "path" a borrar
 * 
 * Output: -1 if error, 0 otherwise
 */ 
int mi_unlink(const char *camino){
    mi_waitSem();
    superbloque_t SB;

    if(bread(posSB, &SB) == -1){  // Leer el SuperBloque para tener los valores actuales 
        fprintf(stderr, "Error while reading SB\n");
        mi_signalSem();
        return -1;
    }

    inodo_t inodo, inodo_dir;
    int error;
    unsigned int p_entrada = 0, p_inodo_dir, p_inodo; 

    p_inodo_dir = p_inodo = SB.posInodoRaiz; 

    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo,&p_entrada, 0, 6); //Permisos lectura escritura
    if(error < 0){ // la entrada no ha sido la esperada
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return -1;
    }
    
    // Miramos si es un directorio no vacio
    leer_inodo(p_inodo, &inodo);
    if((inodo.tipo == 'd') && (inodo.tamEnBytesLog > 0)){
        fprintf(stderr, "El inodo[%d] seleccionado es un directorio que no esta vacio\n", p_inodo);
        mi_signalSem();
        return-1;
    }else{
        leer_inodo(p_inodo_dir, &inodo_dir);
        int num_entradas = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
        
        if(p_entrada != num_entradas - 1){ // La entrada a eleminar no es la ultima 
            struct entrada entrada;
            mi_read_f(p_inodo_dir, &entrada, (num_entradas - 1) * sizeof(struct entrada), sizeof(struct entrada));
            mi_write_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada));
        }
        mi_truncar_f(p_inodo_dir, (num_entradas - 1) * sizeof(struct entrada)); // borramos una entrada
        
        
        inodo.nlinks--;
        if(inodo.nlinks == 0){ // liberar_inodo actualiza el tamaño del inodo "padre"???
            liberar_inodo(p_inodo);
        }else{
            inodo.ctime = time(NULL);
            escribir_inodo(p_inodo, inodo);
        }
    }
    mi_signalSem();
    return 0;

}