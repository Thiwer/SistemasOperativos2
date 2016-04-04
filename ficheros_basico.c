#include "ficheros_basico.h"

int tamMB(unsigned int nbloques){
	if(((nbloques/8)%BLOCKSIZE)>0){
		return ((nbloques/8)/BLOCKSIZE + 1);
	}
	return ((nbloques/8)/BLOCKSIZE);
}

int tamAI(unsigned int ninodos){
	if(((ninodos*T_INODO)%BLOCKSIZE)>0){
		return (((ninodos*T_INODO)/BLOCKSIZE)+1);
	}
	return (((ninodos*T_INODO)/BLOCKSIZE));
}

int initSB(unsigned int nbloques, unsigned int ninodos){

	struct superbloque SB;
	SB.posPrimerBloqueMB = posSB + (sizeof(struct superbloque)/BLOCKSIZE);
	SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
	SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
	SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
	SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
	SB.posUltimoBloqueDatos = nbloques - 1;
	SB.posInodoRaiz = 0;
	SB.posPrimerInodoLibre = 0;
	SB.cantBloquesLibres = nbloques-1-tamMB(nbloques)-tamAI(nbloques); //Actualizados los bloques libres según etapa 3
	SB.cantInodosLibres = ninodos;
	SB.totBloques = nbloques;
	SB.totInodos = ninodos;
	return bwrite(posSB, &SB);
}
//----------------------------------------------------------------------------HERE!
unsigned int calcular_bloques(){
	struct superbloque SB;
	bread(posSB, &SB);
	return SB.totBloques;
}

int initMB(unsigned int nbloques){

	struct superbloque SB;
	bread(posSB, &SB);
	unsigned char buffer[BLOCKSIZE];
	memset(buffer, 0, sizeof(buffer));
	int i;
	for(i = 0; i < tamMB(nbloques); i++){
		bwrite(SB.posPrimerBloqueMB + i, buffer);
	}
	// Iniciamos los bits del superbloque, mapa de bits y array de inodos
	int tamSB = 1;
	for(i = 0; i < tamSB; i++){
		if(escribir_bit(i, 1) < 0){ //Superbloque
			printf("Error al escribir bit\n");
			return -1;
		}
	}
	for(i = 0; i < tamMB(nbloques); i++){
		if(escribir_bit(i+tamSB, 1) < 0){ //Mapa de bits
			printf("Error al escribir bit\n");
			return -1;
		}
	}
	for(i = 0; i < tamAI(nbloques); i++){
		if(escribir_bit(i+tamSB+tamMB(nbloques), 1) < 0){; //Array de inodos
			printf("Error al escribir bit\n");
			return -1;
		}
	}
	return 0;
}

int initAI(unsigned int ninodos){

	struct superbloque SB;
	bread(posSB, &SB);
	int x = 1;
	struct inodo inodos[BLOCKSIZE/T_INODO];
	int i = SB.posPrimerBloqueAI;
	while(i <= SB.posUltimoBloqueAI){
		int j = 0;
		int over = 0;
		while( (j < (BLOCKSIZE/T_INODO)) &&(!over) ){
			inodos[j].tipo = 'l';
			if(x < ninodos){
				inodos[j].punterosDirectos[0] = x;
				x++;
			}else{
				inodos[j].punterosDirectos[0] = UINT_MAX;
				over = 1;
			}
			j++;
		}
		bwrite(i, inodos);
		i++;
	}
	return 0;
}

int escribir_bit(unsigned int nbloque, unsigned int bit){ // Escribe un bit a partir de la posicion 0 del mapa de bits. Se le pasa por parametro el bloque del sistema que se quiere marcar o desmarcar.
	struct superbloque SB;
	bread(posSB, &SB);
	unsigned char mascara = 128;
	unsigned char bufferMB[BLOCKSIZE];
	memset(bufferMB, 0, sizeof(bufferMB));
	
	int posbyte = nbloque / 8;
	int posbit = nbloque % 8;
	int bloque_MB = nbloque/(BLOCKSIZE*8);
	bread(SB.posPrimerBloqueMB + bloque_MB, bufferMB); // Leemos el mapa de bits
	if(bit == 0){
		mascara = mascara >> posbit;
		bufferMB[posbyte%BLOCKSIZE] &= ~mascara;
	}else if(bit == 1){
		mascara = mascara >> posbit;
		bufferMB[posbyte%BLOCKSIZE] |= mascara;
	}else{
		printf("El bit no es ni 1 ni 0\n");
		return -1;
	}

	return bwrite(SB.posPrimerBloqueMB + bloque_MB, bufferMB);
}

unsigned char leer_bit(unsigned int nbloque){
	struct superbloque SB;
	bread(posSB, &SB);
	unsigned char mascara = 128;
	unsigned char bufferMB[BLOCKSIZE];
	memset(bufferMB, 0, sizeof(bufferMB));
	int posbyte = (nbloque/8)%BLOCKSIZE;
	int posbit = nbloque % 8;
	unsigned int bl = nbloque/(BLOCKSIZE*8);
	mascara = mascara >> posbit;
	if(bread(SB.posPrimerBloqueMB+bl, bufferMB) < 0) return -1;
	mascara = mascara & bufferMB[posbyte]; // Realizamos operacion AND binario de la mascara con el byte correspondiente
	mascara = mascara >> (7-posbit); // Desplazamos el bit hacia la derecha
	return mascara;

}

int reservar_bloque(){
	struct superbloque SB;
	bread(posSB, &SB);
	
	if(SB.cantBloquesLibres == 0){
		printf("No hay bloques libres\n");
		return -1;
	}
	unsigned char buffer[BLOCKSIZE];
	unsigned char bufaux[BLOCKSIZE];
	memset(buffer, 0, sizeof(buffer));
	memset(bufaux, 255, sizeof(bufaux));
	unsigned char mask = 128;
	unsigned int posbit = 0;
	int j;
	int i = SB.posPrimerBloqueMB;
	int found = 0;
	int pos = 0;
	int posbyte = 0;
	while((!found)&&(i <= SB.posUltimoBloqueMB)){
		if(bread(i, &buffer) < 0) return -1;
		if(memcmp(buffer, bufaux, sizeof(buffer)) != 0){
			int found2 = 0;
			j = 0;
			while((!found2)&&(j < sizeof(buffer))){
				if(buffer[j] < 255){
					while(buffer[j] & mask){
						buffer[j] <<= 1;
						posbit++;					
					}
					posbyte = j;
					pos = i;	
					found2 = 1;
				}
				j++;
			}
			found = 1;
		}
		i++;
	}
	unsigned int nbloque = ((pos-SB.posPrimerBloqueMB)*BLOCKSIZE + posbyte)*8 + posbit;
	// Actualizamos mapa de bits
	escribir_bit(nbloque, 1);
	
	SB.cantBloquesLibres = SB.cantBloquesLibres - 1;
	bwrite(posSB, &SB);
	return nbloque;
}

int liberar_bloque(unsigned int nbloque){
	struct superbloque SB;
	bread(posSB, &SB);
	
	// Actualizamos mapa de bits
	if(escribir_bit(nbloque, 0) < 0){
		printf("Error al escribir bit\n");
		return -1;
	};

	SB.cantBloquesLibres = SB.cantBloquesLibres + 1;
	
	bwrite(posSB, &SB);
	
	return nbloque;
}

int escribir_inodo(struct inodo Inodo, unsigned int ninodo){
	struct superbloque SB;
	bread(posSB, &SB);
	int nbloque = SB.posPrimerBloqueAI + ninodo/(BLOCKSIZE/T_INODO);
	struct inodo arrayInodos[BLOCKSIZE/T_INODO];
	bread(nbloque, arrayInodos);
	arrayInodos[ninodo % (BLOCKSIZE/T_INODO)] = Inodo;
	int a = bwrite(nbloque, arrayInodos);
	return a;
}

struct inodo leer_inodo(unsigned int ninodo){
	struct superbloque SB;
	bread(posSB, &SB);
	int nbloque = SB.posPrimerBloqueAI + ninodo/(BLOCKSIZE/T_INODO);
	struct inodo arrayInodos[BLOCKSIZE/T_INODO];
	bread(nbloque, &arrayInodos);
	struct inodo Inodo = arrayInodos[ninodo % (BLOCKSIZE/T_INODO)];
	return Inodo;
}

int reservar_inodo(unsigned char tipo, unsigned char permisos){
	struct superbloque SB;
	bread(posSB, &SB);
	int ninodo = SB.posPrimerInodoLibre;
	struct inodo Inodo = leer_inodo(ninodo);
	unsigned int siguientelibre = Inodo.punterosDirectos[0];
	Inodo.tipo = tipo;
	Inodo.permisos = permisos;
	Inodo.nlinks = 1;
	Inodo.tamEnBytesLog = 0;
	Inodo.atime = time(NULL);
	Inodo.mtime = time(NULL);
	Inodo.ctime = time(NULL);
	Inodo.numBloquesOcupados = 0;
	memset(Inodo.punterosDirectos, 0, sizeof(Inodo.punterosDirectos));
	memset(Inodo.punterosIndirectos, 0, sizeof(Inodo.punterosIndirectos));
	escribir_inodo(Inodo, SB.posPrimerInodoLibre); // Escribimos el inodo reservado
	

	// Actualizamos la lista enlazada de inodos libres

	SB.cantInodosLibres--; // Reducimos la cantidad de inodos libres, controlada por el superbloque

	SB.posPrimerInodoLibre = siguientelibre; // El primer inodo libre será siempre al que apunte el superbloque.posPrimerInodoLibre, por lo que para actualizar la lista sólo habrá que corregir la posición del primero. (Para el liberar_inodo deberemos actualizar SB.posPrimerInodoLibre=idnuevo_inodo y nuevo_inodo.punterosDirectos[0]=sb.posPrimerInodoLibre (necesario swap))
	bwrite(posSB, &SB); // Escribimos el superbloque actualizado
	return ninodo;
}

int traducir_bloque_inodo(unsigned int ninodo, unsigned int blogico, unsigned char reservar){
	
	struct inodo Inodo = leer_inodo(ninodo);
	int ptr = 0;
	int antptr = 0;
	unsigned int buffer[BLOCKSIZE/sizeof(unsigned int)]; // 256 entradas de unsigned int por bloque
	int level = obtenerRangoBL(Inodo, blogico, &ptr); // Encuentra el nivel y actualiza el ptr
	int levelfinal = level;
	int i;
	while(level > 0){
		if(ptr == 0){
			if(!reservar){
				return -1;
			}else{
				ptr = reservar_bloque();
				Inodo.numBloquesOcupados++;
				Inodo.ctime = time(NULL);
				if(level == levelfinal){
					Inodo.punterosIndirectos[level-1] = ptr;
				}else{
					buffer[i] = ptr;
					if(bwrite(antptr, &buffer) < 0) return -1;
				}
			}
		}
		memset(buffer, 0, (BLOCKSIZE/sizeof(unsigned int))); // Ponemos el buffer a 0s
		if(bread(ptr, &buffer) < 0) return -1;
		i = obtenerIndice(blogico, level);
		antptr = ptr;
		ptr = buffer[i];
		level--;
	}
	if(ptr == 0){
		if(!reservar){
			return -1;
		}else{
			ptr = reservar_bloque();
			Inodo.numBloquesOcupados++;
			Inodo.ctime = time(NULL);
			if(levelfinal == 0){
				Inodo.punterosDirectos[blogico] = ptr;
			}else{
				buffer[i] = ptr;
				if(bwrite(antptr, &buffer) < 0) return -1;
			}
		}
	}
	escribir_inodo(Inodo, ninodo);
	return ptr;
}

unsigned int obtenerRangoBL(struct inodo Inodo, unsigned int blogico, int *ptr){
	int npunteros = BLOCKSIZE/sizeof(unsigned int); // 256
	int directos = 12;
	int indirectos0 = npunteros + directos;
	int indirectos1 = npunteros*npunteros + indirectos0;
	int indirectos2 = npunteros*npunteros*npunteros + indirectos1;
	if(blogico < directos){
		*ptr = Inodo.punterosDirectos[blogico];
		return 0;
	}else if(blogico < indirectos0){
		*ptr = Inodo.punterosIndirectos[0];
		return 1;
	}else if(blogico < indirectos1){
		*ptr = Inodo.punterosIndirectos[1];
		return 2;
	}else if(blogico < indirectos2){
		*ptr = Inodo.punterosIndirectos[2];
		return 3;
	}else{
		printf("No existe el bloque\n");
		*ptr = 0;
		return -1;
	}
}


int obtenerIndice(unsigned int blogico, unsigned int level){
	int npunteros = BLOCKSIZE/sizeof(unsigned int); // 256
	int directos = 12;
	int indirectos0 = npunteros + directos;
	int indirectos1 = npunteros*npunteros + indirectos0;
	int indirectos2 = npunteros*npunteros*npunteros + indirectos1;
	if(blogico < directos){
		return blogico;
	}else if(blogico < indirectos0){
		return blogico-directos;
	}else if(blogico < indirectos1){
		if(level == 2){
			return ((blogico-indirectos0)/(npunteros));
		}else if(level == 1){
			return ((blogico-indirectos0)%(npunteros));
		}
	}else if(blogico < indirectos2){
		if(level == 3){
			return ((blogico-indirectos1)/(npunteros*npunteros));
		}else if(level == 2){
			return (((blogico-indirectos1)%(npunteros*npunteros))/npunteros);
		}else if(level == 1){
			return (((blogico-indirectos1)%(npunteros*npunteros))%npunteros);
		}
	}
}

int vaciar_nivel(int level, int ptr, int primero, int blogico, int *bliberados){
	unsigned int buffer[BLOCKSIZE/sizeof(unsigned int)];
	unsigned int baux[BLOCKSIZE/sizeof(unsigned int)];
	unsigned int ceros[BLOCKSIZE/sizeof(unsigned int)];
	memset(buffer, 0, sizeof(buffer));
	memset(baux, 0, sizeof(baux));
	memset(ceros, 0, sizeof(ceros));
	if(ptr != 0){
		if(level == 1){
			bread(ptr, buffer);
			int x;
			x = obtenerIndice(blogico, level);
			while(x < 256){
				if(buffer[x] != 0){
					bwrite(buffer[x], ceros);
					liberar_bloque(buffer[x]);
					buffer[x] = 0;
					(*bliberados)++;
				}
				x++;
			}
			bwrite(ptr, buffer);
		}else{
			bread(ptr, buffer);
			int x;
			if(primero == 1){
				x = obtenerIndice(blogico, level);
			}else{
				x = 0;
			}
			level--;
			while(x < 256){
				if(buffer[x] != 0){
					vaciar_nivel(level, buffer[x], primero, blogico, bliberados);
					primero = 0;
					bread(buffer[x], baux);
					if(memcmp(baux, ceros, sizeof(baux)) == 0){
						liberar_bloque(buffer[x]);
						buffer[x] = 0;
						(*bliberados)++;
					}
				}
				x++;
			}
			bwrite(ptr, buffer);
		}
	}
}

int liberar_inodo(unsigned int ninodo){
	
	liberar_bloques_inodo(ninodo, 0);
	
	struct superbloque SB;
	bread(posSB, &SB);
	struct inodo Inodo = leer_inodo(ninodo);
	Inodo.tipo = 'l';
	Inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;
	SB.posPrimerInodoLibre = ninodo;
	SB.cantInodosLibres++;
	escribir_inodo(Inodo, ninodo);
	bwrite(posSB, &SB);
	return ninodo;
	
}

int liberar_bloques_inodo(unsigned int ninodo, unsigned int blogico){
	
	struct inodo Inodo = leer_inodo(ninodo);
	int ptr;
	int bloquesliberados = 0;
	int levelfinal = obtenerRangoBL(Inodo, blogico, &ptr);
	int level = levelfinal;
	unsigned int bufferaux[BLOCKSIZE/sizeof(unsigned int)];
	unsigned int ceros[BLOCKSIZE/sizeof(unsigned int)];
	memset(ceros, 0, sizeof(ceros));
	memset(bufferaux, 0, sizeof(bufferaux));
	int i, j;
	if(levelfinal == 0){
		for(i = blogico; i < 12; i++){
			if(Inodo.punterosDirectos[i] != 0){
				liberar_bloque(Inodo.punterosDirectos[i]);
				Inodo.punterosDirectos[i] = 0;
				bloquesliberados++;
			}
		}
		level++;
	}
	for(j = level; j < 4; j++){
		if(Inodo.punterosIndirectos[j-1] != 0){
			vaciar_nivel(j, Inodo.punterosIndirectos[j-1], 1, blogico, &bloquesliberados);
			bread(Inodo.punterosIndirectos[j-1], bufferaux);
			if(memcmp(bufferaux, ceros, sizeof(bufferaux)) == 0){
				liberar_bloque(Inodo.punterosIndirectos[j-1]);
				Inodo.punterosIndirectos[j-1] = 0;
				bloquesliberados++;
			}
		}
	}
	
	Inodo.numBloquesOcupados = Inodo.numBloquesOcupados - bloquesliberados;
	Inodo.ctime = time(NULL);
	escribir_inodo(Inodo, ninodo);
	return 0;
}