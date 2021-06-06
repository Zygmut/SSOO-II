// Jaume Julia Vallespir, Ruben Palmer Perez, Marc Torres Torres
#include "simulacion.h"


int acabados = 0;

void reaper(){
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended=waitpid(-1, NULL, WNOHANG))>0) {
        acabados++;
    }
}

int main(int argc,char **argv){
    char path[21]; // nombre del archivo
    char tmp[100]; // año, mes,dia,hora,minuto,segundo
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    pid_t pid;

    signal(SIGCHLD,reaper);
    if(argv[1] == NULL){ // Checkear syntax 
        fprintf(stderr,"Command syntax should be: ./simulacion <disco>\n");
        return -1;
    }

    if(bmount(argv[1]) == -1){
        fprintf(stderr, "Error while mounting\n");
    }

    // Creamos el nombre del directorio
    strcpy(path, "/simul_");
    sprintf(tmp, "%d%02d%02d%02d%02d%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,  tm->tm_sec);
    strcat(path, tmp); 
    strcat(path, "/"); 

    mi_creat(path, 6);    

    for(int proceso = 1; proceso <= NUMPROCESOS; proceso++){
        pid = fork();
        if(pid == 0){
            //Hijo
            if(bmount(argv[1]) == -1){
                fprintf(stderr, "Error while mounting\n");
            }
            char path_d[40];
            char path_f[50];
            memset(path_d, 0, sizeof(path_d));
            memset(path_f, 0, sizeof(path_f));
            char path_file[30];
            char pid_d[10];
            memset(path_file, 0, sizeof(path_file));
             
            strcpy(path_d, path);


            // Cremamos el nombre del directorio hijo
            strcat(path_d, "proceso_");
            sprintf(pid_d, "%d", getpid()); // Obtenemos el pid del hijo
            strcat(path_d, pid_d);
            strcat(path_d, "/");

            mi_creat(path_d, 6);

            
            strcpy(path_f, path_d);
            strcat(path_f, "prueba.dat");
            mi_creat(path_f, 6);

            srand(time(NULL) + getpid());
            
            
            for(int nescritura=1; nescritura <= NUMESCRITURAS; nescritura++){
                struct REGISTRO registro;
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = nescritura;
                registro.nRegistro = rand() % REGMAX;
                
                mi_write(path_f, &registro, registro.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO));
                // fprintf(stderr,"[simulacion.c -> Escritura %d en %s]\n", nescritura, path_f);
                usleep(50000);
                if(nescritura == NUMESCRITURAS -1){
                    fprintf(stderr,"Proceso %d: Completadas %d escrituras en %s\n", proceso, NUMESCRITURAS, path_f);
                }
            }
            
            bumount();
            exit(0);
        }
        usleep(200000);
        
    }
    while(acabados < NUMPROCESOS){
        pause();
    }
    fprintf(stderr, "Total de procesos terminados: %d\n", acabados);
    bumount();
    return 0;
}

