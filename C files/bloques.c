// Jaume, Marc, Ruben
#include "bloques.h"
#include "semaforo_mutex_posix.h"

static int descriptor = 0;
static unsigned int inside_sc = 0;
static sem_t *mutex;

/*
 * Montar una unidad virtual
 * 
 * Input:   path -> Path to file 
 * Output:  return first unused file descriptor. -1 in case of error
 * Using:   open 
 */
int bmount(const char* path){
    if(descriptor > 0){
        close(descriptor);
    }
    
    umask(000);     // Correct permissions rw-rw-rw
    descriptor = open(path, O_RDWR|O_CREAT, 0666);
    if(!mutex){
        mutex = initSem();
        if(mutex == SEM_FAILED){
            return -1;
        }
    }

    return descriptor; //Error should be handled by the user as this returns -1 in case of error
}

/*
 * Desmontar una unidad virtual
 * 
 * Output:  0 on success, -1 otherwise 
 * Using:   close
 */
int bumount(){
    
    descriptor = close(descriptor);
    deleteSem();
    return descriptor; //Error should be handled by the user, same thing as open
}

/*
 * Escritura de UN bloque 
 * 
 * Input:   n_block -> Number of th1e block to write  
 *          buf     -> Buffer to dump on write  
 * Output:  n_bytes written in case of success, -1 in case of error
 * Using:   lseek, write
 */
int bwrite(unsigned int n_block, const void *buf){
    if(lseek(descriptor, n_block*BLOCKSIZE, SEEK_SET) == -1){
        return -1;
    }
    return write(descriptor, buf, BLOCKSIZE);
}

/*
 * Lectura de un bloque. B R E A D
 * 
 * Input:   n_block -> Ammount of blocks to write 
 *          buf     -> Buffer to dump on write  
 * Output:  n_bytes read in case of success, -1 in case of error
 * Using:   lseek, read  
 */
int bread(unsigned int n_block, void *buf){
    if(lseek(descriptor, n_block*BLOCKSIZE, SEEK_SET) == -1){ // maybe w/out SEEK_SET ??
        return -1;
    }
    return read(descriptor, buf, BLOCKSIZE);
}

/*
 * Control del semáforo WAIT
 */
void mi_waitSem() {
   if (!inside_sc) { // inside_sc==0
       waitSem(mutex);
   }
   inside_sc++;
}
/*
 * Control del semáforo SIGNAL 
 */
void mi_signalSem() {
   inside_sc--;
   if (!inside_sc) {
       signalSem(mutex);
   }
}
