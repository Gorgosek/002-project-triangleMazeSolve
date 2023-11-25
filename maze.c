#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// int main() exit values constants
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

typedef struct {
    int rows;
    int cols;
    unsigned char *cells;
} Map;

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
    for(int argNum = 1; argNum < argc; argNum++){
        if(strcmp(argv[argNum], "--help") == 0){
            printHelp();
            return EXIT_SUCCESS;
        }
        // checks for --test filename mandatory arg
        if(argc == 3 && strcmp(argv[argNum], "--test") == 0){
            FILE *file = fopen(argv[argNum+1], "r"); // next argv is a filename
            if(file == NULL){
                fprintf(stderr, "Error opening file %s", argv[argNum+1]);
                return EXIT_FAILURE;
            }

            fclose(file);

        }
    }

    return EXIT_SUCCESS;
}
