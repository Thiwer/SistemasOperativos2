#include "bloques.h"

int main(int argc, char **argv) {

	/* 
	argv[0]= "mi_mkfs"
	argv[1]= nombre del fichero
	argv[2]= cantidad de bloques
	*/

	if(argc != 3){
		printf("Faltan argumentos\n");
		return -1;
	}

	int nbloques = atoi(argv[2]);
	int ninodos = nbloques/4;

	unsigned char buffer[BLOCKSIZE];
	memset(buffer, 0, BLOCKSIZE);
	bmount(argv[1]);

	printf("Se ha montado el dispositivo\n");

	int i;
	for(i = 0; i < nbloques; i++){
		bwrite(i, buffer);
		memset(buffer, 0, sizeof(buffer));
	}
	printf("Se han escrito los bloques\n");

	printf("Iniciando superbloque...\n");
	if(initSB(nbloques, ninodos) < 0){
		printf("Error al iniciar el superbloque\n");
		return -1;
	}
	
	printf("Iniciando mapa de bits...\n");
	if(initMB(nbloques) < 0){
		printf("Error al iniciar el mapa de bits\n");
		return -1;
	}

	printf("Iniciando array de inodos...\n");
	if(initAI(ninodos) < 0){
		printf("Error al iniciar el array de inodos\n");
		return -1;
	}
	
	printf("Creando directorio raíz...\n");
	int raiz = reservar_inodo('d', '7');
	if(raiz < 0){
		printf("Error al crear el directorio raíz\n");
		return -1;
	}
	
	printf("Desmontando dispositivo...\n");
	bumount();
	printf("Se ha desmontado el dispositivo\n");

	return 0;
}