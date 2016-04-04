#include "bloques.h"
#include "ficheros_basico.h"

struct STAT {
	unsigned char tipo;
	unsigned char permisos;
	unsigned char reservado_alineacion[2]; //alineacion
	time_t atime;
	time_t mtime;
	time_t ctime;
	unsigned int nlinks;
	unsigned int tamEnBytesLog;
	unsigned int numBloquesOcupados;

	// char padding[T_INODO-2*sizeof(unsigned char)-3*sizeof(time_t)-20*sizeof(unsigned int)]; 
};

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes);
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes);

int mi_chmod_f(unsigned int ninodo, unsigned char permisos);
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes);
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat);


