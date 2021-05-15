#include "directorios.h"
#define TAMFILA 100

static struct UltimaEntrada UltimaEntradaEscritura;

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo){
    int siDir = 0; //me encanta que no hayan booleanas en C, es muy triste 
                    //Alexa play end - Clowncore
    const char barra = '/';
    if(camino[0] == barra ){        
        
        for (int i = 1 ; (i < strlen(camino)) && (siDir==0); i++) { // Iteramos por todo el path
            if(camino[i] != barra){
                inicial[i-1] = camino[i];
            }else{
                siDir = 1;
                strcpy(final,camino+i);
            }
        }

        //si dir
        if(siDir == 1){
            strcpy(tipo, "d");
            return 1;
        } else { //no hay dir luego es un fichero
            //copiamos camino en inicial sin primera /
            strncpy(inicial, camino + 1, sizeof(char) * strlen(camino) - 1); //+1 para evitar la primera '/'
            strcpy(final, " ");
            strcpy(tipo, "f");
            return 0;
        }   
        
    }
    else{
        // fprintf(stderr, "ERROR en extraer_camino, no empieza por /");
        return ERROR_CAMINO_INCORRECTO;
    }

    return 0;
}
 int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos){
    //reservamos variables
    superbloque_t SB;
    struct entrada entrada;
    inodo_t inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    memset(inicial, 0, sizeof(inicial));
    char final[strlen(camino_parcial)];

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
    fprintf(stderr, "[buscar_entrada() → inicial: %s, final: %s, reserva: %d] \n", inicial, final, reservar);

    leer_inodo(*p_inodo_dir,&inodo_dir);
    if((inodo_dir.permisos & 4) != 4){
        return ERROR_PERMISO_LECTURA;
    }

    //calculamos número de entradas a inodo
    struct entrada buf_entradas[BLOCKSIZE / sizeof(struct entrada)];
    memset(buf_entradas, '\0', sizeof(buf_entradas)); //Init a 0's
    int b_leidos = 0;

    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    num_entrada_inodo = 0;
    
    if(cant_entradas_inodo > 0 ){
        b_leidos += mi_read_f(*p_inodo_dir, &buf_entradas, b_leidos, BLOCKSIZE);

        while((num_entrada_inodo < cant_entradas_inodo) && (strcmp(inicial,buf_entradas[num_entrada_inodo].nombre) != 0)){
            
            num_entrada_inodo++;
            if((num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))) == 0){
                b_leidos += mi_read_f(*p_inodo_dir,&buf_entradas,b_leidos,BLOCKSIZE);
            }
        }
    }
    
    if(((strcmp(inicial,buf_entradas[num_entrada_inodo].nombre)) != 0)){ //strcmp be? maybe !=?
    
        switch (reservar) {
            
        case 0: 
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
        case 1:
            if(inodo_dir.tipo == 'f'){
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;

            }

            if((inodo_dir.permisos & 2) != 2){
                return  ERROR_PERMISO_ESCRITURA;
            }else {
                strcpy(entrada.nombre,inicial);
                if(tipo == 'd'){
                    if(strcmp(final,"/") == 0){
                        entrada.ninodo = reservar_inodo('d',6);
                        fprintf(stderr, "[buscar_entrada() → reservado inodo: %d tipo 'd' con permisos %u para: %s]\n", entrada.ninodo, permisos, entrada.nombre);
                    }else{
                        // printf(" ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO (BUSCAR_ENTRADA, DIRECTORIOS.C)");
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                    
                }else{
                    entrada.ninodo = reservar_inodo('f',6);
                    fprintf(stderr, "[buscar_entrada() → reservado inodo: %d tipo 'f' con permisos %u para: %s]\n", entrada.ninodo, permisos, entrada.nombre);
                }

               
                 printf("[buscar_entrada()-> creada entrada: %s inodo: %d] \n", inicial, entrada.ninodo);
                if( mi_write_f(*p_inodo_dir, &entrada, inodo_dir.tamEnBytesLog,sizeof(struct entrada)) == -1){
                   
                    if(entrada.ninodo != -1){
                        fprintf(stderr, "[buscar_entrada() → liberado inodo %i, reservado a %s\n", num_entrada_inodo, inicial);
                        liberar_inodo(entrada.ninodo);
                    }

                    return EXIT_FAILURE;
                }
               
            
            }
        }
    
     }
    if (((strcmp(final, "/")) == 0) || (tipo == 'f')){   // Tambien tenemos en cuenta si es un fichero
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar==1)){
            // printf("ERROR_ENTRADA_YA_EXISTENTE, (BUSCAR_ENTRADA, DIRECTORIOS.C)");
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        *p_inodo = buf_entradas[num_entrada_inodo].ninodo;
        *p_entrada = num_entrada_inodo;
        return EXIT_SUCCESS;
    }
    else{
        // printf("yeet\n");
        *p_inodo_dir = buf_entradas[num_entrada_inodo].ninodo;
        
        return buscar_entrada(final,p_inodo_dir,p_inodo,p_entrada,reservar,permisos);
    }
}

void mostrar_error_buscar_entrada(int error) {
   // fprintf(stderr, "Error: %d\n", error);
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

int mi_creat(const char *camino, unsigned char permisos){
    superbloque_t SB;
    
    if(bread(posSB, &SB) == -1){  // Leer el SuperBloque para tener los valores actuales 
        fprintf(stderr, "Error while reading SB\n");
        return -1;
    }

    unsigned int p_entrada = 0,p_inodo_dir = 0,p_inodo = 0; //asumimos que es 0 por simplicidad
    //unsigned int p_inodo = SB.posInodoRaiz;
    char reservar = 1;

    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, permisos); //Permisos lectura va bien
    if(error < 0){ // la entrada no ha sido la esperada
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    return 0;
}

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
    char nomTipo[2];
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

int mi_chmod(const char *camino, unsigned char permisos){
    superbloque_t SB;

    if(bread(posSB, &SB) == -1){  // Leer el SuperBloque para tener los valores actuales 
        fprintf(stderr, "Error while reading SB\n");
        return -1;
    }
    int error;
    unsigned int p_inodo_dir, p_inodo;
    p_inodo_dir = SB.posInodoRaiz; p_inodo = SB.posInodoRaiz;
    unsigned int p_entrada = 0;
    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos);
    if(error < 0){ // la entrada no ha sido la esperada
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    mi_chmod_f(p_inodo, permisos); // funcion de ficheros
    return 0;
}

int mi_stat(const char *camino, struct STAT *p_stat){
    superbloque_t SB;

    if(bread(posSB, &SB) == -1){  // Leer el SuperBloque para tener los valores actuales 
        fprintf(stderr, "Error while reading SB\n");
        return -1;
    }
    int error;
    unsigned int p_inodo_dir, p_inodo;
    p_inodo_dir = SB.posInodoRaiz; p_inodo = SB.posInodoRaiz;
    unsigned int p_entrada = 0;
    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4); // Permisos de lectura, no queremos modificar nada
    if(error < 0){ // la entrada no ha sido la esperada
        mostrar_error_buscar_entrada(error);
        return -1;
    }

    mi_stat_f(p_inodo, p_stat);
    return 0;
}

int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes){
    unsigned int p_inodo_dir=0,p_inodo=0,p_entrada=0;

    if(strcmp(UltimaEntradaEscritura.camino, camino) == 0){ //vemos si es escritura sobre el mismo inodo
       p_inodo = UltimaEntradaEscritura.p_inodo;
    }else{

        if(buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,0,2) == -1){
            fprintf(stderr,"Error in buscar_entrada");
            return -1;
        }

        strcpy(UltimaEntradaEscritura.camino, camino);
        UltimaEntradaEscritura.p_inodo = p_inodo;
    }
    return mi_write_f(p_inodo, buf, offset, nbytes);
}

int mi_read(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes){
    unsigned int p_inodo_dir=0,p_inodo=0,p_entrada=0;

    if(strcmp(UltimaEntradaEscritura.camino, camino) == 0){ //vemos si es escritura sobre el mismo inodo
        p_inodo = UltimaEntradaEscritura.p_inodo;
    }
    else{
        if(buscar_entrada(camino,&p_inodo_dir,&p_inodo,&p_entrada,0,4) == -1){
            fprintf(stderr,"Error in buscar_entrada");
            return -1;
        }

        strcpy(UltimaEntradaEscritura.camino, camino);
        UltimaEntradaEscritura.p_inodo = p_inodo;
    }
    return mi_read_f(p_inodo, &buf, offset, nbytes);
}