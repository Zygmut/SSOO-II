#include "bloques.h"
#include "limits.h" // Valor muy grande para el enlace de Inodos

#define INODOSIZE 128

typedef union _superbloque{
    struct{
        unsigned int posPrimerBloqueMB;
        unsigned int posUltimoBloqueMB;
        unsigned int posPrimerBloqueAI;
        unsigned int posUltimoBloqueAI;
        unsigned int posPrimerBloqueDatos;
        unsigned int posUltimoBloqueDatos;
        unsigned int posInodoRaiz;
        unsigned int posPrimerInodoLibre;
        unsigned int cantBloquesLibres;
        unsigned int cantInodosLibres;
        unsigned int totBloques;
        unsigned int totInodos;
        //char padding[BLOCKSIZE-12*sizeof(unsigned int)];
    };
     char padding[BLOCKSIZE];
}superbloque_t;


typedef union _inodo{
    struct{   
        unsigned char tipo;                          // l: libre, d: directorio, f: fichero
        unsigned char permisos;                      // (rwx), this values goes from 0 to 7 on octal
        unsigned char reservado_alineacion1[6];
        
        time_t atime;                       // last access                      default time(NULL)
        time_t mtime;                       // last modification of data        default time(NULL)
        time_t ctime;                       // last modification of an Inodo    default time(NULL)

        unsigned int nlinks;                 // 1 default
        unsigned int tamEnBytesLog;
        unsigned int numBloquesOcupados;

        unsigned int punterosDirectos[12];
        unsigned int punterosInderectos[3];

        // char padding[INODOSIZE - 8*sizeof(unsigned char) - 3*sizeof(time_t) - 18*sizeof(unsigned int)];
    };
    char padding[INODOSIZE];
}inodo_t;

int tamMB(unsigned int nbloques);
int tamAI(unsigned int ninodos);
int initSB(unsigned int nbloques, unsigned int ninodos);
int initMB();
int initAI();