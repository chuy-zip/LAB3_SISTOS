#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <sys/syscall.h> // Para syscall(SYS_gettid)
#include <sys/wait.h>    // Para wait()
#include <omp.h>

// Declarar la matriz global para el sudoku
int sudoku_grid[9][9];

bool columns_valid = false;

void* check_columns_thread(void* arg){
    printf("El hilo que ejecuta la revision de columnas es: %ld\n", syscall(SYS_gettid));

    #pragma omp parallel for
    for(int i = 0; i < 9; i++){
        printf("ID del hilo en ejecucion (dentro del forloop en check_columns_thread): %ld\n", syscall(SYS_gettid));
        // esta lista de booleanos sera para saber si estan todos los numeros del 1 al 9
        // por ejemplo si encontramos un numero num = 6 en la columna, en el arreglo la posicion
        // list[n - 1] = true, osea la sexta posicion en el array (indice 5) sera verdadero
        // al final veremos si hay algun false en la lista, y si lo hay la funcion retorna false
        // ya que quiere decir que hay alguna columna que no cumple con tener todos los numeros del 1 al 9
        bool list[9] = {false};

        for(int j = 0; j < 9; j++){
            int num = sudoku_grid[j][i];
            list[num - 1] = true;
        }

        for(int b = 0; b < 9; b++){
            //encontramos un valor falso dentro de la lista
            if (!list[b]) {
                columns_valid = false;
            }
        }
    }
    // llegados a este punto, quiere decir que todas las columnas tenian todos los numeros del 1 al 9
    columns_valid = true;
    pthread_exit(0);
}

bool check_rows(){

    bool all_rows_valid = true;

    #pragma omp parallel for
    for(int i = 0; i < 9; i++){

        // para las filas es la misma idea que con las columnas
        // al final veremos si hay algun false en la lista, y si lo hay la funcion retorna false
        // ya que quiere decir que hay alguna fila que no cumple con tener todos los numeros del 1 al 9
        bool list[9] = {false};

        for(int j = 0; j < 9; j++){
            int num = sudoku_grid[i][j];
            list[num - 1] = true;
        }

        for(int b = 0; b < 9; b++){

            //encontramos un valor falso dentro de la lista
            if (!list[b]) {
                all_rows_valid = false;
            }
        }
    }
    // llegados a este punto, quiere decir que todas las filas tenian todos los numeros del 1 al 9
    return all_rows_valid;
}

bool check3x3subset(int row_num, int col_num){
    bool list[9] = {false};

    // mismo concepto que los otros 2 pero ahora no es para todo el tablero, sino que se obtiene
    // una coordenada inicial
    #pragma omp parallel for
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int num = sudoku_grid[row_num + i][col_num + j];
            list[num - 1] = true;
        }
    }

    for (int b = 0; b < 9; b++) {
        if (list[b] == false) {
            return false;
        }
    }

    return true;

}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Uso: %s <nombre_del_archivo>\n", argv[0]);
        return 1;
    }

    // Abrir el archivo con open()
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1)
    {
        perror("Error al abrir el archivo");
        return 1;
    }

    off_t file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    // Mapear el archivo a memoria con mmap()
    char *file_content = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_content == MAP_FAILED)
    {
        perror("Error al mapear el archivo");
        close(fd);
        return 1;
    }

    if (file_size != 81) {
        printf("Error: El archivo debe contener exactamente 81 numeros.\n");
        munmap(file_content, file_size);
        close(fd);
        return 1;
    }

    // Cargar el contenido del archivo en la matriz sudoku_grid
    int line_counter = 0;
    for (int i = 0; i < 81; i++)
    {
        char current_char = file_content[i];
        int numero = current_char - '0';

        sudoku_grid[line_counter][i % 9] = numero;

        if ((i + 1) % 9 == 0)
        {
            printf("%i \n", numero);
            line_counter += 1;
        }
        else
        {
            printf("%i", numero);
        }
    }

    // Liberar la memoria mapeada y cerrar el archivo
    munmap(file_content, file_size);
    close(fd);

    // Imprimir la matriz para verificar
    printf("\nMatriz cargada:\n");
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            printf("%d ", sudoku_grid[i][j]);
        }
        printf("\n");
    }

    // ver los subsets de 3x3
    bool subsets_valid = true;
    // ver los subsets de 3x3
    for (int i = 0; i < 9 && subsets_valid; i += 3) {
        for (int j = 0; j < 9 && subsets_valid; j += 3) {
            if (!check3x3subset(i, j)) {
                subsets_valid = false;
                break;
            }
        }
    }


    // si salir es verdadero, quiere decir que en algun punto algun subset no cumplio
    // si salir es verdadero entonces todos los subsets no son correctos
    printf("Todos los subsets 3x3 son correctos? %s\n", subsets_valid ? "true" : "false");

    pid_t parent_pid = getpid();
    //printf("PID del proceso padre (el main): %d\n", parent_pid);

    // Ejecutar fork()
    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("Error al hacer fork");
        return 1;
    }

    if (child_pid == 0) {
        // Código del proceso hijo
        char parent_pid_str[20];
        sprintf(parent_pid_str, "%d", parent_pid); // Convertir el PID a texto

        // Ejecutar el comando ps -p <#proc> -lLf
        printf("El hijo va a ejectuar ps:\n");
        execlp("ps", "ps", "-p", parent_pid_str, "-lLf", NULL);

        // Si execlp falla
        perror("Error al ejecutar execlp");
        exit(1);

    } else { // codigo del padre
        pthread_t column_thread;
        pthread_create(&column_thread, NULL, check_columns_thread, NULL);

        // Esperar a que el hilo termine
        pthread_join(column_thread, NULL);

        // Mostrar el ID del hilo en ejecución
        printf("ID del hilo en ejecución: %ld\n", syscall(SYS_gettid));

        // Esperar a que el primer proceso hijo termine
        wait(NULL);

        // Revisar las filas en el proceso padre
        bool rows_valid = check_rows();

        // es valido el sudoku?
        bool sudoku_valid = columns_valid && rows_valid && subsets_valid;
        printf("La solución del sudoku es %s\n", sudoku_valid ? "válida" : "inválida");

        wait(NULL);

        // Ejecutar un nuevo fork() para el segundo hijo (ps -p <#proc> -lLf)
        pid_t child_pid2 = fork();

        if (child_pid2 == -1) {
            perror("Error al hacer fork");
            return 1;
        }

        if (child_pid2 == 0) {
            // Código del segundo proceso hijo
            char parent_pid_str[20];
            sprintf(parent_pid_str, "%d", parent_pid); // Convertir el PID a texto

            // Ejecutar el comando ps -p <#proc> -lLf
            printf("El segundo hijo hace el comando ps antes de terminar\n");
            execlp("ps", "ps", "-p", parent_pid_str, "-lLf", NULL);

            // Si execlp falla
            perror("Error al ejecutar execlp");
            exit(1);
        }

        // Esperar al segundo proceso hijo
        wait(NULL);

    }

    return 0;

}