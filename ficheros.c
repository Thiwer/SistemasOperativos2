#include "ficheros.h"



int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){
	struct inodo Inodo = leer_inodo(ninodo);
	if ((Inodo.permisos & 2) != 2){
		printf("No tiene permisos de escritura en mi_write_f\n");
		return -1;
	}
	int bytesEscritos = 0;
	unsigned int primerLogico = offset/BLOCKSIZE;
	unsigned int ultimoLogico = (offset+nbytes-1)/BLOCKSIZE;
	unsigned char buf_bloque[BLOCKSIZE];
	memset(buf_bloque, 0, sizeof(buf_bloque));
	int bloqueFisico;
	int incremento;
	
	if (primerLogico == ultimoLogico){ //afecta a un bloque
		int desp1 = offset % BLOCKSIZE;
		bloqueFisico = traducir_bloque_inodo(ninodo, primerLogico, 1); //obtencion y reserva del bloque logico
		if (bread(bloqueFisico, buf_bloque) < 0){
			printf("Error en bread en mi_write_f\n");
			return -1;
		}
		memcpy(buf_bloque+desp1, buf_original, nbytes);
		bytesEscritos = bwrite(bloqueFisico, &buf_bloque); //actualizamos los bytes escritos
		if (bytesEscritos < 0){
			printf("Error en bwrite en mi_write_f\n");
			return -1;
		}
	} else {
		int desp = 0;
		int desplazamiento1 = offset % BLOCKSIZE;
		bloqueFisico = traducir_bloque_inodo(ninodo, primerLogico, 1);
		if (bloqueFisico < 0){
			return bytesEscritos;
		}
		if (bread(bloqueFisico, &buf_bloque)){
			printf("Error en bread en mi_write_f\n");
			return -1;
		}
		memcpy(buf_bloque + desplazamiento1, buf_original, BLOCKSIZE - desplazamiento1);
		incremento = bwrite(bloqueFisico, &buf_bloque);
		if(incremento < 0){
			printf("Error en incremento en mi_write_f, el primero\n");
			return -1;
		}
		desp += desplazamiento1;
		bytesEscritos += incremento;

		//Para los bloques intermedios
		int i;
		for (i = primerLogico; i < ultimoLogico; ++i){
			bloqueFisico = traducir_bloque_inodo(ninodo, i, 1);
			if(bloqueFisico < 0){
				return bytesEscritos;
			}
			memcpy(buf_bloque, buf_original + (BLOCKSIZE - desplazamiento1) + (i - primerLogico - 1) * BLOCKSIZE, BLOCKSIZE);
			incremento = bwrite(bloqueFisico, &buf_bloque);
			if(incremento < 0){
				printf("Error en incremento en mi_write_f, el segundo\n");
				return -1;
			}
			bytesEscritos += incremento;
		}
		//para el ultimo
		int desplazamiento2 = (offset+nbytes-1)%BLOCKSIZE;
		bloqueFisico = traducir_bloque_inodo(ninodo, ultimoLogico, 1);
		if(bloqueFisico < 0){
			return bytesEscritos;
		}
		memcpy(buf_bloque, buf_original + (nbytes - desplazamiento2 - 1), desplazamiento2 + 1);
		incremento = bwrite(bloqueFisico, &buf_bloque);
		if(incremento < 0){
			printf("Error en incremento en mi_write_f, el segundo\n");
			return -1;
		}
		bytesEscritos += incremento;
	}
	Inodo = leer_inodo(ninodo);
	if(offset+nbytes > Inodo.tamEnBytesLog){
		Inodo.tamEnBytesLog = offset + nbytes;
	}
	Inodo.mtime = time(NULL);
	Inodo.ctime = time(NULL);
	if (escribir_inodo(Inodo, ninodo) < 0){
		printf("Error al escribir el inodo en mi_write_f\n");
		return -1;
	}
	return bytesEscritos;
}

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes){
	return 1;
	
	
}

//primera fase de este fichero, etapa 5



