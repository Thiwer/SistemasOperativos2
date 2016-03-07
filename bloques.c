#include "bloques.h"
static int descriptor = 0;


int bmount(const char *camino){
	descriptor = open(camino, O_RDWR | O_CREAT , 0666);
	if (descriptor < 0){
		printf("Error en bmount\n");
		return -1;
	}
	return descriptor;
}

int bumount(){
	int cerrado = close(descriptor);
	if (cerrado < 0){
		printf("Error en bumount\n");
		return -1;
	}
	return cerrado;
}

int bwrite(unsigned int nbloque, const void *buf){
	unsigned int n = BLOCKSIZE*nbloque;
	lseek(descriptor, n, SEEK_SET);
	int numBytes = write(descriptor, buf, BLOCKSIZE);
	if ( numBytes != BLOCKSIZE){
		printf("Error en bwrite\n");
		return -1;
	}
	return numBytes;
}

int bread(unsigned int nbloque, void *buf){
	unsigned int n = BLOCKSIZE*nbloque;
	lseek(descriptor, n, SEEK_SET);
	int numBytes = read(descriptor, buf, BLOCKSIZE);
	if ( numBytes < 0) {
		printf("Error en bread\n");
		return -1;
	}
	return numBytes;
}

