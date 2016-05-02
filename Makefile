main: bloques.o ficheros.o mi_mkfs.o leer_SF.o ficheros_basico.o leer.o escribir.o escribir.o directorios.o
	gcc -o mi_mkfs bloques.o ficheros.o mi_mkfs.o ficheros_basico.o -g
	gcc -o leer_SF leer_SF.o ficheros_basico.o ficheros.o bloques.o -g
	gcc -o escribir escribir.o ficheros_basico.o ficheros.o bloques.o -g
	gcc -o leer leer.o ficheros_basico.o ficheros.o bloques.o -g
	
leer: leer.o ficheros_basico.o ficheros.o bloques.o
	gcc -o leer leer.o ficheros_basico.o ficheros.o bloques.o -g 

escribir: escribir.o ficheros_basico.o ficheros.o bloques.o
	gcc -o escribir escribir.o ficheros_basico.o ficheros.o bloques.o -g
	
bloques.o: bloques.c bloques.h
	gcc -c bloques.c -g 

ficheros_basico.o: ficheros_basico.c ficheros_basico.h
	gcc -c ficheros_basico.c -g
	
ficheros.o: ficheros.c ficheros.h
	gcc -c ficheros.c -g

mi_mkfs.o: mi_mkfs.c ficheros_basico.h
	gcc -c mi_mkfs.c -g
	
leer_SF.o: leer_SF.c ficheros_basico.h
	gcc -c leer_SF.c -g

escribir.o: escribir.c ficheros.h
	gcc -c escribir.c -g

leer.o: leer.c ficheros.h
	gcc -c leer.c -g

directorios.o: directorios.c directorios.h
	gcc -c directorios.c -g

	
.PHONY: clean
clean:
	rm -f *.o

