#include "ficheros.h"

int main(int argc, const char * argv[]){

	bmount(argv[1]);
    char text [2048]= "Empieza con un prólogo en el que se burla de la erudición pedantesca y con unos poemas cómicos, a manera de preliminares, compuestos en alabanza de la obra por el propio autor, quien lo justifica diciendo que no encontró a nadie que quisiera alabar una obra tan extravagante como esta, como sabemos por una carta de Lope de Vega. En efecto, se trata, como dice el cura, de una escritura desatada libre de normativas que mezcla lo lírico, épico, trágico, cómico y donde se entremeten en el desarrollo historias de varios géneros, como por ejemplo: Grisóstomo y la pastora Marcela, la novela de El curioso impertinente, la historia del cautivo, el discurso sobre las armas y las letras, el de la Edad de Oro, la primera salida de don Quijote solo y la segunda con su inseparable escudero Sancho Panza (la segunda parte narra la tercera y postrera salida La novela comienza describiéndonos a un tal Alonso Quijano, hidalgo pobre, que enloquece leyendo libros de caballerías y se cree un caballero medieval. Decide armarse como tal en una venta, que él ve como castillo. Le suceden toda suerte de cómicas aventuras en las que el personaje principal, impulsado en el fondo por la bondad y el idealismo, busca «desfacer agravios» y ayudar a los desfavorecidos y desventurados. Profesa un amor platónico a una tal Dulcinea del Toboso; que es, en realidad, una moza labradora «de muy buen parecer»: Aldonza Lorenzo. El cura y el barbero del lugar someten la biblioteca de don Quijote a un expurgo, y queman parte de los libros que le han hecho tanto mal.Don Quijote lucha contra unos gigantes, que no son otra cosa que molinos de viento";
    
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