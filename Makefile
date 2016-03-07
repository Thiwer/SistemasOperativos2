main: bloques.o mi_mkfs.o leer_SF.o ficheros_basico.o
	gcc -o mi_mkfs bloques.o mi_mkfs.o ficheros_basico.o -g
	gcc -o leer_SF leer_SF.o ficheros_basico.o bloques.o -g	
	
bloques.o: bloques.c bloques.h
	gcc -c bloques.c -g 

ficheros_basico.o: ficheros_basico.c ficheros_basico.h
	gcc -c ficheros_basico.c -g

mi_mkfs.o: mi_mkfs.c ficheros_basico.h
	gcc -c mi_mkfs.c -g

leer_SF.o: leer_SF.c ficheros_basico.h
	gcc -c leer_SF.c -g

.PHONY: clean
clean:
	rm -f *.o
	
cleanSF:
	rm -f *.imagen

