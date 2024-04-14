#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

/*
    Subprograma para comprobar el funcionamiento de get_digits, en especial, la escritura y el manejo de los ficheros.

    NOTA: Recuerda asignar las variables globales my_path y n_acad correctamente antes de ejecutar en get_digits.c
    NOTA: Asignar el nombre de la carpeta de weights y biases ↓ ↓ 
*/
char* weights_y_biases = "../Datos"; // Literalmente el nombre de la carpeta donde estan guardados los modelos (el ultimo "/esto/" de my_path)

int main(int argc, char* argv[]){
    if(argc != 2){
        fprintf(stderr, "ERROR: numero de argumentos inválido. Has de introducir el numero de paralelizaciones a realizar.\n");
        return(1);
    }

    fprintf(stdout, "\n---Realizando la ejecución de get_digits.c---\n\n");
    char* comando = malloc(100);

    // Ejecutar get_digits con time
    sprintf(comando, "time ./get_digits %d", atoi(argv[1]));
    system(comando);
    sleep(2);

    // Mostrar el wc de la carpeta resultados
    fprintf(stdout, "\nMostrando [numero de lineas | palabras | bytes] de los ficheros resultado\n");
    sprintf(comando, "wc %s/resultados/*", weights_y_biases);
    system(comando);
    sleep(2);

    // Limpiar contenido resultados
    // NOTA: Mejor pedimos confirmacion a ver si vamos a borrarle algo que no debemos a alguien
    fprintf(stdout, "\nDeseas eliminar los ficheros de el directorio resultados (en principio) creado por el programa? (1 - si, otro numero - no): ");
    int confirmacion;
    scanf("%d", &confirmacion);
    if (confirmacion == 1)
    {
        // LIMPIAR
        sprintf(comando, "rm %s/resultados/*", weights_y_biases);
        system(comando);

        // Confirmacion visual de que se ha limpiado bien la carpeta resultados
        fprintf(stdout, "\nMostrando el numero de elementos del directorio resultados (deberia de ser 0 si se ha borrado)\n");
        sprintf(comando, "ls -1 %s/resultados | wc -l", weights_y_biases);
        system(comando);
        fprintf(stdout, "\n");
    }
    else printf("Saltando limpieza de resultados...\n");
    sleep(2);
    
    free(comando);
    return(0);
}
