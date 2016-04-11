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
	SB.cantBloquesLibres = nbloques-1-tamMB(nbloques)-tamAI(nbloques);
	SB.cantInodosLibres = ninodos;
	SB.totBloques = nbloques;
	SB.totInodos = ninodos;
	return bwrite(posSB, &SB);
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
	while(i <= SB.posUltimoBloqueAI){ //para todos bloques del array de inodos
		int j = 0;
		int over = 0;
		while( (j < (BLOCKSIZE/T_INODO)) &&(!over) ){ //para cada inodo dentro de 1 bloque
			inodos[j].tipo = 'l';
			if(x < ninodos){ //si no es el ultimo
				inodos[j].punterosDirectos[0] = x;
				x++;
			}else{ //para el ultimo
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

int escribir_bit(unsigned int nbloque, unsigned int bit){
	
	if(bit < 0 || bit > 1){
		printf("El bit indicado no es correcto\n");
		return -1;
	}

	struct superbloque SB;
	bread(posSB, &SB);
	unsigned char mascara = 128;
	unsigned char bufMB[BLOCKSIZE];
	memset(bufMB, 0, sizeof(bufMB));
	
	int posbit = nbloque % 8;
	int posbyte = nbloque / 8;
	int bloque_MB = nbloque/(BLOCKSIZE*8);
	bread(SB.posPrimerBloqueMB + bloque_MB, bufMB); // Leemos el mapa de bits
	if(bit == 0){
		mascara = mascara >> posbit;
		bufMB[posbyte%BLOCKSIZE] &= ~mascara;
	}else if(bit == 1){
		mascara = mascara >> posbit;
		bufMB[posbyte%BLOCKSIZE] |= mascara;
	}
	return bwrite(SB.posPrimerBloqueMB + bloque_MB, bufMB);
}

unsigned char leer_bit(unsigned int nbloque){
	struct superbloque SB;
	bread(posSB, &SB);
	unsigned char mascara = 128;
	unsigned char bufMB[BLOCKSIZE];
	memset(bufMB, 0, sizeof(bufMB));
	int posbyte = (nbloque/8)%BLOCKSIZE;
	int posbit = nbloque % 8;
	unsigned int bl = nbloque/(BLOCKSIZE*8);
	mascara = mascara >> posbit;
	if(bread(SB.posPrimerBloqueMB+bl, bufMB) < 0) return -1;
	mascara = mascara & bufMB[posbyte]; //And binaria
	mascara = mascara >> (7-posbit); // Desplazamos a la derecha
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
	unsigned char bufferAux[BLOCKSIZE];
	memset(buffer, 0, sizeof(buffer));
	memset(bufferAux, 255, sizeof(bufferAux));
	unsigned char mascara = 128;
	unsigned int posbit = 0;
	int i = SB.posPrimerBloqueMB;
	int j;
	int encontrado = 0;
	int posicion = 0;
	int posbyte = 0;
	while((!encontrado)&&(i <= SB.posUltimoBloqueMB)){ //busqueda por bloques
		if(bread(i, &buffer) < 0) return -1;
		if(memcmp(buffer, bufferAux, sizeof(buffer)) != 0){
			int encontrado2 = 0;
			j = 0;
			while((!encontrado2)&&(j < sizeof(buffer))){  //busqueda dentro de bloques
				if(buffer[j] < 255){
					while(buffer[j] & mascara){
						buffer[j] <<= 1;
						posbit++;					
					}
					posbyte = j;
					posicion = i;	
					encontrado2 = 1;
				}
				j++;
			}
			encontrado = 1;
		}
		i++;
	}
	unsigned int nbloque = ((posicion-SB.posPrimerBloqueMB)*BLOCKSIZE + posbyte)*8 + posbit;
	// Actualizamos mapa de bits
	escribir_bit(nbloque, 1);
	
	SB.cantBloquesLibres = SB.cantBloquesLibres - 1;
	bwrite(posSB, &SB);
	return nbloque;
}

int liberar_bloque(unsigned int nbloque){
	struct superbloque SB;
	bread(posSB, &SB);
	
	if(escribir_bit(nbloque, 0) < 0){ //actualizacion del mapa de bits
		printf("Error al escribir el bit\n");
		return -1;
	}

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
	unsigned int siguientelibre = Inodo.punterosDirectos[0]; //el siguiente libre esta en el primer puntero
	Inodo.tipo = tipo;
	Inodo.permisos = permisos;
	Inodo.nlinks = 1;
	Inodo.tamEnBytesLog = 0;
	Inodo.atime = time(NULL); //actualizacion de tiempos
	Inodo.mtime = time(NULL);
	Inodo.ctime = time(NULL);
	Inodo.numBloquesOcupados = 0;
	memset(Inodo.punterosDirectos, 0, sizeof(Inodo.punterosDirectos));
	memset(Inodo.punterosIndirectos, 0, sizeof(Inodo.punterosIndirectos));
	escribir_inodo(Inodo, SB.posPrimerInodoLibre);
	SB.cantInodosLibres--; // Hay un inodo libre menos

	SB.posPrimerInodoLibre = siguientelibre;
	bwrite(posSB, &SB); 
	return ninodo;
}

int traducir_bloque_inodo(unsigned int ninodo, unsigned int blogico, unsigned char reservar){

	struct inodo Inodo = leer_inodo(ninodo);
	int ptr = 0;
	int ptrPre = 0; //puntero precedente
	unsigned int buffer[BLOCKSIZE/sizeof(unsigned int)]; //256
	int nivel = obtenerRangoBL(Inodo, blogico, &ptr);
	int nivelFinal = nivel;
	int i;
	while(nivel > 0){
		if(ptr == 0){
			if(!reservar){
				return -1;
			}else{ //si no hay que reservar no hace falta reservar bloques
				ptr = reservar_bloque();
				Inodo.numBloquesOcupados++;
				Inodo.ctime = time(NULL);
				if(nivel == nivelFinal){
					Inodo.punterosIndirectos[nivel-1] = ptr;
				}else{
					buffer[i] = ptr;
					if(bwrite(ptrPre, &buffer) < 0){
						return -1;
					} 
				}
			}
		}
		memset(buffer, 0, (BLOCKSIZE/sizeof(unsigned int)));
		if(bread(ptr, &buffer) < 0) return -1;
		i = obtenerIndice(blogico, nivel);
		ptrPre = ptr;
		ptr = buffer[i];
		nivel--;
	}
	if(ptr == 0){
		if(!reservar){ 
			return -1;
		}else{ //si no hay que reservar no hace falta reservar bloques
			ptr = reservar_bloque();
			Inodo.numBloquesOcupados++;
			Inodo.ctime = time(NULL);
			if(nivelFinal == 0){
				Inodo.punterosDirectos[blogico] = ptr;
			}else{
				buffer[i] = ptr;
				if(bwrite(ptrPre, &buffer) < 0){
					return -1;
				} 
			}
		}
	}
	escribir_inodo(Inodo, ninodo);
	return ptr;
}

unsigned int obtenerRangoBL(struct inodo Inodo, unsigned int blogico, int *ptr){
	int npunteros = BLOCKSIZE/sizeof(unsigned int); // 256
	int directos = 12;
	int indirectos0 = npunteros+directos;
	int indirectos1 = npunteros*npunteros+indirectos0;
	int indirectos2 = npunteros*npunteros*npunteros+indirectos1;
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


int obtenerIndice(unsigned int blogico, unsigned int nivel){
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
		if(nivel == 2){
			return ((blogico-indirectos0)/(npunteros));
		}else if(nivel == 1){
			return ((blogico-indirectos0)%(npunteros));
		}
	}else if(blogico < indirectos2){
		if(nivel == 3){
			return ((blogico-indirectos1)/(npunteros*npunteros));
		}else if(nivel == 2){
			return (((blogico-indirectos1)%(npunteros*npunteros))/npunteros);
		}else if(nivel == 1){
			return (((blogico-indirectos1)%(npunteros*npunteros))%npunteros);
		}
	}
}

int vaciar_nivel(int nivel, int ptr, int primero, int blogico, int *bliberados){ //funcion recursiva de liberar bloques inodo
	
	unsigned int buffer[BLOCKSIZE/sizeof(unsigned int)];
	memset(buffer, 0, sizeof(buffer));

	unsigned int bufferAux[BLOCKSIZE/sizeof(unsigned int)];
	memset(bufferAux, 0, sizeof(bufferAux));

	unsigned int vacio[BLOCKSIZE/sizeof(unsigned int)];
	memset(vacio, 0, sizeof(vacio));

	if(ptr != 0){
		if(nivel == 1){ // ultimo nivel
			bread(ptr, buffer);
			int x;
			x = obtenerIndice(blogico, nivel);
			while(x < 256){
				if(buffer[x] != 0){
					bwrite(buffer[x], vacio);
					liberar_bloque(buffer[x]);
					buffer[x] = 0;
					(*bliberados)++;
				}
				x++;
			}
			bwrite(ptr, buffer);
		}else{ //nivel intermedio
			bread(ptr, buffer);
			int x;
			if(primero == 1){
				x = obtenerIndice(blogico, nivel);
			}else{
				x = 0;
			}
			nivel--;
			while(x < 256){
				if(buffer[x] != 0){
					vaciar_nivel(nivel, buffer[x], primero, blogico, bliberados);
					primero = 0;
					bread(buffer[x], bufferAux);
					if(memcmp(bufferAux, vacio, sizeof(bufferAux)) == 0){
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
	
	liberar_bloques_inodo(ninodo, 0); //liberar los bloques del inodo
	
	struct superbloque SB;
	bread(posSB, &SB);
	struct inodo Inodo = leer_inodo(ninodo); //modificacion de la informacion en el array de inodos
	Inodo.tipo = 'l';
	Inodo.punterosDirectos[0] = SB.posPrimerInodoLibre; //seguimos con la cadena de libres (lista)
	SB.posPrimerInodoLibre = ninodo; //el nuevo libre es el que acabamos de liberar
	SB.cantInodosLibres++;
	escribir_inodo(Inodo, ninodo);
	bwrite(posSB, &SB);
	return ninodo;
	
}

int liberar_bloques_inodo(unsigned int ninodo, unsigned int blogico){
	struct inodo Inodo = leer_inodo(ninodo);
	int ptr;
	int liberados = 0;
	int nivelFinal = obtenerRangoBL(Inodo, blogico, &ptr);
	int nivel = nivelFinal;
	unsigned int bufferaux[BLOCKSIZE/sizeof(unsigned int)];
	unsigned int vacio[BLOCKSIZE/sizeof(unsigned int)];
	memset(vacio, 0, sizeof(vacio));
	memset(bufferaux, 0, sizeof(bufferaux));
	int i, j;
	if(nivelFinal == 0){
		for(i = blogico; i < 12; i++){
			if(Inodo.punterosDirectos[i] != 0){
				liberar_bloque(Inodo.punterosDirectos[i]);
				Inodo.punterosDirectos[i] = 0;
				liberados++;
			}
		}
		nivel++;
	}
	for(j = nivel; j < 4; j++){
		if(Inodo.punterosIndirectos[j-1] != 0){
			vaciar_nivel(j, Inodo.punterosIndirectos[j-1], 1, blogico, &liberados);
			bread(Inodo.punterosIndirectos[j-1], bufferaux);
			if(memcmp(bufferaux, vacio, sizeof(bufferaux)) == 0){
				liberar_bloque(Inodo.punterosIndirectos[j-1]);
				Inodo.punterosIndirectos[j-1] = 0;
				liberados++;
			}
		}
	}
	
	Inodo.numBloquesOcupados = Inodo.numBloquesOcupados - liberados;
	Inodo.ctime = time(NULL); //actualizar tiempo
	escribir_inodo(Inodo, ninodo);
	return 0;
}