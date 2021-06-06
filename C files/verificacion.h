// Jaume Julia Vallespir, Ruben Palmer Perez, Marc Torres Torres
#include "simulacion.h"

struct INFORMACION{
    int pid;
    unsigned int nEscrituras; //validadas
    struct REGISTRO PrimeraEscritura;
    struct REGISTRO UltimaEscritura;
    struct REGISTRO MenorPosicion;
    struct REGISTRO MayorPosicion;
};

void guardarRegistro(struct INFORMACION info, char *str_informe, int *offset);