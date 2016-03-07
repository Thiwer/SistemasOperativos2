#include "ficheros_basico.h"

int tamMB(unsigned int nbloques){
	int bloques;
	if (((nbloques/8)%BLOCKSIZE) == 0)	{
		bloques = (nbloques/8)/BLOCKSIZE;
	} else {
		bloques = (nbloques/8)/BLOCKSIZE + 1;
	}
	return bloques;
}

int tamAI(unsigned int ninodos){
	int inodos;
	if (((ninodos * T_INODO) % BLOCKSIZE) > 0) {
		inodos = (ninodos / (BLOCKSIZE / 128)) + 1;
	} else {
		inodos = (ninodos / (BLOCKSIZE / 128));
	}
	return inodos;
}

int initSB(unsigned int nbloques, unsigned int ninodos){
	struct superbloque SB;
	unsigned int tamSB = 1;

	SB.posPrimerBloqueMB = posSB + tamSB;
	SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques);
	SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
	SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos);
	SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI +1;
	SB.posUltimoBloqueDatos = nbloques - 1;
	SB.posInodoRaiz = 0; //respecto a los inodos
	SB.posPrimerInodoLibre = 0; //se actualizara al crear el inod raiz
	SB.cantBloquesLibres = nbloques - tamSB - tamMB(nbloques) - tamAI(ninodos);
	SB.cantInodosLibres = ninodos - 1; //el primero es para la raiz
	SB.totBloques = nbloques;
	SB.totInodos = ninodos;
	if (bwrite(posSB, &SB) < 0){
		printf("Error al escribir el super bloque\n");
		return -1;
	}
	return 0;
}

int initMB(unsigned int nbloques){ //puede ir sin parametro
	struct superbloque SB;
	unsigned char buffer[BLOCKSIZE];
	memset(buffer, 0, BLOCKSIZE);
	int i;

	if (bread(posSB, &SB) < 0){
		printf("Error al leer el super bloque en initMB\n");
		return -1;
	}

	for (i = SB.posUltimoBloqueMB; i < SB.posUltimoBloqueMB; ++i){
		if (bwrite(i, buffer) < 0){
			printf("Error al inicializar el mapa de bits en initMB\n");
			return -1;
		}
	}
	return 0;
}

int initAI(unsigned int ninodos){
	struct superbloque SB;
	if (bread(posSB, &SB) < 0){
		printf("Error al leer el super bloque en initAI\n");
		return -1;
	}
	struct inodo Inodos[BLOCKSIZE/T_INODO];
	int i;
	int x = 1;
	for (i = SB.posPrimerBloqueAI; i < SB.posUltimoBloqueAI; ++i){
		int j;
		for (j = 0; j < BLOCKSIZE/T_INODO; ++j){
			Inodos[j].tipo = 'l';
			if (x < ninodos){
				Inodos[j].punterosDirectos[0] = x;
				x++;
			} else  {
				Inodos[j].punterosDirectos[0] = UINT_MAX;
				j = BLOCKSIZE/T_INODO; //forzar la salida
			}
		}
		if (bwrite(i, Inodos) < 0){
			printf("Error al escribir el inodo en initAI\n");
		}
	}
}

//Hasta aqui la etapa 2

int escribir_bit(unsigned int nbloque, unsigned int bit){
	if (bit < 0 || bit > 1){
		printf("El bit introducido no es correcto, escribir bit\n");
		return -1;
	}
	struct superbloque SB;
	if (bread(posSB, &SB) < 0){
		printf("Error al leer el super bloque en initAI\n");
		return -1;
	}
	unsigned char mascara = 128;
	int posbyte = nbloque / 8;
	int posbit = nbloque % 8;
	int bloque_MB = nbloque/(BLOCKSIZE*8);
	unsigned char buffer[BLOCKSIZE];
	memset(buffer, 0, BLOCKSIZE);

	bread(SB.posPrimerBloqueMB +bloque_MB, buffer); //escribimos en la posicion indicada

	//factor comun
	mascara = mascara >> posbit;

	if (bit == 0){
		buffer[posbyte%BLOCKSIZE] &= ~mascara;
	} else if (bit == 1){
		buffer[posbyte%BLOCKSIZE] |= mascara;
	}
	return bwrite(SB.posPrimerBloqueMB + bloque_MB, buffer);
}


unsigned char leer_bit(unsigned int nbloque){
	struct superbloque SB;
	if (bread(posSB, &SB) < 0){
		printf("Error al leer el super bloque en leer_bit\n");
		return -1;
	}
	unsigned char mascara = 128;
	unsigned char buffer[BLOCKSIZE];
	memset(buffer, 0, BLOCKSIZE);
	int posbyte = (nbloque/8)%BLOCKSIZE;
	int posbit = nbloque % 8;
	unsigned int bl = nbloque/(BLOCKSIZE*8);
	mascara = mascara >> posbit;
	if(bread(SB.posPrimerBloqueMB + bl, buffer) < 0){
		return -1;
	}
	mascara = mascara & buffer[posbyte]; // Realizamos operacion AND binario de la mascara con el byte correspondiente
	mascara = mascara >> (7-posbit); // Desplazamos el bit hacia la derecha
	return mascara;

}

int reservar_bloque(){
	struct superbloque SB;
	if (bread(posSB, &SB) < 0){
		printf("Error al leer el super bloque en reservar_bloque\n");
		return -1;
	}
	if (SB.cantBloquesLibres == 0){
		printf("No quedan bloques libres\n");
		return -1;
	}
	unsigned char mascara = 128;
	unsigned char buffer[BLOCKSIZE];
	memset(buffer, 0, BLOCKSIZE);
	unsigned char bufferAux[BLOCKSIZE];
	memset(bufferAux, 255, BLOCKSIZE);
	int encontrado = 0;
	int i = SB.posUltimoBloqueMB;
	int j;

	int pos = 0;
	int posbyte = 0;
	int posbit = 0;

	while(!encontrado && i <= SB.posUltimoBloqueMB){ //buscar los bloques candidatos
		if (bread(i, &buffer) < 0){
			printf("Error al leer en reservar_bloque\n");
			return -1;
		}
		if (memcmp(buffer, bufferAux, BLOCKSIZE) != 0){
			int encontradoDentro = 0; //para buscar dentro del bloque
			j = 0;
			while(!encontradoDentro && j < BLOCKSIZE){
				if (buffer[j] < 255){
					while(buffer[j] & mascara){
						buffer[j] <<= 1;
						posbit++;					
					}
					posbyte = j;
					pos = i;	
					encontradoDentro = 1;
				}
				j++;
			}
			encontrado = 1;
		}
		i++;
	}
	unsigned int nbloque = ((pos-SB.posPrimerBloqueMB)*BLOCKSIZE + posbyte)*8 + posbit;
	escribir_bit(nbloque, 1);
	SB.cantBloquesLibres=SB.cantBloquesLibres - 1;
	if (bwrite(posSB, &SB) != 0){
		printf("Error al escribir el super bloque en reservar_bloque\n");
		return -1;
	}
	return nbloque;
}

int liberar_bloque(unsigned int nbloque){
	struct superbloque SB;
	if (bread(posSB, &SB) < 0){
		printf("Error al leer el super bloque en reservar_bloque\n");
		return -1;
	}
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
	if (bread(posSB, &SB) < 0){
		printf("Error al leer el super bloque en escribir_inodo\n");
		return -1;
	}
	int nbloque = SB.posPrimerBloqueAI + ninodo/(BLOCKSIZE/T_INODO);
	struct inodo arrayInodos[BLOCKSIZE/T_INODO];
	bread(nbloque, &arrayInodos);
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
	if (bread(posSB, &SB) < 0){
		printf("Error al leer el super bloque en reservar_bloque\n");
		return -1;
	}
	if (SB.cantInodosLibres == 0){
		printf("No quedan inodos libres\n");
		return -1;
	}
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
	escribir_inodo(Inodo, SB.posPrimerInodoLibre);
	//Actualizacion de la lista enlazada de inodos
	SB.cantInodosLibres--;
	SB.posPrimerInodoLibre = siguientelibre;
	if (bwrite(posSB, &SB) < 0){
		printf("Error al escribir el super bloque en reservar_inodo\n");
		return -1;
	}
	return ninodo;
}

//Hasta aqui la tercera etapa
//Y empieza la cuarta etapa


int traducir_bloque_inodo(unsigned int ninodo, unsigned int blogico, unsigned char reservar){
	
	struct inodo Inodo = leer_inodo(ninodo);
	int ptr = 0;
	int ptr_ant = 0;
	int salvar_inodo = 0;
	int level = encontrarNivel(Inodo, blogico, &ptr);
	int levelfinal = level;
	int i;
	unsigned int buffer[BLOCKSIZE/sizeof(unsigned int)];
	while(level > 0){
		if (ptr == 0){
			if (!reservar){
				printf("Error, no se reservara.\n");
				return -1;
			} else { // si que hay que reservar
				salvar_inodo = 1;
				ptr = reservar_bloque();
				Inodo.numBloquesOcupados++;
				Inodo.ctime = time(NULL);
				if(level == levelfinal){
					Inodo.punterosIndirectos[level-1] = ptr;
				} else {
					buffer[i] = ptr;
					if(bwrite(ptr_ant, &buffer) < 0){
						printf("Error en bwrite en traducir_bloque_inodo\n");
						return -1;
					}
				}

			}
		}
		memset(buffer, 0, (BLOCKSIZE/sizeof(unsigned int)));
		if (bread(ptr, &buffer) < 0){
			printf("Error en bread en traducir_bloque_inodo\n");
			return -1;
		}
		i = get_index(blogico, level);
		ptr_ant = ptr;
		ptr = buffer[i];
		level--;
	}

	if (ptr = 0){
		if (!reservar){
			printf("Error, no se reservara.\n");
			return -1;
		} else {
			salvar_inodo = 1;
			ptr = reservar_bloque();
			Inodo.numBloquesOcupados++;
			Inodo.ctime = time(NULL);
			if(levelfinal == 0){
				Inodo.punterosIndirectos[level-1] = ptr;
			} else {
				buffer[i] = ptr;
				if(bwrite(ptr_ant, &buffer) < 0){
					printf("Error en bwrite en traducir_bloque_inodo\n");
					return -1;
				}
			}
		}



	}
	if (salvar_inodo == 1){
		if (escribir_inodo(Inodo, ninodo) < 0){
			printf("Error al escribir_inodo en traducir_bloque_inodo\n");
		}
	}
	return ptr;
}



int liberar_inodo(unsigned int ninodo){
	liberar_bloques_inodo(ninodo, 0);
	struct superbloque SB;
	if (bread(posSB, &SB) < 0){
		printf("Error al leer el super bloque en liberar_inodo\n");
		return -1;
	}
	struct inodo Inodo = leer_inodo(ninodo);
	Inodo.tipo = 'l';
	Inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;
	SB.posPrimerInodoLibre = ninodo;
	SB.cantInodosLibres++;
	escribir_inodo(Inodo, ninodo);
	if (bwrite(posSB, &SB) < 0){
		printf("Error al escribir el inodo en liberar_inodo\n");
		return -1;
	}
	return ninodo;
}

int liberar_bloques_inodo(unsigned int ninodo, unsigned int blogico){
	
	
}

//Funciones auxiliares de la cuarta etapa

unsigned int encontrarNivel(struct inodo Inodo, unsigned int blogico, int *ptr){ //obtener_rangoBL
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

   int get_index(unsigned int blogico, unsigned int level){
	int npunteros = BLOCKSIZE/sizeof(unsigned int); // 256
	int directos = 12;
	int indirectos0 = npunteros + directos; //268
	int indirectos1 = npunteros*npunteros + indirectos0; //65.804
	int indirectos2 = npunteros*npunteros*npunteros + indirectos1; //16.843.020
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
	
	
}
