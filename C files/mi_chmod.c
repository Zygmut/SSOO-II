// Jaume Julia Vallespir, Ruben Palmer Perez, Marc Torres Torres
#include "directorios.h"

int main(int argc,char **argv){

    if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL){ // Checkear syntax 
        fprintf(stderr,"Command syntax should be: mi_chmod <disco> <permisos> </ruta>\n");
        return -1;
    }

    if((atoi(argv[2]) < 0) || (atoi(argv[2]) > 7)){ //Check value of permisos
        fprintf(stderr, "Permisos no validos\n");
        return -1;
    }

    if (bmount(argv[1]) == -1){
        fprintf(stderr,"Error while mounting\n");
        return -1;
    }

    mi_chmod(argv[3], atoi(argv[2]));
    
    bumount();
}