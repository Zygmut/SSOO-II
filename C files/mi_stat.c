#include "directorios.h"

int main(int argc,char **argv){

    if(argv[1] == NULL || argv[2] == NULL){ // Checkear syntax 
        fprintf(stderr,"Command syntax should be: mi_stat <disco> </ruta>\n");
        return -1;
    }

    if (bmount(argv[1]) == -1){
        fprintf(stderr,"Error while mounting\n");
        return -1;
    }

    char adate[24], cdate[24], mdate[24]; 
    struct tm * info;
    struct STAT p_stat;
    int ninodo = mi_stat(argv[2], &p_stat);

    strftime(adate, 24, "%a %d-%m-%Y %H:%M:%S", info = localtime(&p_stat.atime));
    strftime(cdate, 24, "%a %d-%m-%Y %H:%M:%S", info = localtime(&p_stat.ctime));
    strftime(mdate, 24, "%a %d-%m-%Y %H:%M:%S", info = localtime(&p_stat.mtime));
    
    printf("Ruta: %s\tNinodo: %i\n", argv[2], ninodo);
    printf("Tamaño: %i\tBloques: %i\tLinks: %i\n", p_stat.tamEnBytesLog, p_stat.numBloquesOcupados, p_stat.nlinks);
    printf("Tipo: %c\tPermisos: %i\n", p_stat.tipo, p_stat.permisos);
    printf("Acceso: %s\n", adate);
    printf("Modificación: %s\n", mdate);
    printf("Cambio: %s\n", cdate);
    bumount();
}