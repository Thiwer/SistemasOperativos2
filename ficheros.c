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
		int desplazamiento1 = offset%BLOCKSIZE;
		int bFisico = traducir_bloque_inodo(ninodo, primerLogico, 1); // Obtenemos el bloque fisico y reservamos
		printf("---%d---\n", bFisico);
		if(bread(bFisico, bufBloque) < 0) return -1;
		memcpy(bufBloque+desplazamiento1, buf_original, nbytes);
		int aumento;
		if((aumento = bwrite(bFisico, &bufBloque)) < 0) return -1;
		bytesEscritos += aumento;
	}else{
		// Primer bloque
		int desp = 0;
		unsigned char bufBloque[BLOCKSIZE];
		memset(bufBloque, 0, sizeof(bufBloque));
		int desplazamiento1 = offset%BLOCKSIZE;
		int bFisico = traducir_bloque_inodo(ninodo, primerLogico, 1); // Obtenemos el bloque fisico y reservamos
		printf("---%d---\n", bFisico);
		if(bFisico < 0) return bytesEscritos;
		if(bread(bFisico, bufBloque) < 0) return -1;
		memcpy(bufBloque+desplazamiento1, buf_original, BLOCKSIZE-desplazamiento1);
		int aumento;
		if((aumento = bwrite(bFisico, &bufBloque)) < 0) return -1;
		desp += desplazamiento1;
		bytesEscritos += aumento;
		// Bloques intermedios
		int i;
		for(i = primerLogico + 1; i < ultimoLogico; i++){
			bFisico = traducir_bloque_inodo(ninodo, i, 1);
			if(bFisico < 0) return bytesEscritos;
			memcpy(bufBloque, buf_original + (BLOCKSIZE - desplazamiento1) + (i - primerLogico - 1) * BLOCKSIZE, BLOCKSIZE);
			int aumento;
			if((aumento = bwrite(bFisico, &bufBloque)) < 0) return -1;
			bytesEscritos += aumento;
		}
		// Último bloque
		int desplazamiento2 = (offset+nbytes-1)%BLOCKSIZE;
		bFisico = traducir_bloque_inodo(ninodo, ultimoLogico, 1);
		if(bFisico < 0) return bytesEscritos;
		if(bread(bFisico, &bufBloque) < 0) return -1;
		memcpy(bufBloque, buf_original + (nbytes - desplazamiento2 - 1), desplazamiento2 + 1);
		if((aumento = bwrite(bFisico, &bufBloque)) < 0) return -1;
		bytesEscritos += aumento;
	}
	// Actualizar Inodo
	Inodo = leer_inodo(ninodo);
	if(offset+nbytes > Inodo.tamEnBytesLog){
		Inodo.tamEnBytesLog = offset + nbytes;
	}
	Inodo.mtime = time(NULL);
	Inodo.ctime = time(NULL);
	if(escribir_inodo(Inodo, ninodo) < 0) return -1;
	return bytesEscritos;
}


int mi_read_f(unsigned int ninodo, void *buf, unsigned int offset, unsigned int nbytes){
	
	struct inodo Inodo = leer_inodo(ninodo);
	if(!(Inodo.permisos & 4)){
		printf("Error: no tiene los permisos necesarios\n");
		return -1;
	}
	if(offset + nbytes > Inodo.tamEnBytesLog){ // Caso en el que los bytes a leer superan el tamaño de fichero
		nbytes = Inodo.tamEnBytesLog - offset;
	}
	if(offset >= Inodo.tamEnBytesLog){ // Caso en el que se quiere empezar a leer en una posicion posterior al fin de fichero
		nbytes = 0;
		return nbytes;
	}
	int bFisico = 0;
	unsigned int primerLogico = offset/BLOCKSIZE;
	unsigned int ultimoLogico = (offset+nbytes-1)/BLOCKSIZE;
	unsigned char buffer[BLOCKSIZE];
	//unsigned char bufferAux[BLOCKSIZE];
	//memset(bufferAux, 0, sizeof(bufferAux));
	memset(buffer, 0, sizeof(buffer));
	int bytesleidos = 0;
	if(primerLogico == ultimoLogico){ // Caso mismo bloque a leer
		bFisico = traducir_bloque_inodo(ninodo, primerLogico, 0);
		if(bread(bFisico, &buffer) < 0) return -1;
		int desplazamiento1 = offset % BLOCKSIZE;
		memcpy(buf, buffer+desplazamiento1, nbytes);
		bytesleidos += nbytes;

	}else{ // Caso varios bloques
		// Primer bloque ->
		bFisico = traducir_bloque_inodo(ninodo, primerLogico, 0);
		if(bread(bFisico, &buffer) < 0) return -1;
		int desplazamiento1 = offset % BLOCKSIZE;
		memcpy(buf, buffer + desplazamiento1, BLOCKSIZE - desplazamiento1);
		bytesleidos += BLOCKSIZE - desplazamiento1;
		memset(buffer, 0, sizeof(buffer));
		// Bloques intermedios ->
		int i;
		for(i = primerLogico + 1; i < ultimoLogico; i++){
			bFisico = traducir_bloque_inodo(ninodo, i, 0);
			if(bread(bFisico, &buffer) < 0) return -1;
			memcpy(buf + bytesleidos, buffer, BLOCKSIZE);
			bytesleidos += BLOCKSIZE;
			memset(buffer, 0, sizeof(buffer));
		}
		// Ultimo bloque ->
		int desplazamiento2 = (offset + nbytes - 1) % BLOCKSIZE;
		bFisico = traducir_bloque_inodo(ninodo, ultimoLogico, 0);
		if(bread(bFisico, &buffer) < 0) return -1;
		memcpy(buf + bytesleidos, buffer, desplazamiento2 + 1);
		bytesleidos += desplazamiento2 + 1;
	}
	

	Inodo = leer_inodo(ninodo);
	Inodo.atime = time(NULL);
	if(escribir_inodo(Inodo, ninodo) < 0) return -1;
	return bytesleidos;
}



int mi_chmod_f(unsigned int ninodo, unsigned char modo) {
    // Creamos una estructura inodo.
	struct inodo Inodo;
    // Leemos el inodo indicado.
	Inodo = leer_inodo(ninodo);
    // Cambiamos los permisos del inodo.
	Inodo.permisos = modo;
    // Modificamos la variable ctime.
	Inodo.ctime = time(NULL);
    // Actualizamos el inodo.
	escribir_inodo(Inodo, ninodo);
}


int mi_truncar_f(unsigned int ninodo, unsigned int nbytes) {
    // Creamos una estructura inodo.
	struct inodo Inodo;
    // Leemos el inodo indicado por parámetro.
	Inodo = leer_inodo(ninodo);

	if (nbytes > Inodo.tamEnBytesLog) {
		printf("ERROR -> No se ha podido truncar el fichero por sobrepasar su tamaño.\n");
		return -1;
	}

    // Si el número de bytes es 0, vaciamos el fichero.
	if (nbytes == 0) {
		liberar_inodo(ninodo);

		Inodo.tamEnBytesLog = nbytes;

		if (escribir_inodo(Inodo, ninodo) == -1) {
			return -1;
		}
    }// Si no...
    else if (nbytes > 0) {
        // Calculamos el último bloque a truncar.
    	int ultimoblogico = Inodo.tamEnBytesLog / BLOCKSIZE;

        // Si el tamaño en bytes lógicos del inodo es múltiplo del tamaño de bloque
        // decrementamos la variable ultimoblogico.
    	if ((Inodo.tamEnBytesLog % BLOCKSIZE) == 0) {
    		ultimoblogico--;
    	}

        // Calculamos el último bloque que conservamos.
    	int bloq_queda = nbytes / BLOCKSIZE;
        // Si el número de bytes es múltiplo de tamaño de bloque, decrementamos 
        // la variable bloq_queda.
    	if (nbytes % BLOCKSIZE == 0) {
    		bloq_queda--;
    	}

    	unsigned int bfisico;
    	int i = 0;
    	char reservar = '0';
        // Iteramos para todos los bloques que queremos liberar.
    	for (i = bloq_queda + 1; i <= ultimoblogico; i++) {
            // Obtenemos el bloque físico.
    		bfisico = traducir_bloque_inodo(ninodo, i, reservar);

            // Si no es el bloque raíz (SUPERBLOQUE)...
    		if (bfisico > 0) {
    			if (liberar_bloque(bfisico) == -1) {

    				printf("ERROR -> No se ha liberado el bloque.\n");
    				return -1;
    			}
                // Decrementamos los bloques ocupados.
    			Inodo.numBloquesOcupados--;
    		}

            // Si estamos en el último bloque y no ocupa el bloque entero...
    		if ((i == ultimoblogico) && (Inodo.tamEnBytesLog % BLOCKSIZE != 0)) {
                // Truncamos el trozo del bloque.
    			Inodo.tamEnBytesLog = Inodo.tamEnBytesLog - (Inodo.tamEnBytesLog % BLOCKSIZE);
    		} else {
                // Truncamos todo el bloque.
    			Inodo.tamEnBytesLog = Inodo.tamEnBytesLog - BLOCKSIZE;
    		}
    	}

    	Inodo.tamEnBytesLog = nbytes;

    	if (escribir_inodo(Inodo, ninodo) == -1) {
    		return -1;
    	}
    }
    return 0;
}
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat) {
    // Creamos una estructura inodo.
	struct inodo Inodo;
	struct STAT miStat;
    // Leemos el inodo indicado por parámetro.
	Inodo = leer_inodo(ninodo);
    // Asignamos información a la estructura STAT.
	miStat.tipo = Inodo.tipo;
	miStat.tamEnBytesLog = Inodo.tamEnBytesLog;
	miStat.permisos = Inodo.permisos;
	miStat.numBloquesOcupados = Inodo.numBloquesOcupados;
	miStat.nlinks = Inodo.nlinks;
	miStat.mtime = Inodo.mtime;
	miStat.ctime = Inodo.ctime;
	miStat.atime = Inodo.atime;

	*p_stat = miStat;

}