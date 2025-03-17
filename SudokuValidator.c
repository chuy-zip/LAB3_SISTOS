#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <nombre_del_archivo>\n", argv[0]);
        return 1;
    }

    FILE *archivo = fopen(argv[1], "r");
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return 1; 
    }
    int sudoku_grid[9][9];
    char linea[82];
    while (fgets(linea, sizeof(linea), archivo) != NULL){
        printf("%s \n", linea);

        int line_length = strlen(linea);
        printf("line length is %i \n", line_length);

        int line_counter = 0;
    
        for (int i = 0; i < line_length; i++){
    
            char current_char = linea[i];
            int numero = current_char - '0';

            sudoku_grid[line_counter][i % 9] = numero;

            if ( (i + 1) % 9 == 0){
                printf("%i \n", numero);
                line_counter += 1;
                
            } else {
                printf("%i", numero);
            }
        }
    }

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            printf("%d ", sudoku_grid[i][j]);
        }
        printf("\n"); 
    }

    return 0;

}