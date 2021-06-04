#include "verificacion.h"
#include "simulacion.c"

int main(int argc,char **argv){

    if(argv[1] == NULL || argv[2] == NULL || argv[2][length(argv[2]-1)] != "/"){ // Checkear syntax y aseguramos que sea un directorio
        fprintf(stderr,"Command syntax should be: ./verificacion <disco> <directorio_simulacion>\n");
        return -1;
    }

    if(bmount(argv[1]) == -1){
        fprintf(stderr, "Error while mounting\n");
    }

    struct STAT stat;
    struct entrada buff_entrada [NUMPROCESOS];
    memset(buff_entrada, 0, sizeof(buff_entrada)); //NUMPROCESOS * sizeof(struct entrada)); // Init a 0's
    //sacamos el inodo
    int ninodo = mi_stat(argv[2],&stat);
    //calculamos número de entradas del inodo
    int num_entradas = stat.tamEnBytesLog / sizeof(struct entrada);
 
    if(num_entradas != NUMPROCESOS){
        fprintf(stderr, "Error: num_entradas != NUMPROCESOS\n");
        return -1;
    }
    //reservamos memoria para copiar la dirección de simulación y un nombre.txt
    char *str_informe = malloc(strlen(argv[2]) + 20); // El char ocupa 1 byte, da igual multiplicar

    strcpy(str_informe, argv[2]);
    strcat(str_informe, "informe.txt");

    //creamos archivo
    if(mi_creat(str_informe, 6) < 0){
        fprintf(stderr, "Error while creating");
        return -1;
    }

    //mejora en las entradas usando mi_read para reducir la cantidad de lecturas en el disco
    //leemos los directorios correspondientes a los procesos
    if(mi_read(argv[2], &buff_entrada, 0, sizeof(buff_entrada)) < 0) { //argv[2] may be &argv[2] ; NUMPROCESOS * sizeof(struct entrada) -> sizeof(buff_entrada)
        return -1;
    }

    struct INFORMACION info[NUMPROCESOS];
    memset(info, 0, sizeof(info)); // limpiamos el buffer de informacion
    int cant_registros_buffer_escrituras = 256;
    struct REGISTRO buffer_escrituras[cant_registros_buffer_escrituras];
        
    struct entrada entrada_aux;

    for(int i = 0 ; i < num_entradas ; i++){
        //memset(&info, 0, sizeof(struct INFORMACION));        
        //memcpy(&info, i * sizeof(struct INFORMACION), sizeof(struct INFORMACION));
        memset(&entrada_aux, 0, sizeof(struct entrada));
        
        // Creemos que son equivalentes | "leemos" la i-esima entrada del directorio
        memcpy(&entrada_aux, i * sizeof(struct entrada) + buff_entrada, sizeof(struct entrada));
        memcpy(&entrada_aux, &buff_entrada[i], sizeof(struct entrada));

        info[i].pid = atoi(strchr(entrada_aux.nombre, '_') + 1);
        char dir_prueba[64];

        // Crear la direccion de prueba.dat
        sprintf(dir_prueba, "%s%s/%s", argv[2], entrada_aux.nombre, "prueba.dat");
        //Se podria hacer con buscar entrada, a partir del inodo que se encuentra en entrada_aux.ninodo
        //de esa manera, funcionará aunque nos cambien el nombre "prueba.dat". Si lo de arriba funciona, es mas eficiente.
        //buscar_entrada(entrada_aux.ninodo) le hace falta mas cosicas pero ahora no las tenemos
        
        int cant_registros_buffer_escrituras = 256; 
        struct REGISTRO buffer_escrituras [cant_registros_buffer_escrituras];
        memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
        int bytes_leidos = 0;
        int contadorEscriturasValidadas = 0;
        // leer prueba.dat de esta entrada        
        while(mi_read(dir_prueba, buffer_escrituras, bytes_leidos, sizeof(buffer_escrituras)) > 0 ){

            for(int j = 0; j < length(buffer_escrituras) ; j++){
                if(info[i].pid == buffer_escrituras[j].pid){ 
                    if(!contadorEscriturasValidadas){ 
                        info[i].PrimeraEscritura = buffer_escrituras[j];
                        info[i].UltimaEscritura = buffer_escrituras[j];
                        info[i].MayorPosicion = buffer_escrituras[j];
                        info[i].MenorPosicion = buffer_escrituras[j];                                       
                    }else{
                        if(difftime(info[i].PrimeraEscritura.fecha, buffer_escrituras[j].fecha) > 0){
                            info[i].PrimeraEscritura = buffer_escrituras[j];
                        }else if((difftime(info[i].PrimeraEscritura.fecha, buffer_escrituras[j].fecha)) == 0){
                            if(info[i].PrimeraEscritura.nEscritura >= buffer_escrituras[j].nEscritura){ // La primera escritura sera aquella que tenga menos nescrituras
                                info[i].PrimeraEscritura = buffer_escrituras[j];
                            }
                        }

                        if(difftime(info[i].UltimaEscritura.fecha, buffer_escrituras[j].fecha) < 0){
                            info[i].UltimaEscritura = buffer_escrituras[j];
                        }else if((difftime(info[i].PrimeraEscritura.fecha, buffer_escrituras[j].fecha)) == 0){
                            if(info[i].UltimaEscritura.nEscritura >= buffer_escrituras[j].nEscritura){ // La primera escritura sera aquella que tenga menos nescrituras
                                info[i].UltimaEscritura = buffer_escrituras[j];
                            }
                        }
                       
                    }
                    contadorEscriturasValidadas ++;
                }
            }
            memset(buffer_escrituras, 0, sizeof(buffer_escrituras)); 
        }
        //obtener la escritura de la ultima posición //será delimitada por el EOF
        //Añadir la información del struct info al fichero informe.txt por el final
        printRegistro();
    }

    bumount();
}

void printRegistro(){ //metodo para printear los registros

}