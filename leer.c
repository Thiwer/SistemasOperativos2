#include "ficheros.h"

int main(int argc, const char * argv[]){

	printf("El archivo es: %s\n", argv[1]);
	bmount(argv[1]);
	printf("El numero de inodo es: %s\n", argv[2]);
	unsigned char buffer[BLOCKSIZE];
	


	bumount();

}