#include "directorios.h"

int main(int argc, char **argv){

    if(argc != 5){
        fprintf(stderr,"Command syntax should be: ./mi_escribir_varios <nombre_dispositivo> </ruta_fichero> <texto> <offset>\n");
        return -1;
    }

    if (bmount(argv[1]) == -1){
        fprintf(stderr,"Error while mounting\n");
        return -1;
    }

    char *buffer_texto = argv[3];
    int longitud = strlen(buffer_texto);

    if(argv[2][strlen(argv[2])-1] == '/'){
        fprintf(stderr, "Error: La ruta se corresponde a un directorio\n");
        return -1;
    }

    char *camino = argv[2];

    unsigned int offset = atoi(argv[4]);
    int escritos = 0;
    int varios = 10;
    fprintf(stderr,"Longitud de texto: %d\n", longitud);
    for(int i = 0; i < varios; i++){
        escritos += mi_write(camino, buffer_texto, offset+BLOCKSIZE*i, longitud);
    }
    fprintf(stderr, "Bytes escritps: %d\n", escritos);

    bumount();
}