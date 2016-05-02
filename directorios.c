#include "directorios.h"

int error(int error) { //devuelve el tipo de error que hemos tenido
	if(error < 0){
		switch(error){
			case -1: printf("Error al extraer camino\n"); break;
			case -2: printf("Sin permisos de lectura\n"); break;
			case -3: printf("No existe entrada\n"); break;
			case -4: printf("No existe directorio intermedio\n"); break;
			case -5: printf("Sin permisos escritura\n"); break;
			case -6: printf("Entrada ya existe\n"); break;
		}
		return -1;
	}
	return 0;
}

int extraer_camino(const char *camino, char *inicial, char *final){
	int fichero;
	int i = 0;
	int j = 0;
	int k = 0;
	if ((camino[i] == '/')) // 0 como caracter de final
	{
		i++;
		while((camino[i] != '/')&& (camino[i] != 0)){
			inicial[j] = camino[i];
			i++;
			j++;
		}
		if (camino[i] != '/') //fichero
		{
			fichero = 1;
		} else {
			fichero = 0;
		}
		while(camino[i] != 0){
			final[k] = camino[i];
			i++;
		}
	}
	return fichero;

}

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos){
	
	/*
La función buscar_entrada, dada una cadena de caracteres (camino_parcial) y el inodo del directorio sobre el que se apoya esta cadena (p_inodo_dir), calcula:

    * El número de inodo de su directorio más cercano (p_inodo_dir).
    * Su número de inodo (p_inodo).
    * El número de su entrada dentro del último directorio que lo contiene (p_entrada).
*/

    //lista de errores
    int errorExtraerCamino = -1;
	int sinPermisosLectura = -2;
	int noExisteEntrada = -3;
	int noExisteDirIntermedio = -4;
	int sinPermisosEscritura = -5;
	int entradaYaExiste = -6;
	struct inodo Inodo;
	struct entrada Entrada;
	int nbloque = 0;
	int numbloques = 0;

	char inicial[60];
	char final[60];
	char empty[60];
	memset(inicial, 0, sizeof(inicial));
	memset(final, 0, sizeof(final));
	memset(empty, 0, sizeof(empty));


    if (camino_parcial == "/")
    {
     	*p_inodo = 0;  //la raiz siempre estara asociada al inodo 0
        *p_entrada = 0;
        return 0;	
    }

    int tipo = extraer_camino(camino_parcial, inicial, final);
    if (tipo < 0)
    {
    	return errorExtraerCamino;
    }
		
    //buscamos la entrada cuyo nombre se encuentra en inicial
    Inodo = leer_inodo(*p_inodo_dir);
    memset(Entrada.nombre, '\0', sizeof(Entrada.nombre));

    //calcular el nº de entradas del inodo
    if (Inodo.tamEnBytesLog % BLOCKSIZE == 0) //
    {
    	numbloques = Inodo.tamEnBytesLog / BLOCKSIZE;
    } else {
    	numbloques = (Inodo.tamEnBytesLog / BLOCKSIZE) + 1;
    }


    int numentradas = Inodo.tamEnBytesLog/(sizeof(struct entrada));
    int nentrada = 0;  //nº de entrada
    int existe = 0;

    if (numentradas > 0)
    {
    	struct entrada arrayEntradas[BLOCKSIZE/(sizeof(struct entrada))]; //con 1024 -> 16
    	memset(arrayEntradas,0,sizeof(arrayEntradas));
    	if (mi_read_f(*p_inodo_dir, &arrayEntradas,nbloque*BLOCKSIZE,BLOCKSIZE) < 0) //nbloque*BLOCKSIZE siempre sera 0
    	{
    		return sinPermisosLectura;
    	}
    	int salir = 0;
    	while((nbloque < numbloques)&&(salir == 0)){
    		int i = 0;
    		while((i < (BLOCKSIZE/(sizeof(struct entrada))))&&(salir == 0)){
    			struct inodo inAux = leer_inodo(arrayEntradas[i].inodo);
    			int tipoInodo;
    			if (inAux.tipo == 'd')
    			{
    				tipoInodo = 0;
    			} else {
    				tipoInodo = 1;
    			}
    			if ((strcmp(empty, arrayEntradas[i].nombre) == 0)&&(tipoInodo == tipo)) //tipo proviene de extraer camino
    			{
    				Entrada = arrayEntradas[i];
    				salir = 1;
    				existe = 1;
    			} else if (strcmp(inicial,arrayEntradas[i].nombre) == 0)
    			{
    				salir = 1;
    			} else {
    				i++;
    				nentrada++;
    			}
    		if (salir != 1)
    		{
    				nbloque++;
    				memset(arrayEntradas,0,sizeof(arrayEntradas));
    				mi_read_f(*p_inodo_dir, &arrayEntradas, nbloque*BLOCKSIZE, BLOCKSIZE);
    		}
    	}
    }

    if (existe == 0)
    {
    	if (reservar == '0')
    	{
    		return noExisteEntrada;
    	}
    	strcpy(Entrada.nombre, inicial);
    	if (tipo == 0)
    	{
    		if (strcmp(final, "/") == 0 )
    		{
    			Entrada.inodo = reservar_inodo('d', permisos);
    		} else {
    			return noExisteDirIntermedio;
    		}
    	} else {
    		Entrada.inodo = reservar_inodo('f', permisos);
    	}
    	if (mi_write_f(*p_inodo_dir, &Entrada, nentrada*sizeof(struct entrada), sizeof(struct entrada) < 0))
    	{
    		if (Entrada.inodo != -1)
    		{
    			liberar_inodo(Entrada.inodo);
    		}
    		return sinPermisosEscritura;
    	}
    }

    if ((strcmp(final, "/") == 0) || strcmp(final, "") == 0)
    {
    	if ((existe == 1) && (reservar == '1'))
    	{
    		return entradaYaExiste;
    	}
    	*p_inodo = Entrada.inodo;
    	*p_entrada = nentrada;
    	return 0;
    } else {
    	*p_inodo_dir = Entrada.inodo;
    	return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }

}

//siguiente etapa

int mi_creat(const char *camino, unsigned char permisos){

	int p_inodo_dir = 0;
	int p_inodo = 0;
	int p_entrada = 0;
	//comprobar permisos
	if ((buscar_entrada(camino, p_inodo_dir, p_inodo, p_entrada, 1, permisos)) < 0)
	{
		printf("Error en mi_creat\n");
		return -1;
	}
	return 0;

}

int mi_dir(const char *camino, char *buffer){
	int p_inodo_dir = 0;
	int p_inodo = 0;
	int p_entrada = 0;
	//comprobar permisos
	int resultado = buscar_entrada(camino, p_inodo_dir, p_inodo, p_entrada, 1, permisos);
	if ( resultado < 0)
	{
		error(resultado);
		printf("Error en mi_chmod\n");
		return -1;
	}
	struct inodo Inodo = leer_inodo(p_inodo);
	if (Inodo.tipo != 'd')
	{
		printf("No es un directorio\n");
        return -1;
	}
	if ((Inodo.permisos & 4) != 4) { //comprobamos que tenga permisos de lectura
        printf("No tienes permisos de lectura\n");
        return -1;
    } 
    
    struct entrada entrada;
    int netrada;
    int numEntradas = Inodo.tamEnBytesLog / sizeof(struct entrada);
    //for each entrada
    for (nentrada = 0; nentrada < numEntradas; nentrada++)
    {
    	mi_read_f(p_inodo, &entrada, nentrada*sizeof(struct entrada), sizeof(struct entrada));
    	strcat(buffer, " | ");
    	strcat(buffer, entrada.nombre);
    	strcat(buffer, " | ");

    	Inodo = leer_inodo(entrada.inodo);
    	if (Inodo.tipo == 'd')
    	{
    		strcat(buffer,"Directorio");
    		strcat(buffer, " | ");
    	} else if (Inodo.tipo == 'f')
    	{
    		strcat(buffer, "fichero");
    		strcat(buffer," | ");
    	}

    	if (Inodo.permisos & 4)
    	{
    		strcat(buffer, "r");
    	} else {
    		strcat(buffer, "-");
    	}

    	if (Inodo.permisos & 2)
    	{
    		strcat(buffer, "w");
    	} else {
    		strcat(buffer, "-");
    	}

    	if (Inodo.permisos & 1)
    	{
    		strcat(buffer, "x");
    	} else {
    		strcat(buffer, "-");
    	}
    	strcat(buffer, " | ");

    	struct tm *tm; //ver info: struct tm
		char tmp[100]:
		tm = localtime(&inodo.mtime);
		sprintf(tmp,"%d-%02d-%02d %02d:%02d:%02d\t",tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
		strcat(buffer,tmp);
		strcat(buffer, " | ");
    }







}



int mi_link(const char *camino1, const char *camino2){

}

int mi_unlink(const char *camino){

}

int mi_chmod(const char *camino, unsigned char permisos){
	int p_inodo_dir = 0;
	int p_inodo = 0;
	int p_entrada = 0;
	//comprobar permisos
	int resultado = buscar_entrada(camino, p_inodo_dir, p_inodo, p_entrada, 1, permisos);
	if ( resultado < 0)
	{
		error(resultado);
		printf("Error en mi_chmod\n");
		return -1;
	}
	mi_chmod_f(p_inodo, p_stat);
}

int mi_stat(const char *camino, struct STAT *p_stat){
	
	int p_inodo_dir = 0;
	int p_inodo = 0;
	int p_entrada = 0;
	//comprobar permisos
	int resultado = buscar_entrada(camino, p_inodo_dir, p_inodo, p_entrada, 1, permisos);
	if ( resultado < 0)
	{
		error(resultado);
		printf("Error en mi_stat\n");
		return -1;
	}
	mi_stat_f(p_inodo, p_stat);

}

int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes){
	int p_inodo_dir = 0;
	int p_inodo = 0;
	int p_entrada = 0;
	//comprobar permisos
	int resultado = buscar_entrada(camino, p_inodo_dir, p_inodo, p_entrada, 1, permisos);
	if ( resultado < 0)
	{
		error(resultado);
		printf("Error en mi_read\n");
		return -1;
	}
	mi_read_f(p_inodo, p_stat);
}

int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes){
	int p_inodo_dir = 0;
	int p_inodo = 0;
	int p_entrada = 0;
	//comprobar permisos
	int resultado = buscar_entrada(camino, p_inodo_dir, p_inodo, p_entrada, 1, permisos);
	if ( resultado < 0)
	{
		error(resultado);
		printf("Error en mi_write\n");
		return -1;
	}
	mi_write_f(p_inodo, p_stat);
}

	
	
	
	
}

