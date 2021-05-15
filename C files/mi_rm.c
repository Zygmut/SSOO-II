#include "directorios.h"

int main(int argc,char **argv){

    if(argv[1] == NULL || argv[2] == NULL){ // Checkear syntax 
        fprintf(stderr,"Command syntax should be: mi_rm <disco> </ruta>\n");
        return -1;
    }

    if (bmount(argv[1]) == -1){
        fprintf(stderr,"Error while mounting\n");
        return -1;
    }

    mi_unlink(argv[2]);

    bumount();

}