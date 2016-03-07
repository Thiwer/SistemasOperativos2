#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> /* Modos de apertura y función open()*/
#include <stdio.h>
#include <string.h>

#define BLOCKSIZE 1024 //bytes

int bmount(const char *camino);
int bumount();
int bwrite(unsigned int nbloque, const void *buf);
int bread(unsigned int nbloque, void *buf);


