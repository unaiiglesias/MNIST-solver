#define _GNU_SOURCE
#include <stdio.h>  // file handling functions
#include <stdlib.h> // atoi
#include <string.h> // strtok
#include <sched.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h> // crear_directorio_resultados
#define stacksize 1048576

double **data;
int data_nrows;
int data_ncols = 784;
char* my_path = "/home/dorron/01_Programas/IRCSO/Datos/";  // Ruta absoluta del directorio de weights y biases (terminado en /)
char* path_resultados; // Se asigna al crear el directorio
char* n_acad = "11"; // Hace referencia al numero de weights y biases que se va a usar (en este caso el de acad de Iglesias)

int seed = 0;
int matrices_rows[4] = {784, 200, 100, 50};
int matrices_columns[4] = {200, 100, 50, 10};
int vector_rows[4] = {200, 100, 50, 10};
char *str;

int rows_per_div;
static double *digits;
static double **mat1;
static double **mat2;
static double **mat3;
static double **mat4;
static double *vec1;
static double *vec2;
static double *vec3;
static double *vec4;

int read_matrix(double** mat, char* file, int nrows, int ncols, int fac){
    /*
     * Dada una matrix (mat), un nombre de fichero (file), una cantidad de filas (nrows) y columnas (ncols), y un multiplicador (no se usa),
     * deja en mat la matriz (de dimensión nrows x ncols) de datos contenida en el fichero con nombre file
     */

    char *buffer = malloc(2048);
    char *record;
    FILE *fstream = fopen(file,"r");
    double aux;
    if(fstream == NULL)
    {
        fprintf(stderr, "\n file opening failed %s ", file);
        return -1 ;
    }
    for(int row=0; row<nrows; row++)
    {
        fgets(buffer,2048,fstream);
        record = strtok(buffer," ");
        mat[row] = malloc(ncols * sizeof(double));
        for(int column=0;column<ncols;column++)
        {
            if(record){
                aux = strtod(record, NULL)*(float)fac;
                mat[row][column] = aux;
            }
            else {
                mat[row][column] = -1.0;
            }
            record = strtok(NULL," ");
        }
    }
    fclose(fstream);
    free(buffer);
    return 0;
}

int read_vector(double* vect, char* file, int nrows) {
    /*
     * Dado un vector (vect), un nombre de fichero (file), y una cantidad de filas (nrows),
     * deja en vect el vector (de dimensión nrows) de datos contenido en el fichero con nombre file
     */
    char *buffer = malloc(2048);
    FILE *fstream = fopen(file, "r");
    double aux;
    if (fstream == NULL) {
        fprintf(stderr, "\n file opening failed %s ", file);
        return -1;
    }
    for (int row = 0; row < nrows; row++) {
        fgets(buffer, 2048, fstream);

        aux = strtod(buffer, NULL);
        //printf("Buffer: %s, %f", buffer, aux);
        vect[row] = aux;
    }
    fclose(fstream);
    free(buffer);
    return 0;
}

int relu(double** mat, int nrows, int ncols){
    /*
     * Dada una matriz (mat) y sus dimensiones, aplica la función ReLU a todos sus elementos
     */
    for(int row=0;row<nrows;row++){
        for(int col=0;col<ncols;col++){
            if(mat[row][col]<0)
                mat[row][col] = 0;
        }
    }
    return 0;
}

int print_matrix(double** mat, int nrows, int ncols, int offset_row, int offset_col){
    /*
     * Dada una matriz (mat), una cantidad de filas (nrows) y columnas (ncols) a imprimir, y una cantidad de filas (offset_row)
     * y columnas (offset_col) a ignorar, imprime por salida estándar nrows x ncols de la matriz
     */
    for(int row=0;row<nrows;row++){
        for(int col=0;col<ncols;col++){
            printf("%f ", mat[row+offset_row][col+offset_col]);
        }
        printf("\n");
    }
    return 0;
}

int print_vector(double* vect, int nrows, int offset_row){
    /*
     * Loo mismo que print_matrix pero con vectores
     */
    for( int row = offset_row; row < nrows; row++){
        printf("%f \n", vect[row] );
        printf("\n");
    }
    return 0;

}

int matmul(double** matr1, double** matr2, double** res, int rows1_start, int rows1_end, int ncols1, int ncols2) {
    /*
     * Dadas dos matrices (mat1 y mat2), una matriz resultado (res), dos índices de filas de la primera matriz (rows1_start y rows_end),
     * y la cantidad de columnas de mat1 y mat2, calcula el producto matricial entre las filas contenidas entre rows1_start y rows1_end
     * de mat1, y mat2 (todas sus columnas).
     */
    double aux, a1, a2;
    for (int row = 0; row < rows1_end - rows1_start; row++) {
        res[row] = malloc(ncols2 * sizeof(double));
        for (int col2 = 0; col2 < ncols2; col2++) {
            aux = 0;
            for (int col1_row2 = 0; col1_row2 < ncols1; col1_row2++) {
                a1 = matr1[row + rows1_start][col1_row2];
                a2 = matr2[col1_row2][col2];
                aux += (a1 * a2);
            }
            res[row][col2] = aux;
        }
    }
    return 0;
}

int argmax(double** mat, int nrows, int ncols, int* predictions){
    /*
     * Dada una matriz de valores (mat) y sus dimensiones (nrows y ncols), y un vector vacío (predictions), esta función
     * calculará, para cada una de las filas de la matriz mat, la posición del valor más alto, y lo colocará en predictions
     */
    double max;
    int argmax;
    for(int row=0;row<nrows;row++){
        max = -9999999;
        argmax = -1;
        for(int col=0;col<ncols;col++){
            if(mat[row][col]>max) {
                max = mat[row][col];
                argmax = col;
            }
        }
        predictions[row] = argmax;
    }
    return 0;
}

int add_vector(double** mat, const double* vect, int nrows, int ncols){
    /*
     * Dada una matriz (mat), un vector (vect), la cantidad de filas de mat y vect, y de columnas de mat, suma los valores
     * de vect a cada columna de mat
     */
    for(int row=0;row<nrows;row++){
        for(int col=0;col<ncols;col++){
            mat[row][col] += vect[col];
        }
    }
    return 0;
}

int load_data(char* path){

    /*
     * Dado un directorio en el que están los datos y parámetros, los carga en las variables de entorno
     */

    // Cargar las prediciones que idealmente obtendremos (las soluciones) digits.csv
    digits = malloc(data_nrows * sizeof(double));
    sprintf(str, "%sdigits.csv", path);
    read_vector(digits, str, data_nrows);

    // mostramos las primeras 20 columnas de digits para ver que todo va bien
    //print_vector(digits, 20, 0);


    // Cargar data.csv (mega matriz con los numeros a procesar)
    data = malloc(data_nrows * data_ncols * sizeof(double));
    sprintf(str, "%sdata.csv", path);
    read_matrix(data, str, data_nrows, data_ncols, 1);

    // mostramos una submatrix 20x40 para comprobar (el offset de 70 porque a la izaquierda solo hay 0s)
    //print_matrix(data, 20, 40, 0, 70);


    // Cargar weights (weights[0, 1, 2, 3]_[Numero acad].csv)
    mat1 = malloc(matrices_rows[0] * matrices_columns[0] * sizeof(double));
    mat2 = malloc(matrices_rows[1] * matrices_columns[1] * sizeof(double));
    mat3 = malloc(matrices_rows[2] * matrices_columns[2] * sizeof(double));
    mat4 = malloc(matrices_rows[3] * matrices_columns[3] * sizeof(double));

    sprintf(str, "%sweights0_%s.csv", path, n_acad);
    read_matrix(mat1, str, matrices_rows[0], matrices_columns[0], 1);

    sprintf(str, "%sweights1_%s.csv", path, n_acad);
    read_matrix(mat2, str, matrices_rows[1], matrices_columns[1], 1);

    sprintf(str, "%sweights2_%s.csv", path, n_acad);
    read_matrix(mat3, str, matrices_rows[2], matrices_columns[2], 1);

    sprintf(str, "%sweights3_%s.csv", path, n_acad);
    read_matrix(mat4, str, matrices_rows[3], matrices_columns[3], 1);

    // Cargar biases (biases[0, 1, 2, 3]_[Numero acad].csv)
    vec1 = malloc(vector_rows[0] * sizeof(double));
    vec2 = malloc(vector_rows[1] * sizeof(double));
    vec3 = malloc(vector_rows[2] * sizeof(double));
    vec4 = malloc(vector_rows[3] * sizeof(double));

    sprintf(str, "%sbiases0_%s.csv", path, n_acad);
    read_vector(vec1, str, vector_rows[0]);

    sprintf(str, "%sbiases1_%s.csv", path, n_acad);
    read_vector(vec2, str, vector_rows[1]);

    sprintf(str, "%sbiases2_%s.csv", path, n_acad);
    read_vector(vec3, str, vector_rows[2]);

    sprintf(str, "%sbiases3_%s.csv", path, n_acad);
    read_vector(vec4, str, vector_rows[3]);

    return 0;
}

int unload_data(){
    /*
     * Descarga la memoria
     */

    free(vec1);
    free(vec2),
    free(vec3);
    free(vec4);
    free(mat1);
    free(mat2);
    free(mat3);
    free(mat4);
    free(digits);
    free(data);
    free(str);

    return 0;
}

double** reservar_matriz_nxm (double** mat, int n, int m)
{
    /*
        Dado un double** donde se pretende crear la matriz y sus dimensiones nxm (filas x columnas)
        reserva (malloc) suficiente espacio como para llenarla de doubles
    */

    mat = malloc(n * sizeof(double));
    for (int i = 0; i < n; i++) mat[i] = malloc(m * sizeof(double));
    return mat;
}

/*
    Crea (si no existe) el directorio my_path/resultados y asigna el valor a path_resultados

    Como usamos ficheros para guardar los resultados de cada hilo, es conveniente hacer una carpeta en la que guardarlos y borrarlos.
    (Por organizacion, mas que nada)
*/
void crear_directorio_resultados () {
    struct stat sb;
    char* comando;
    char* path = malloc(sizeof(char) * (strlen(my_path) + 20) ); // Ruta del directorio que queremos crear / comprobar que ya esta creado
    strcpy(path, my_path); strcat(path, "resultados");


    if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode))
    {
        // Ya existe el directorio, podemos usarlo, no hay que hacer nada
        ;
    } 

    else 
    {
        // Hay que crear el directorio
        comando = malloc(sizeof(char) * (strlen(my_path) + 20) );

        strcpy(comando, "mkdir ");
        strcat(comando, path);
        system(comando);
    }

    path_resultados = malloc((strlen(my_path) + 20) * sizeof(char));
    strcpy(path_resultados, my_path);
    strcat(path_resultados, "resultados/"); // el path_resultados tiene que acabar en /

}

int perform_multiplications(void* arg) {

    /*
     * Dado el apuntador a un índice de iteración, esta función lleva a cabo el proceso de multiplicación, suma, y aplicación de ReLU
     * al intervalo de filas de la matriz de datos que le corresponda, y escribirá el resultado en un fichero
     */


    int param = *(int *)arg * rows_per_div; //Indica la primera fila del tramo de filas a utilizar

    double** res1 = reservar_matriz_nxm(res1, rows_per_div, matrices_columns[0]);
    matmul(data, mat1, res1, param, param + rows_per_div, matrices_rows[0], vector_rows[0]);
    add_vector(res1, vec1, rows_per_div, vector_rows[0]);
    relu(res1, rows_per_div, matrices_columns[0]);

    double** res2 = reservar_matriz_nxm(res2, rows_per_div, matrices_columns[1]);
    matmul(res1, mat2, res2, 0, rows_per_div, matrices_rows[1], vector_rows[1]);
    add_vector(res2, vec2, rows_per_div, vector_rows[1]);
    relu(res2, rows_per_div, matrices_columns[1]);

    double** res3 = reservar_matriz_nxm(res3, rows_per_div, matrices_columns[2]);
    matmul(res2, mat3, res3, 0, rows_per_div, matrices_rows[2], vector_rows[2]);
    add_vector(res3, vec3, rows_per_div, vector_rows[2]);
    relu(res3, rows_per_div, matrices_columns[2]);

    double** res4 = reservar_matriz_nxm(res4, rows_per_div, matrices_columns[3]);
    matmul(res3, mat4, res4, 0, rows_per_div, matrices_rows[3], vector_rows[3]);
    add_vector(res4, vec4, rows_per_div, vector_rows[3]);
    //relu(res4, rows_per_div, matrices_columns[3]);
    int* resul = (int*) malloc(rows_per_div * sizeof(int));
    argmax(res4, rows_per_div, matrices_columns[3], resul);
    
    /* Ver contexto completo en main */

    // Escribimos los resultados de este proceso en un fichero
    char* ficheroRes = malloc(sizeof(char) * (strlen(path_resultados) + 20));
    sprintf(ficheroRes, "%sprocess%d", path_resultados, *(int *)arg);

    FILE* f = fopen(ficheroRes, "w");
    for(int i=0;i<rows_per_div; i++){
        fprintf(f, "%d \n", resul[i]);
    }
    fclose(f);
    
    return 0;
}


int main(int argc, char* argv[]){
    /*
     * El programa recibe un único argumento, la cantidad de procesos que se emplearán en la paralelización.
     * Por ejemplo, parallel 3 tendrá que dividir la matriz en tres, y lanzar tres procesos paralelos. Cada proceso, deberá
     * procesar un tercio de la matriz de datos
     */

    if (argc!=2){
        printf("El programa debe tener un único argumento\n");
        exit(1);
    }

    // TODO
    // Para poder cargar los archivos, se necesita el numero de acad que vamos a usar
    //printf("Por favor, introduce el numero de acad de los archivos a cargar: ");
    //scanf("%d", &n_acad);

    // Cargar todos los .csv que vamos a utilizar
    str = (char*) malloc( sizeof(char) * (strlen(my_path) + 20)); // Asignamos suficiente memoria para que quepa my_path + "nombre del archivo"
    data_nrows = 60000;  // Cantidad de datos para multiplicar: 800 para ver si va bien, 60.000 para la prueba del tiempo
    load_data(my_path);

    printf("\nEMPIEZAN LOS CALCULOS\n");
    /*
        Notas para el desarrollo:
        Nota: las multiplicaciones matriciales no se pueden hacer con solo 2 variables, se necesita una variable 
              resultado en la que guardar el resultado o sale todo 0
        Nota: Para reservar una matriz hay que reservar las filas y luego, dentro de las filas, reservar las columnas
    */

    /*
        Contexto: Como no hemos conseguido que todos los procesos escriban sus resultados en un array del padre o algo asi hemos ideado
                  (credito a Dorronsoro) lo siguiente: Cada proceso escribe sus resultados en un fichero en una carpeta resultados y luego
                  desde el programa principal los leemos y combinamos en un fichero que tenga los resultados finales.
    */
    crear_directorio_resultados(); // los procesos escribiran sus resultados en el

    // Inicializamos las variables que van a necesitar los subprocesos
    int divisions = (int)strtol(argv[1], NULL, 10); // Numero de procesos que vamos a usar -> cuantas veces hemos de fragmentar los datos
    rows_per_div = data_nrows / divisions; // Cuantas filas tocan por proceso
    int pids[divisions];
    char *stack[divisions]; // Crearemos tantos stacks como procesos necesitemos

    // Creamos y lanzamos los procesos
    for(int i = 0; i < divisions; i++){
        // Por cada proceso de los divisions procesos que vamos a tener

        stack[i] = malloc(stacksize); // Reservamos el stack de este proceso
        pids[i] = clone((int (*)(void *)) &perform_multiplications, stack[i] + stacksize, SIGCHLD, (void *)(&i)); 
        // Lanzamos un nuevo proceso y le pasamos el i del for. Este numero lo usaremos para decirle a cada proceso que cacho
        // de los datos le toca
    }

    // Ruta del fichero de resultados finales unificados
    sprintf(str, "%spredictions", path_resultados);
    FILE* resultsFile = fopen(str, "w+");

    unsigned char buffer[100]; // Buffer para leer los ficheros creados por los hijos

    for(int i = 0; i < divisions; i++){
        // esperamos a que finalice el hijo numero i
        waitpid(pids[i], NULL, 0);

        // Leemos los resultados del hijo y los volcamos a los resultados totales
        sprintf(str, "%sprocess%d", path_resultados, i);
        FILE* read = fopen(str, "r");
        size_t numLeidos = fread(buffer, sizeof(char), sizeof(buffer), read);
        while(numLeidos!=0){
            fwrite(buffer, sizeof(char), numLeidos, resultsFile);
            numLeidos = fread(buffer, sizeof(char), sizeof(buffer), read);
        }
        fclose(read);
        //remove(str); //Elimnar resultados de los procesos paralelos.
    }
    
    // Imprimimos unos calculos sobre la precision obtenida
    printf("\nRESULTADOS FINALES\n");

    int aciertos = 0;
    int misstreak = 0;

    fseek(resultsFile, 0, SEEK_SET);
    double resul;
    char* buffer2 = malloc(2048);
    
    // Calcular tasa de aciertos
    for (int pred = 0; pred < data_nrows; pred++) {
        fgets(buffer2, sizeof(buffer), resultsFile);
        resul = strtod(buffer2, NULL);

        //printf("pred = %d, found = %d, expected = %d\n", pred, (int)resul, (int)digits[pred]);

        if (resul == digits[pred])
        {
            aciertos += 1;
        //    if (misstreak != 0) printf("missed at %d, misstreak %d\n", pred, misstreak);
            misstreak = 0;
        }
        //else if (misstreak != 0)
        //{
        //    misstreak++;
        //}
        //else
        //{
        //    printf("miss, found %d expected %f index %d\n", resul[pred], digits[pred], pred + 1);
        //    misstreak++;
        //}
    }
    free(buffer2);
    printf("Accuracy: %f\n\n", (float)aciertos / (data_nrows)); // Provisional, modificar para multithreading

    fclose(resultsFile);

    unload_data();
    return 0;
}