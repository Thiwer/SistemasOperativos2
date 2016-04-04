#include <time.h>
#include "ficheros.h"

int main(int argc, char **argv) {
    // Obtenemos el nombre del fichero.
    char *camino = argv[1];
    // Abrimos el fichero.
    bmount(camino);
    // Obtenemos el número de inodo.
    int ninodo = atoi(argv[2]);

    // Creamos un buffer para leer contenido.
    unsigned char buffer[BLOCKSIZE];
    // Inicializamos el buffer.
    memset(buffer, 0, BLOCKSIZE);

    int i = 0;
    printf("\n\t* Leyendo archivo... * ");
    printf("\n\t* Contenido del archivo: ");
    // Mientras podamos leer...
    int leidos = 0;
    int total = 0;
    
    while ((leidos = mi_read_f(ninodo, buffer, i * BLOCKSIZE, BLOCKSIZE)) > 0) {
        total = total + leidos;
        i++;
        printf("%s", buffer);
        memset(buffer,0,BLOCKSIZE);
    }

    // MODIFICACIÓN DESPUÉS DE HABLAR CON ADELAIDA!!
    if (leidos == -1) {
        printf("No existe el fichero");
    }

    // FIN MODIFICACIÓN.
    printf("\n\nLeidos %i", total);

    printf("\n\n");
    
    bumount();
}

