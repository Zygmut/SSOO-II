// Jaume, Marc, Ruben
#include "ficheros.h"

int main(int argc,char **argv){
    const char *dir;
    int ninodo = atoi(argv[2]);
    int permisos = atoi(argv[3]);
    dir = argv[1];
    
    if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL){ // dir ninodo permisos
        fprintf(stderr, "Command syntax should be: permitir <nombre_dispositivo> <ninodo> <permisos>\n");
        return -1;
    }else{
        bmount(dir);
        mi_chmod_f(ninodo, permisos);
        bumount();
    }

    return 0;
}