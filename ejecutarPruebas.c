#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

/*
    Subprograma para comprobar el funcionamiento de get_digits, en especial, la escritura y el manejo de los ficheros.
*/
int main(int argc, char* argv[]){
    if(argc != 2){
        fprintf(stderr, "ERROR: numero de argumentos inválido. Has de introducir el numero de paralelizaciones a realizar.\n");
        return(1);
    }
    fprintf(stdout, "\n---Realizando la ejecución de get_digits.c---\n\n");
    char* comando = malloc(100);
    int numHijos = strtod(argv[1], NULL);
    sprintf(comando, "time ./get_digits %d", numHijos);
    system(comando);
    sleep(2);

    fprintf(stdout, "\nMostrando [numero de lineas | palabras | bytes] de los ficheros resultado\n");
    sprintf(comando, "wc ../Resultados/*"); //Poner la ruta correspondiente
    system(comando);
    sleep(2);

    fprintf(stdout, "\nEliminando los ficheros creados en la ejecuccion del programa...\n");
    sprintf(comando, "rm ../Resultados/*"); //Poner la ruta correspondiente
    system(comando);
    sleep(2);

    fprintf(stdout, "\nMostrando el numero de elementos del directorio ../Resultados\n");
    sprintf(comando, "ls -1 ../Resultados | wc -l"); //Poner la ruta correspondiente
    system(comando);
    fprintf(stdout, "\n");
    
    free(comando);
    return(0);
}
