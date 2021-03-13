#include "ficheros_basico.h"

int main(int argc, char **argsv){
    if(argc == 3){// Not enough arguments  
        if(bmount(argsv[1]) == -1){ // File paths are relative????
            fprintf(stderr,"Error while mounting\n");
            return -1;
        } 
        
        initSB(atoi(argsv[2]), atoi(argsv[2])/4);
        initMB();
        initAI();    
        
        if(bumount() == -1){
            fprintf(stderr, "Error while unmounting\n");
            return -1;
        }
    }else{
        fprintf(stderr, "Not enough arguments\n");
        return -1;
    }
}