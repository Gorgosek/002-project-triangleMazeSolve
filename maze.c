#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// int main() exit values constants
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define LEFT 1
#define RIGHT 2

typedef struct {
    int rows;
    int cols;
    unsigned char *cells;
} Map;

// Checks if the contents and format of a file is Valid or Invalid for defining a matrix
int testValidity(FILE *file){

    int rows, cols;

    if(fscanf(file, "%d %d", &rows, &cols) != 2){
        fprintf(stderr, "Error reading rows and cols from file\n");
        fclose(file);
        return -1;
    }
    // Matrix is Rectangular
    if(rows == cols || rows < 1 || cols < 1){
        fprintf(stderr, "Error wrong matrix dimensions");
        fclose(file);
        return -1;
    }

    int matrix[rows][cols];

    // Checks if a row contains a valid amount of elements and if the provided elements are in bounds
    for(int row = 0; row < rows; row++){
        int readCount = 0;
        for(int col = 0; col < cols; col++){
            if(fscanf(file, "%d", &matrix[row][col]) == 1){
                readCount++;
            } else{
                fprintf(stderr, "Error reading row from file\n");
                fclose(file);
                return -1;
            }

            // Check if an element is in bounds of 3 bits
            if(matrix[row][col] >= 0 && matrix[row][col] <= 7){
                fprintf(stderr, "Error reading row from file\n");
                fclose(file);
                return -1;
            }
        }
    }

    // Correct border definitions across adj elements 

    return 0;
}
int initializeMap(int rows, int cols, Map *map, FILE *file){
    return 0;
}

bool isborder(Map *map, int r, int c, int border);
int start_border(Map *map, int r, int c, int leftright);

void printHelp(){
    printf("%s",
           "Triangular Maze Solver Program\n"
           "-------------------\n"
           "\n"
           "maze is a program that solves a triangular maze using either the right or left hand rules.\n"
           "\n"
           "Usage: ./maze [OPTION]\n"
           "  --help            Shows help info\n"
           "  --test file.txt   Tests the validity of a file. Prints either 'Valid' or 'Invalid'.\n"
           "  --rpath R C file.txt\n"
           "                    Solves the maze using the right hand rule.\n"
           "                    R(INT) and C(INT) specify the row and column of the starting position.\n"
           "                    'file.txt'(FILE) is a matrix of the maze to be solved.\n"
           "  --lpath R C file.txt\n"
           "                    Solves the maze using the left hand rule.\n"
           "                    R(INT) and C(INT) specify the row and column of the starting position.\n"
           "                    'file.txt'(FILE) is a matrix of the maze to be solved.\n"
           "  --shortest R C file.txt\n"
           "                    Finds the shortest path in the maze.\n"
           "                    R(INT) and C(INT) specify the row and column of the starting position.\n"
           "                    'file.txt'(FILE) is a matrix of the maze to be solved.\n"
           );
}
int main(int argc, char *argv[])
{
    if(argc < 2){
        return EXIT_FAILURE;
    }
    Map map;
    int posR = 0, posC = 0;
    int leftright = 0;
    FILE *file;
    for(int argNum = 1; argNum < argc; argNum++){
        if(argc == 2 && strcmp(argv[argNum], "--help") == 0){
            printHelp();
            return EXIT_SUCCESS;
        }
        // checks for --test filename mandatory arg
        if(argc == 3 && strcmp(argv[argNum], "--test") == 0){
            file = fopen(argv[argNum+1], "r"); // next argv is a filename
            if(file == NULL){
                fprintf(stderr, "Error opening file %s\n", argv[argNum+1]);
                return EXIT_FAILURE;
            }
            // TODO SOMETHING
        }
        if(argc == 5 && strcmp(argv[argNum], "--rpath") == 0){
            posR = atoi(argv[argNum+1]);
            posC = atoi(argv[argNum+2]);
            file = fopen(argv[argNum+3], "r"); 
            leftright = RIGHT;
            if(file == NULL){
                fprintf(stderr, "Error opening file %s\n", argv[argNum+1]);
                return EXIT_FAILURE;
            }

            // TODO SOMETHING
        }
        if(argc == 5 && strcmp(argv[argNum], "--lpath") == 0){
            posR = atoi(argv[argNum+1]);
            posC = atoi(argv[argNum+2]);
            file = fopen(argv[argNum+3], "r"); 
            leftright = LEFT;
            if(file == NULL){
                fprintf(stderr, "Error opening file %s", argv[argNum+1]);
                return EXIT_FAILURE;
            }

            // TODO SOMETHING
        }
    }
    fclose(file);

    return EXIT_SUCCESS;
}
