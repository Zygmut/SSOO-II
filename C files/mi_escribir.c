// Jaume Julia Vallespir, Ruben Palmer Perez, Marc Torres Torres
#include "directorios.h"

int main(int argc, char **argsv){

    if(argsv[1] == NULL || argsv[2] == NULL || argsv[3] == NULL || argsv[4] == NULL){ // Checkear syntax 
        fprintf(stderr,"Command syntax should be: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n");
        return -1;
    }

    int pathL = strlen(argsv[2]);

    if(argsv[2][pathL-1] != '/'){ // es un fichero

        if (bmount(argsv[1]) == -1){
        fprintf(stderr,"Error while mounting\n");
        return -1;
    }

        char *camino = argsv[2];
        char *buffer_texto = argsv[3];
        unsigned int offset=atoi(argsv[4]);

        int nbytes = mi_write(camino,buffer_texto, offset , strlen(buffer_texto));

        if(nbytes == -1){ //escribe 0 bytes no -1
            nbytes = 0;
        }

        bumount();
        fprintf(stderr,"Se han escrito %d bytes\n",nbytes);
        return 0;
    }
    else{ //no es un fichero
        fprintf(stderr,"No es un fichero\n");
        return -1;
    }
}