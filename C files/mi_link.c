// Jaume Julia Vallespir, Ruben Palmer Perez, Marc Torres Torres
#include "directorios.h"

int main(int argc,char **argv){

    if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL){ // Checkear syntax 
        fprintf(stderr,"Command syntax should be: mi_link <disco> </ruta_fichero_original> </ruta_enlace>\n");
        return -1;
    }

    if((argv[2][strlen(argv[2])-1] != '/') && (argv[3][strlen(argv[3])-1] != '/')){ // Ambos son ficheros
        if (bmount(argv[1]) == -1){
            fprintf(stderr,"Error while mounting\n");
            return -1;
        }

        mi_link(argv[2], argv[3]);
       
        bumount();
    }else{ //no es un fichero
        fprintf(stderr,"Ambas rutas deben ser ficheros\n");
        return -1;
    }

    return 0;
}