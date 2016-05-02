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
	struct STAT stat;
    // Leemos el inodo indicado por parámetro.
	Inodo = leer_inodo(ninodo);
    // Asignamos información a la estructura STAT.
	stat.tipo = Inodo.tipo;
	stat.tamEnBytesLog = Inodo.tamEnBytesLog;
	stat.permisos = Inodo.permisos;
	stat.numBloquesOcupados = Inodo.numBloquesOcupados;
	stat.nlinks = Inodo.nlinks;
	stat.mtime = Inodo.mtime;
	stat.ctime = Inodo.ctime;
	stat.atime = Inodo.atime;

	*p_stat = stat;

}