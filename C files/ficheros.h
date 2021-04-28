// Jaume, Marc, Ruben
#include "ficheros_basico.h"

struct STAT {     //
    unsigned char tipo;                          // l: libre, d: directorio, f: fichero
    unsigned char permisos;                      // (rwx), this values goes from 0 to 7 on octal
    
    time_t atime;                       // last access                      default time(NULL)
    time_t mtime;                       // last modification of data        default time(NULL)
    time_t ctime;                       // last modification of an Inodo    default time(NULL)

    unsigned int nlinks;             // Cantidad de enlaces de entradas en directorio
    unsigned int tamEnBytesLog;      // Tamaño en bytes lógicos
    unsigned int numBloquesOcupados; // Cantidad de bloques en la zona de datos que estan ocupados
};

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes);
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes);
int mi_chmod_f(unsigned int ninodo, unsigned char permisos);
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat);

int mi_truncar_f(unsigned int ninodo, unsigned int nbytes);