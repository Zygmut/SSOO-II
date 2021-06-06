// Jaume Julia Vallespir, Ruben Palmer Perez, Marc Torres Torres
#include "ficheros.h"

int main(int argc,char **argv){

    const char *dir;
    dir = argv[1];
    
    if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL){ // dir ninodo permisos
        fprintf(stderr, "Command syntax should be: permitir <nombre_dispositivo> <ninodo> <permisos>\n");
        return -1;
    }else{
        if(bmount(dir) == -1){ // File paths are relative????
            fprintf(stderr,"Error while mounting\n");
            return -1;
        } 

        mi_chmod_f(atoi(argv[2]), atoi(argv[3]));

        bumount();
    }

    return 0;
}