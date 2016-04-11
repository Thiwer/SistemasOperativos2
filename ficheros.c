#include "ficheros.h"

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){
	struct inodo Inodo = leer_inodo(ninodo);
	int bytesEscritos = 0;
	unsigned int primerLogico = offset/BLOCKSIZE;
	unsigned int ultimoLogico = (offset+nbytes-1)/BLOCKSIZE;
	if ((Inodo.permisos & 2) != 2) {
		printf("No tienes permisos de escritura.\n");
		return -1;
	}
	if(primerLogico == ultimoLogico){
		// Un unico bloque modificado
		unsigned char bufBloque[BLOCKSIZE];
		memset(bufBloque, 0, sizeof(bufBloque));
		int desp1 = offset%BLOCKSIZE;
		int bFisico = traducir_bloque_inodo(ninodo, primerLogico, 1); //reserva del bloque fisico
		if(bread(bFisico, bufBloque) < 0){
			return -1;
		} 
		memcpy(bufBloque+desp1, buf_original, nbytes);
		int incremento;
		if((incremento = bwrite(bFisico, &bufBloque)) < 0){
			return -1;
		} 
		bytesEscritos += incremento;
	}else{
		// Primer bloque
		int desp = 0;
		unsigned char bufBloque[BLOCKSIZE];
		memset(bufBloque, 0, sizeof(bufBloque));
		int desp1 = offset%BLOCKSIZE;
		int bFisico = traducir_bloque_inodo(ninodo, primerLogico, 1); //reserva del bloque fisico
		if(bFisico < 0) return bytesEscritos;
		if(bread(bFisico, bufBloque) < 0) return -1;
		memcpy(bufBloque+desp1, buf_original, BLOCKSIZE-desp1);
		int incremento;
		if((incremento = bwrite(bFisico, &bufBloque)) < 0){
			return -1;
		} 
		desp += desp1;
		bytesEscritos += incremento;
		// Bloques intermedios
		int i;
		for(i = primerLogico + 1; i < ultimoLogico; i++){
			bFisico = traducir_bloque_inodo(ninodo, i, 1);
			if(bFisico < 0) return bytesEscritos;
			memcpy(bufBloque, buf_original + (BLOCKSIZE - desp1) + (i - primerLogico - 1) * BLOCKSIZE, BLOCKSIZE);
			int incremento;
			if((incremento = bwrite(bFisico, &bufBloque)) < 0){
			return -1;
		} 
			bytesEscritos += incremento;
		}
		// Último bloque
		int desp2 = (offset+nbytes-1)%BLOCKSIZE;
		bFisico = traducir_bloque_inodo(ninodo, ultimoLogico, 1);
		if(bFisico < 0) return bytesEscritos;
		if(bread(bFisico, &bufBloque) < 0){
			return -1;
		} 
		memcpy(bufBloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);
		if((incremento = bwrite(bFisico, &bufBloque)) < 0){
			return -1;
		} 
		bytesEscritos += incremento;
	}
	// Actualizar informacion del inodo
	Inodo = leer_inodo(ninodo);
	if(offset+nbytes > Inodo.tamEnBytesLog){
		Inodo.tamEnBytesLog = offset + nbytes;
	}
	Inodo.mtime = time(NULL);
	Inodo.ctime = time(NULL);
	if(escribir_inodo(Inodo, ninodo) < 0){
			return -1;
		} 
	return bytesEscritos;
}


int mi_read_f(unsigned int ninodo, void *buf, unsigned int offset, unsigned int nbytes){
	
	struct inodo Inodo = leer_inodo(ninodo);
	if(!(Inodo.permisos & 4)){
		printf("Error: no tiene los permisos necesarios\n");
		return -1;
	}
	if(offset + nbytes > Inodo.tamEnBytesLog){ //si se quiere leer mas de lo que ocupa el fichero
		nbytes = Inodo.tamEnBytesLog - offset;
	}
	if(offset >= Inodo.tamEnBytesLog){ //Se quiere leer mas alla del final de fichero
		nbytes = 0;
		return nbytes;
	}
	int bFisico = 0;
	unsigned int primerLogico = offset/BLOCKSIZE;
	unsigned int ultimoLogico = (offset+nbytes-1)/BLOCKSIZE;
	unsigned char buffer[BLOCKSIZE];
	memset(buffer, 0, sizeof(buffer));
	int bytesleidos = 0;
	if(primerLogico == ultimoLogico){ //se lee un unico bloque
		bFisico = traducir_bloque_inodo(ninodo, primerLogico, 0);
		if(bread(bFisico, &buffer) < 0){
			return -1;
		} 
		int desp1 = offset % BLOCKSIZE;
		memcpy(buf, buffer+desp1, nbytes);
		bytesleidos += nbytes;

	}else{ // Hay que leer varios bloques
		// Primer bloque
		bFisico = traducir_bloque_inodo(ninodo, primerLogico, 0);
		if(bread(bFisico, &buffer) < 0){
			return -1;
		} 
		int desp1 = offset % BLOCKSIZE;
		memcpy(buf, buffer + desp1, BLOCKSIZE - desp1);
		bytesleidos += BLOCKSIZE - desp1;
		memset(buffer, 0, sizeof(buffer));
		// Bloques intermedios
		int i;
		for(i = primerLogico + 1; i < ultimoLogico; i++){
			bFisico = traducir_bloque_inodo(ninodo, i, 0);
			if(bread(bFisico, &buffer) < 0){
			return -1;
		} 
			memcpy(buf + bytesleidos, buffer, BLOCKSIZE);
			bytesleidos += BLOCKSIZE;
			memset(buffer, 0, sizeof(buffer));
		}
		// Ultimo bloque
		int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
		bFisico = traducir_bloque_inodo(ninodo, ultimoLogico, 0);
		if(bread(bFisico, &buffer) < 0){
			return -1;
		} 
		memcpy(buf + bytesleidos, buffer, desp2 + 1);
		bytesleidos += desp2 + 1;
	}
	

	Inodo = leer_inodo(ninodo);
	Inodo.atime = time(NULL);
	if(escribir_inodo(Inodo, ninodo) < 0){
			return -1;
		} 
	return bytesleidos;
}