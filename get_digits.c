#define _GNU_SOURCE
#include <stdio.h>  // file handling functions
#include <stdlib.h> // atoi
#include <string.h> // strtok
#include <sched.h>
#include <signal.h>
#include <sys/wait.h>
#define stacksize 1048576

double **data;
int data_nrows;
int data_ncols = 784;
char *my_path = "/home/unai-kubuntu/Desktop/IRCSO/ejercicios/MNIST/parameters/";  // vosotros tendréis que poner vuestro path
char* n_acad = "11"; // TODO 

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
        printf("Buffer: %s, %f", buffer, aux);
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
    digits = malloc(data_nrows*sizeof(double));
    sprintf(str, "%sdigits.csv", path);
    read_vector(digits, str, data_nrows);

    // mostramos las primeras 20 columnas de digits para ver que todo va bien
    //print_vector(digits, 20, 0);


    // Cargar data.csv (mega matriz con los numeros a procesar)
    data = malloc(data_nrows * data_ncols * sizeof(double));
    sprintf(str, "%sdata.csv", path);
    read_matrix(data, str, data_nrows, data_ncols, 1);

    // mostramos una submatrix 20x40 para comprobar (el offset de 70 porque a la izaquierda solo hay 0s)
    print_matrix(data, 20, 40, 0, 70);


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
    read_vector(vec1, str, vector_rows[1]);

    sprintf(str, "%sbiases2_%s.csv", path, n_acad);
    read_vector(vec1, str, vector_rows[2]);

    sprintf(str, "%sbiases3_%s.csv", path, n_acad);
    read_vector(vec1, str, vector_rows[3]);

    return 0;
}

int unload_data(){
    /*
     * Descarga la memoria
     */

    //TODO
    return 0;
}

//int perform_multiplications(void* arg) {
//
//    /*
//     * Dado el apuntador a un índice de iteración, esta función lleva a cabo el proceso de multiplicación, suma, y aplicación de ReLU
//     * al intervalo de filas de la matriz de datos que le corresponda, y escribirá el resultado en un fichero
//     */
//
//    double **res1 = malloc(rows_per_div * sizeof(*res1));
//
//
//    int param = *(int *)arg * rows_per_div;
//    matmul(data, mat1, res1, -1, -1, data_ncols, matrices_columns[0]);
//    add_vector(res1, vec1, rows_per_div, matrices_columns[0]);
//    relu(res1, rows_per_div, matrices_columns[0]);
//    
//    //TODO (hacer el resto de multiplicaciones, sumas, y ReLU
//    
//    for (int pred = 0; pred < rows_per_div; pred++) {
//        if (predictions[pred] == digits[start + pred])
//            aciertos += 1;
//    }
//    printf("Accuracy de la parte %d: %f\n", *(int *)arg, (float)aciertos / (end - start));
//    
//    //TODO (escribir en fichero)
//
//    return 0;
//}



int print(void* arg){
    printf("Hola, soy %d\n", *(int *)arg);
    sleep(10);
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
    data_nrows = 1000;  // Cantidad de datos para multiplicar: 800 para ver si va bien, 60.000 para la prueba del tiempo
    load_data(my_path);

    printf("\nEMPIEZAN LOS CALCULOS\n");

    /*
        Nota: las multiplicaciones matriciales no se pueden hacer con solo 2 variables, se necesita una variable 
              resultado en la que guardar el resultado o sale todo 0
        Nota: Para reservar una matriz hay que reservar las filas y luego, dentro de las filas, reservar las columnas
    */

    double** res1 = reservar_matriz_nxm(res1, data_nrows, matrices_columns[0]);
    matmul(data, mat1, res1, 0, data_nrows, matrices_rows[0], vector_rows[0]);
    add_vector(res1, vec1, data_nrows, vector_rows[0]);
    relu(res1, data_nrows, matrices_columns[0]);

    double** res2 = reservar_matriz_nxm(res2, data_nrows, matrices_columns[1]);
    matmul(res1, mat2, res2, 0, data_nrows, matrices_rows[1], vector_rows[1]);
    add_vector(res2, vec2, data_nrows, vector_rows[1]);
    relu(res2, data_nrows, matrices_columns[1]);

    double** res3 = reservar_matriz_nxm(res3, data_nrows, matrices_columns[2]);
    matmul(res2, mat3, res3, 0, data_nrows, matrices_rows[2], vector_rows[2]);
    add_vector(res3, vec3, data_nrows, vector_rows[2]);
    relu(res3, data_nrows, matrices_columns[2]);

    double** res4 = reservar_matriz_nxm(res4, data_nrows, matrices_columns[3]);
    matmul(res3, mat4, res4, 0, data_nrows, matrices_rows[3], vector_rows[3]);
    add_vector(res4, vec4, data_nrows, vector_rows[3]);
    int* resul = (int*) malloc(data_nrows * sizeof(int));
    argmax(res4, data_nrows, matrices_columns[3], resul);
    
    printf("\nRESULTADOS FINALES\n");
    for (int i = 0; i < data_nrows; i++) printf("%d\n", resul[i]);

    /////// PROVISIONAL
    return 0;

    int divisions = (int)strtol(argv[1], NULL, 10);
    int aux1 = 0;
    int aux = divisions;
    int pids[aux];
    char *stack[aux];

    for(int i=aux1; i<aux; i++){
        stack[i] = malloc(stacksize);
        pids[i] = clone((int (*)(void *)) &print, stack[i] + stacksize, SIGCHLD, (void *)(&i));
        //waitpid(pids[i], NULL, 0);
    }

    /*for(int i=aux1; i<aux; i++){
        waitpid(pids[i], NULL, 0);
    }*/

    return 0;
}