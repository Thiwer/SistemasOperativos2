#include "ficheros.h"

int main(int argc, const char * argv[]){

	printf("El archivo es: %s\n", argv[1]);
	bmount(argv[1]);
	unsigned char buffer[2048] = "Mierdas varias";
	printf("El buffer mide %zu\n", strlen(buffer));
	unsigned int ninodo = reservar_inodo('f', 6);
	//int mi_write_f(ninodo, buffer, 0, sizeof(buffer));
	mi_write_f(ninodo, buffer, 5120, strlen(buffer));
	//int mi_write_f(ninodo, buffer, 256000, sizeof(buffer));
	//int mi_write_f(ninodo, buffer, 30720000, sizeof(buffer));
	//int mi_write_f(ninodo, buffer, 71680000, sizeof(buffer));
	bumount();

}