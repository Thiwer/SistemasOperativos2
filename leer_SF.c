#include "leer_SF.h"

int main(int argc, char **argv) {
    // Cogemos los argumentos.
    // Ponemos el nombre del fichero en la variable.
	char *path = argv[1];
    // Ponemos los números de bloque en la variable.
	int nbloques = atoi(argv[2]);
    // Calculamos el número de inodos.
	int ninodos = nbloques / 4;

    // Abrimos el fichero que se usará como dispositivo.
	bmount(path);

	printf("** INFORMACIÓN DEL SISTEMA DE FICHEROS **\n\n");

    // Parte de información del superbloque.
	struct superbloque SB;
	bread(posSB, &SB);
	SBinfo(SB);

    // Parte de información del mapa de bits.
	MBinfo(SB);

	printf("Tamaño inodo -> %lu\n", sizeof(struct inodo));

    // Parte de información del array de inodos.
	printf("* INFORMACIÓN DEL ARRAY DE INODOS *\n");
	struct inodo miInodo;
	int i;
	for (i = 0; i < SB.totInodos; i++) {
		miInodo = leer_inodo(i);
        // Mostraremos información de inodos de tipo 'f'.
		if ((miInodo.tipo == 'f') || (miInodo.tipo =='d')) {
			inodoinfo(miInodo, i);
		}
	}

	bumount();

	return 0;
}

int SBinfo(struct superbloque SB) {
	printf("* INFORMACIÓN DEL SUPERBLOQUE *\n\n");
	printf("· Primer bloque de Mapa de bits: %i\n", SB.posPrimerBloqueMB);
	printf("· Último bloque de Mapa de bits: %i\n", SB.posUltimoBloqueMB);
	printf("· Primer bloque de Array de inodos: %i\n", SB.posPrimerBloqueAI);
	printf("· Último bloque de Array de inodos: %i\n", SB.posUltimoBloqueAI);
	printf("· Primer bloque de Datos: %i\n", SB.posPrimerBloqueDatos);
	printf("· Último bloque de Datos: %i\n", SB.posUltimoBloqueDatos);
	printf("· Posición inodo raíz: %i\n", SB.posInodoRaiz);
	printf("· Primer inodo libre %i\n", SB.posPrimerInodoLibre);
	printf("· Cantidad bloques libres: %i\n", SB.cantBloquesLibres);
	printf("· Cantidad inodos libres: %i\n", SB.cantInodosLibres);
	printf("· Número total de bloques: %i\n", SB.totBloques);
	printf("· Número total de inodos: %i\n", SB.totInodos);
	printf("\n");
}

int MBinfo(struct superbloque SB) {
	printf("* INFORMACIÓN DEL MAPA DE BITS *\n\n");
	int i;
	char bit;
	for (i = SB.posPrimerBloqueMB; i <= SB.posUltimoBloqueMB; i++) {
		bit = leer_bit(i);
		printf("%d", bit);
	}
	printf("\n\n");
}

int inodoinfo(struct inodo miInodo, int ninodo) {
	printf("\n+ INFORMACIÓN DEL INODO %i +\n\n",ninodo);
	printf("· Tipo del inodo:\n");

	switch (miInodo.tipo) {
		case 'f':
		printf("\t - Fichero.\n");
		break;
		case 'd':
		printf("\t - Directorio.\n");
		break;
		case 'l':
		printf("\t - Libre.\n");
		break;
		default:
		printf("\t - Desconocido.\n");
		return -1;
		break;
	}

	printf("\n· Permisos del inodo:\n");
	if ((miInodo.permisos & 4) == 4) {
		printf("\t - Lectura.\n");
	}
	if ((miInodo.permisos & 2) == 2) {
		printf("\t - Escritura.\n");
	}
	if ((miInodo.permisos & 1) == 1) {
		printf("\t - Ejecución.\n");
	}

	printf("\n· Punteros directos del inodo:\n");
	int i;
	for (i = 0; i < 12; i++) {
		printf("\t - Puntero directo %i: %i\n", i, miInodo.punterosDirectos[i]);
	}
	printf("\n· Punteros indirectos del inodo:\n");

	for (i = 0; i < 3; i++) {
		printf("\t - Puntero Indirecto %i: %i\n", i, miInodo.punterosIndirectos[i]);
	}
	struct tm *ts;
	char atime[80];
	char mtime[80];
	char ctime[80];
	ts = localtime(&miInodo.atime);
	strftime(atime, sizeof (atime), "%a %Y-%m-%d %H:%M:%S", ts);
	ts = localtime(&miInodo.mtime);
	strftime(mtime, sizeof (mtime), "%a %Y-%m-%d %H:%M:%S", ts);
	ts = localtime(&miInodo.ctime);
	strftime(ctime, sizeof (ctime), "%a %Y-%m-%d %H:%M:%S", ts);
	printf("\n· Datos temporales del inodo:\n");
	printf("\t - ATIME: %s\n", atime);
	printf("\t - MTIME: %s\n", mtime);
	printf("\t - CTIME: %s\n", ctime);
	printf("\n· Cantidad de enlaces de entradas en directorio: %d\n", miInodo.nlinks);
	printf("\n· Tamaño en bytes lógicos del inodo: %d\n", miInodo.tamEnBytesLog);
	printf("\n· Cantidad de bloques ocupados en la zona de datos: %d\n", miInodo.numBloquesOcupados);

}