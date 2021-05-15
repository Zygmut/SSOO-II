#include "directorios.h"

int main(int argc, char **argsv){

    if(argsv[1] == NULL || argsv[2] == NULL){ // Checkear syntax 
        fprintf(stderr,"Command syntax should be: ./mi_cat <disco> </ruta_fichero>\n");
        return -1;
    }

    int pathL = strlen(argsv[2]);

    if(argsv[2][pathL-1] != '/'){ // es un fichero

        if (bmount(argsv[1]) == -1){
        fprintf(stderr,"Error while mounting\n");
        return -1;
    }
        
        char *camino = argsv[2];
        int tambuffer = BLOCKSIZE * 4 ; // o 1500
        char read[tambuffer];
        
        if(memset(read, 0, sizeof(read)) == NULL){ // Cleansing
            fprintf(stderr, "Error while setting memory\n");
            return -1;
        }

        //char *lectura = malloc(tambuffer);
        unsigned int offset=0,bytes_leidos =0,total_bytes_leidos=0;

        bytes_leidos = mi_read(camino,read,offset,tambuffer);
        while(bytes_leidos  > 0){
            write(1, read, bytes_leidos); //Motrar resultados por pantalla
            total_bytes_leidos += bytes_leidos;
            offset += tambuffer;

            if(memset(read, 0, sizeof(read)) == NULL){ // Cleansing
                fprintf(stderr, "Error while setting memory\n");
                return -1;
            }

            bytes_leidos = mi_read(camino,read,offset,tambuffer);

        }

        bumount();
        fprintf(stderr,"\nSe han leido %d bytes\n",total_bytes_leidos);
        return 0;
    }
    else{ //no es un fichero
        fprintf(stderr,"No es un fichero\n");
        return -1;
    }
}