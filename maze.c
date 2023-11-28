#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// CONSTANTS
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

typedef struct {
    int rows;
    int cols;
    unsigned char *cells;
} Map;

//
// Represents bit indexes borders
//
typedef enum {
    LEFT_BIT, // Left-most border
    RIGHT_BIT, // Right-most border
    UPDOWN_BIT, // Up or Down border
} BitIndex;

//
// Symbolizes all possible directions for which to solve maze
// Used as an Identifier for RIGHT or LEFT hand rule
// 
//
typedef enum {
    L = 1, // LEFT 
    R, // RIGHT
    U, // UP - odd row at 1. col
    D, // DOWN - even row at 1. col
} Direction;

//
// Checks if the contents and format of a file is Valid or Invalid for defining a matrix
//
int test(const char *fileName)
{

    int rows, cols;
    FILE *file = fopen(fileName, "r");
    if(file == NULL){
        fprintf(stderr, "Error opening file\n");
        fclose(file);
        return -1;
    }

    if(fscanf(file, "%d %d", &rows, &cols) != 2){
        fprintf(stderr, "Error reading rows and cols from file\n");
        fclose(file);
        return -1;
    }
    // Matrix is Rectangular
    // TODO not sure if this is needed
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
            if(!(matrix[row][col] >= 0 && matrix[row][col] <= 7)){
                fprintf(stderr, "Error row %d from file is out of bounds: [%d] != (0-7)\n", row, matrix[row][col]);
                fclose(file);
                return -1;
            }
        }
    }

    // Check file for any extra elements
    int extraElems;
    if(fscanf(file, "%d", &extraElems) != EOF){
        fprintf(stderr, "Error file contains extra elements\n");
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}


//
// Initializes Map structure and cells array, validates correctness of data contained in file (using function test), allocates Map and unsigned char *cells
//
int initialize_map(Map **map, const char *fileName)
{
    if(test(fileName) == -1){
        return -1;
    }

    FILE *file = fopen(fileName, "r");
    if(file == NULL){
        fprintf(stderr, "Error opening file\n");
        fclose(file);
        return -1;
    }

    int rows = 0, cols = 0;

    if(fscanf(file, "%d %d", &rows, &cols) != 2){
        fprintf(stderr, "Error reading rows and cols from file\n");
        fclose(file);
        return -1;
    }

    int numOfCells = rows*cols;

    // Allocates the correct size of predefined struct Map
    *map = malloc(sizeof(int)*2+sizeof(unsigned char *));
    if(*map == NULL){
        fprintf(stderr, "Malloc failed\n");
        fclose(file);
        return -1;
    }

    (*map)->rows = rows;
    (*map)->cols = cols;

    // Allocates memory needed for all fields of the matrix
    (*map)->cells = (unsigned char *) malloc(sizeof(unsigned char) * rows * cols);
    if((*map)->cells == NULL){
        fprintf(stderr, "Malloc failed on cells\n");
        fclose(file);
        return -1;
    }

    for(int cellIndex = 0; cellIndex < numOfCells; cellIndex++){
        unsigned char readValue;
        if(fscanf(file, "%hhu", &readValue) != 1){ // %hhu - specifier for uchar :O
            fprintf(stderr, "Error reading row from file\n");
            fclose(file);
            return -1;
        }
        (*map)->cells[cellIndex] = readValue;
    }
    
    //TODO free these fuckers somehow
    fclose(file);
    return 0;
}

// Destructor for Map structure
void free_map(Map *map)
{
    if(map != NULL) {
        free(map->cells);
        free(map);
    } 
}


//
// Returns a value of a cell at row and column index like an array would
//
int get_cell_value(Map *map, int rowIndex, int columnIndex)
{
    // Index meaning 0 - x < rows 
    if(rowIndex > map->rows || columnIndex > map->cols || rowIndex <= 0 || columnIndex <= 0){
        fprintf(stderr, "Error row or column out of bounds\n");
        return -1;
    }
    rowIndex--;
    columnIndex--;

    return (int)map->cells[rowIndex*map->cols + columnIndex];
}

//
// Prints entire already initialized matrix saved inside Map structure
//
void print_entire_matrix(Map *map)
{

    for(int row = 1; row <= map->rows; row++){
        for(int column = 1; column <= map->cols; column++){
            printf(" %d", get_cell_value(map, row, column));
        }
        printf("\n");
    }
}

// Uses bitwise operations to determine a 0 / 1 state of a bit from a number
bool isolate_bit_value(unsigned eval, BitIndex bit)
{
    unsigned mask = 1U << (sizeof(unsigned) * 8 - bit);
    return eval & mask ? true : false;
}

bool isborder(Map *map, int r, int c, int border)
{
    
    int cell = get_cell_value(map, r, c);
    if(cell == -1){
        fprintf(stderr, "Error cell [%d] isn't valid\n", cell);
    }
    unsigned cellValue = (unsigned) cell;
    
    switch(border){
        case L:
            return isolate_bit_value(cellValue, LEFT_BIT);
        break;
        case R:
            return isolate_bit_value(cellValue, RIGHT_BIT);
        break;
        case U:
            return isolate_bit_value(cellValue, UPDOWN_BIT);
        case D:
            return isolate_bit_value(cellValue, UPDOWN_BIT);
        break;
    }

    // To avoid warnings
    fprintf(stderr, "Error border could not be determined\n");
    return true;
}


//
// Determines if a triangle points up or down
//
int triangle_type(int r, int c)
{
    if(r <= 0 || c <= 0){
        fprintf(stderr, "Error triangle_type couldn't be determined r or c are out of bounds\n");
        return -1;
    }

    // Even Row
    if(r % 2 == 0){
        if(c % 2 == 1)
            return D; // Has a DOWN border
        else
            return U; // Has an UP border
    }

    // Odd Row
    if(r % 2 == 1){
        if(c % 2 == 0)
            return U; // Has an UP border
        else
            return D; // Has a DOWN border
    }
        
    fprintf(stderr, "Error triangle_type couldn't be determined r or c are out of bounds\n");
    return -1;
}

typedef struct {
    int r;
    int c;
} Position;

typedef struct{
    Position pos;
    Direction borderPos[3];
} BordersAtPos;

// TODO use this mb?
typedef struct{
    BordersAtPos checkBorders;
    int triangleType; 
    int result[3];
} TriangleBorders;


// TODO continue
int start_border(Map *map, int r, int c, int leftright)
{
    static int timesUsed = 0;
    // Checks if starting cell is one of the outermost elements of the matrix
    if(!(r < map->rows || r > 0) && c == 1){
        if(!(c < map->cols || c > 0) && r == 1){
            fprintf(stderr, "Error starting row and column values aren't correct\n");
            timesUsed++;
            return -1;
        }
    }

    BordersAtPos matrixCorners[4] = {
        {{1, 1}, {L, U, 0}},                    // Top-Left Corner
        {{1, map->cols}, {R, U, 0}},            // Top-Right Corner 
        {{map->rows, 1}, {L, D, 0}},            // Bottom-Left Corner 
        {{map->rows, map->cols}, {R, D, 0}}     // Bottom-Right Corner 
    };

    int cornerIndex = -1;
    for(int tryCorners = 0; tryCorners < 4; tryCorners++){
        if(r == matrixCorners[tryCorners].pos.r && c == matrixCorners[tryCorners].pos.c){
            cornerIndex = tryCorners;
            break;
        }
    }

    Direction checkDirections[3] = {0, 0, 0}; // Array stating which directions to check later
    int numOfDirectionsToCheck = 0;
    int triangleType = -1;

    // Determine triangleType
    triangleType = triangle_type(r,c);

    // Position isn't in a corner
    if(cornerIndex == -1){
        // LEFT SIDE
        if(c == 1){
            checkDirections[0] = L;
            numOfDirectionsToCheck++;
        }
        // RIGHT SIDE
        if(c == map->cols){
            checkDirections[0] = R;
            numOfDirectionsToCheck++;
        }
        // TOP SIDE - triangleType has to have an upper border
        if(r == 1 && triangleType == U){
            checkDirections[0] = U;
            numOfDirectionsToCheck++;
        }
        // BOTTOM SIDE - triangleType has to have a bottom border denoted as D for DOWN
        if(r == map->rows && triangleType == D){
            checkDirections[0] = D;
            numOfDirectionsToCheck++;
        }
        
    // Position is in a corner
    } else {
        numOfDirectionsToCheck = 2;
        memcpy(checkDirections, matrixCorners[cornerIndex].borderPos, sizeof(checkDirections)); // Copies data of borderPositions in corners
    }
    
    if(numOfDirectionsToCheck == 0){
        fprintf(stderr, "Error couldn't determine a correct entrance\n");
        timesUsed++;
        return -1;
    }

    Direction chosenDirection = 0;

    for(int tryDirections = 0; tryDirections < numOfDirectionsToCheck; tryDirections++){
        if(isborder(map, r, c, checkDirections[tryDirections]) == false){
            chosenDirection = checkDirections[tryDirections];
        }
    }

    if(chosenDirection == 0){
        fprintf(stderr, "Error couldn't determine an unobstructed entrance\n");
        timesUsed++;
        return -1;
    }

    // Determine which borders to check ->

    switch(leftright){
        case L:
            break;
        case R:
            // L-> R
            // L-> D
            // R-> U
            // R-> L
            // U-> L
            // D-> R
            switch(chosenDirection){
                case L:
                    if(isborder(map, r, c, R) == false){
                        return R;
                    }
                    if(isborder(map, r, c, L) == false){
                        return R;
                    }
                break;
            }
            


            break;
    }
    
    timesUsed++;
    return -1;

}

void printHelp()
{
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
    Map *map;
    int posR = 0, posC = 0;
    //int leftright = 0;
    const char *fileName;
    for(int argNum = 1; argNum < argc; argNum++){
        if(argc == 2 && strcmp(argv[argNum], "--help") == 0){
            printHelp();
            return EXIT_SUCCESS;
        }
        // checks for --test filename mandatory arg
        if(argc == 3 && strcmp(argv[argNum], "--test") == 0){
            fileName = argv[argNum+1];
            // TODO SOMETHING
            int testResult = test(fileName);
            if(testResult == 0){
                printf("Valid\n");
            }
            else if(testResult == -1){
                printf("Invalid\n");
            }
            return EXIT_SUCCESS;
        }
        if(argc == 5 && strcmp(argv[argNum], "--rpath") == 0){
     //       leftright = R;
            posR = atoi(argv[argNum+1]);
            posC = atoi(argv[argNum+2]);
            fileName = argv[argNum+3];

            // TODO SOMETHING
            // JUST TRYING OUT 
            if(initialize_map(&map, fileName) == -1){
                return EXIT_FAILURE;
            }
            int x = get_cell_value(map, posR, posC);
            if(x == -1){
                return EXIT_FAILURE;
            }
            print_entire_matrix(map);

        }
        if(argc == 5 && strcmp(argv[argNum], "--lpath") == 0){
      //      leftright = L;
            posR = atoi(argv[argNum+1]);
            posC = atoi(argv[argNum+2]);
            fileName = argv[argNum+3];
        }

        // TODO SOMETHING
    }

    return EXIT_SUCCESS;
}
