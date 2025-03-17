#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>    
#include <unistd.h>     
#include <sys/mman.h>   
#include <stdbool.h>

// Declarar la matriz global para el sudoku
int sudoku_grid[9][9];


bool check_columns(){

    for(int i = 0; i < 9; i++){
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
            if(!list[b]) return false;
        }
    }
    // llegados a este punto, quiere decir que todas las columnas tenian todos los numeros del 1 al 9
    return true;
}

bool check_rows(){
    for(int i = 0; i < 9; i++){

        // para las filas es la misma idea que con las columnas
        // al final veremos si hay algun false en la lista, y si lo hay la funcion retorna false
        // ya que quiere decir que hay alguna columna que no cumple con tener todos los numeros del 1 al 9
        bool list[9] = {false};

        for(int j = 0; j < 9; j++){
            int num = sudoku_grid[i][j];
            list[num - 1] = true;
        }

        for(int b = 0; b < 9; b++){

            //encontramos un valor falso dentro de la lista
            if(!list[b]) return false;
        }
    }
    // llegados a este punto, quiere decir que todas las filas tenian todos los numeros del 1 al 9
    return true;
}

bool check3x3subset(int row_num, int col_num){
    bool list[9] = {false};

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

    printf("Todas las columnas son correctas? %s\n", check_columns() ? "true" : "false");
    printf("Todas las filas son correctas? %s\n", check_rows() ? "true" : "false");
    printf("El subset es correcto? %s\n", check3x3subset(0,0) ? "true" : "false");
    
    return 0;
}