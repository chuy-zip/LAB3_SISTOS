#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>    
#include <unistd.h>     
#include <sys/mman.h>   

// Declarar la matriz global para el sudoku
int sudoku_grid[9][9];

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

    return 0;
}