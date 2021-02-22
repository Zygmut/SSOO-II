#include "bloques.h"

static int descriptor = 0;

/*
 * Montar una unidad virtual
 * 
 * Input:   path -> Path to file 
 * Output:  return first unused file descriptor. -1 in case of error
 * Called:  What functions call bmount
 * Using:   What functions are being used 
 */
int bmount(const char* path){

}

/*
 * Desmontar una unidad virtual
 * 
 * Input:   None 
 * Output:  0 on success, -1 otherwise 
 * Called:  
 * Using:   
 */
int bumount(){

}

/*
 * Escritura de UN bloque 
 * 
 * Input:   nbloque -> Ammount of blocks to write 
 *          buf     -> Buffer to dump on write  
 * Output:  
 * Called:   
 * Using:    
 */
int bwrite(unsigned int nbloque, const void *buf){

}

/*
 * Lectura de un bloque. B R E A D
 * 
 * Input:   nbloque -> Ammount of blocks to read 
 *          buf     -> Buffer to dump on read  
 * Output:  
 * Called:  
 * Using:    
 */
int bread(unsigned int nbloque, void *buf){

}
