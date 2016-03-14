#include "ficheros.h"

int main(int argc, char **argv) {
    bmount(argv[1]);
    
    char text [2048]= "Pendiente de rellenar";

/*
    int ninodo = reservar_inodo('f', '6');
    printf("\t- Reserva primer inodo: %d\n", ninodo);
    mi_write_f(ninodo, "Esto es la prueba numero 1\0", 5120, 27);
*/

    int ninodo = reservar_inodo('f', '6');
    printf("\t- Reserva inodo: %d\n", ninodo);
    
    mi_write_f(ninodo, text, 409604096, strlen(text));

    //ninodo = reservar_inodo('f', '6');
    // printf("\t- Reserva segundo inodo: %d\n", ninodo);
    //    mi_write_f(ninodo, "Esto es la segunda prueba\0", 256000, 26);

    // ninodo = reservar_inodo('f', '6');
    //  printf("\t- Reserva tercer inodo: %d\n", ninodo);
    //    mi_write_f(ninodo, "Esto es la prueba numero 3\0", 30721024, 27);

    // ninodo = reservar_inodo('f', '6');
    // printf("\t- Reserva cuarto inodo: %d\n", ninodo);
    //    mi_write_f(ninodo, "Esto es la cuarta prueba\0", 71680000, 25);

    bumount();
    return ninodo;
}

