#include "directorios.h"

#define TAMFILA 100
#define TAMBUFFER ( TAMFILA * 1000 )

int main(int argc,char **argv){

    if(argv[1] == NULL || argv[2] == NULL){ // Checkear syntax 
        fprintf(stderr,"Command syntax should be: mi_ls <disco> </ruta directorio>\n");
        return -1;
    }

    if (bmount(argv[1]) == -1){
        fprintf(stderr,"Error while mounting\n");
        return -1;
    }
    superbloque_t SB;
    char tipo;
    if(bread(posSB, &SB) == -1){
        fprintf(stderr, "Error while reading\n");
        return -1;
    }

    char buffer[TAMBUFFER];
    memset(buffer, 0, sizeof(buffer));
    if((mi_dir(argv[2], buffer, &tipo)) != -1){
        if((argv[2][(strlen(argv[2])-1)] != '/') && (tipo == 'd')){
            fprintf(stderr, "No se ha encontrado el fichero\n");
            bumount();
            return -1;
        }
        printf("TIPO\tPERMISOS\tMTIME\t\t\tTAMAÑO\tNOMBRE\n");
        printf("-----------------------------------------------------\n%s", buffer);
    }else{
        bumount();
        return -1;
    }

    bumount();
}