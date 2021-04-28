// Jaume, Marc, Ruben
#include "bloques.h"
#include <limits.h> // Valor muy grande para el enlace de Inodos
#include <time.h>   // ctime, atime & mtime 

#define INODOSIZE 128
#define posSB 0
#define tamSB 1

#define NPUNTEROS (BLOCKSIZE/sizeof(unsigned int)) //256
#define DIRECTOS 12
#define INDIRECTOS0 (NPUNTEROS + DIRECTOS) // nivel 1 268
#define INDIRECTOS1 (NPUNTEROS * NPUNTEROS + INDIRECTOS0) // 2 niveles   65804
#define INDIRECTOS2 (NPUNTEROS * NPUNTEROS * NPUNTEROS + INDIRECTOS1) // 3 niveles   16843020

typedef union superbloque{
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
    };
     char padding[BLOCKSIZE];
}superbloque_t;


typedef union inodo{
    struct{   
        unsigned char tipo;                          // l: libre, d: directorio, f: fichero
        unsigned char permisos;                      // (rwx), this values goes from 0 to 7 on octal
        unsigned char reservado_alineacion1[6];
        
        time_t atime;                       // last access                      default time(NULL)
        time_t mtime;                       // last modification of data        default time(NULL)
        time_t ctime;                       // last modification of an Inodo    default time(NULL)

        unsigned int nlinks;                 // Cantidad de enlaces de entradas en directorio
        unsigned int tamEnBytesLog;          // Tamaño en bytes lógicos
        unsigned int numBloquesOcupados;     // Cantidad de bloques en la zona de datos que estan ocupados

        unsigned int punterosDirectos[12];
        unsigned int punterosIndirectos[3];

    };
    char padding[INODOSIZE];
}inodo_t;

int tamMB(unsigned int nbloques);
int tamAI(unsigned int ninodos);
int initSB(unsigned int nbloques, unsigned int ninodos);
int initMB();
int initAI();

int escribir_bit(unsigned int nbloque, unsigned int bit);
char leer_bit(unsigned int nbloque);
int reservar_bloque();
int liberar_bloque(unsigned int nbloque);
int escribir_inodo(unsigned int ninodo, inodo_t inodo);
int leer_inodo(unsigned int ninodo, inodo_t *inodo);
int reservar_inodo(unsigned char tipo, unsigned char permisos);

int obtener_nRangoBL(inodo_t *inodo, unsigned int nblogico, unsigned int *ptr);
int obtener_indice(int nblogico, int nivel_punteros);
int traducir_bloque_inodo(int ninodo, int nblogico, char reservar);

int liberar_inodo(unsigned int ninodo);
int liberar_bloques_inodo(unsigned int primerBL, inodo_t inodo);