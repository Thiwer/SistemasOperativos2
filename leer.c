#include <time.h>
#include "ficheros.h"

int main(int argc, char **argv) {
    bmount(argv[1]);
    int ninodo = atoi(argv[2]);
    unsigned char buffer[BLOCKSIZE];
    memset(buffer, 0, BLOCKSIZE);

    int i = 0;
    int leido = 0;
    printf("Leyendo archivo\n");
 
    while ((leido = mi_read_f(ninodo, buffer, i * BLOCKSIZE, BLOCKSIZE)) > 0) {
        printf("%s", buffer);
        memset(buffer,0,BLOCKSIZE);
        i++;
    }
    printf("\n");

    if (leido == -1) {
        printf("No existe el fichero");
    }
    
    bumount();
}

