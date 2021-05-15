#include "directorios.h"

int main(int argc, char **argsv){

    if(argsv[1] == NULL || argsv[2] == NULL){ // Checkear syntax 
        fprintf(stderr,"Command syntax should be: ./mi_cat <disco> </ruta_fichero>\n");
        return -1;
    }

    int pathL = strlen(argsv[2]);

    if(argsv[2][pathL-1] != '/'){ // es un fichero

        bmount(argsv[1]);
        
        char *camino = argsv[2];
        int tambuffer = 1500;
        char *lectura = malloc(tambuffer);
        unsigned int offset=0,nbytes =0;

        nbytes = mi_read(camino,lectura,offset,tambuffer);
        
        bumount();
        fprintf(stderr,"Se han leido %d bytes",nbytes);
        return 0;
    }
    else{ //no es un fichero
        fprintf(stderr,"No es un fichero\n");
        return -1;
    }
}