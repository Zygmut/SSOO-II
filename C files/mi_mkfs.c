#include "bloques.h"

int main(int argc, char **argsv){
    if(argc == 3){ // not enough arguments  
        if(bmount(argsv[1]) == -1){//file paths are relative????
            fprintf(stderr,"Error while mounting\n");
            return -1;
        } 
        char buffer[BLOCKSIZE];
        memset(buffer, 0, sizeof(buffer));

        for(int i = 0; i < atoi(argsv[2]); i++){
            if(bwrite(i, buffer) == -1){
                fprintf(stderr, "Error while writting\n");
                return -1;
            }
            memset(buffer, 0, sizeof(buffer));
        }
        if(bumount() == -1){
            fprintf(stderr, "Error while unmounting\n");
            return -1;
        }
    }else{
        fprintf(stderr, "Not enough arguments\n");
        return -1;
    }
}