#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> // Modos de apertura y función open()
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#define BLOCKSIZE 1024 //bytes

int bmount(const char *ruta);
int bumount();
int bwrite(unsigned int bloque, const void *buf);
int bread(unsigned int bloque, void *buf);