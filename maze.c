#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

// CONSTANTS
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

typedef struct {
    int rows;
    int cols;
    unsigned char *cells;
} Map;

// Represents bit indexes borders needed values from 0-2
typedef enum {
    LEFT_BIT, // Left-most border
    RIGHT_BIT, // Right-most border
    UPDOWN_BIT, // Up or Down border
} BitIndex;

// Symbolizes all possible directions for which to solve maze
typedef enum {
    L = 1, // LEFT 
    R, // RIGHT
    U, // UP - odd row at 1. col
    D, // DOWN - even row at 1. col
} Direction;

typedef struct {
    int r;
    int c;
} Position;

// Used in determine_triangle_type func.
typedef enum {
    CONTAINS_UP,
    CONTAINS_DOWN,
} TriangleType;

// Defines a triangle
typedef struct {
    Position pos;
    TriangleType type;
    unsigned borderValue; // Derived from cells inside map
    unsigned mazeBoundary; // Explaines sides of a triangle that act as outside maze boundaries
} Triangle;

// ? TRY
Map *map;



// Prints help onto the screen when using --help option
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

// Initializes Map structure and cells array, validates correctness of data contained in file (using function test), allocates Map and unsigned char *cells
int initialize_map(Map **map, const char *fileName)
{
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
// In other words if triangle contains an UPPER or an LOWER BOUND
//
//int triangle_type(int r, int c)
//{
//    if(r <= 0 || c <= 0){
//        fprintf(stderr, "Error triangle_type couldn't be determined r or c are out of bounds\n");
//        return -1;
//    }
//
//    // Even Row
//    if(r % 2 == 0){
//        if(c % 2 == 1)
//            return D; // Has a DOWN border
//        else
//            return U; // Has an UP border
//    }
//
//    // Odd Row
//    if(r % 2 == 1){
//        if(c % 2 == 0)
//            return U; // Has an UP border
//        else
//            return D; // Has a DOWN border
//    }
//        
//    fprintf(stderr, "Error triangle_type couldn't be determined r or c are out of bounds\n");
//    return -1;
//}


// OPPS FIX
// WORKS!
int determine_triangle_type(Position pos)
{
    if(pos.r < 1 || pos.c < 1){
        fprintf(stderr, "Error triangle type couldn't be determined pos.r or pos.c are out of bounds\n");
        return -1;
    }
    //if(pos.r == 1){
    //    if(pos.c % 2 == 1)
    //        return CONTAINS_UP;
    //    else
    //     return CONTAINS_DOWN;
    //}

    // Even Row
    if(pos.r % 2 == 0){
        if(pos.c % 2 == 1)
            return CONTAINS_DOWN; // Has a DOWN border
        else
            return CONTAINS_UP; // Has an UP border
    }

    // Odd Row
    if(pos.r % 2 == 1){
        if(pos.c % 2 == 0)
            return CONTAINS_DOWN; // Has a DOWN border
        else
            return CONTAINS_UP; // Has an UP border
    }
        
    fprintf(stderr, "Error triangle type couldn't be determined pos.r or pos.c are out of bounds\n");
    return -1;
}

//
// Sets the maze boundary of an triangle returns a DEC value of 0-7
//
int determine_maze_boundary(Map *map, Triangle triangle)
{
    unsigned mazeBoundary = 0;
    int r = triangle.pos.r, c = triangle.pos.c;
    if(r < 1 || c < 1){
        fprintf(stderr, "Error couldn't read position of triangle\n");
        return -1;
    }

    if(r == 1 && triangle.type == CONTAINS_UP){
        mazeBoundary += pow(2, UPDOWN_BIT);     // Adds correct DEC value corresponding to the bit
    } else if(r == map->rows && triangle.type == CONTAINS_DOWN){
        mazeBoundary += pow(2, UPDOWN_BIT);
    }

    if(c == 1){
        mazeBoundary += pow(2, LEFT_BIT);
    } else if(c == map->cols){
        mazeBoundary += pow(2, RIGHT_BIT);
    }
    
    return mazeBoundary;
}

void initialize_triangle(Map *map, Triangle *triangle, int r, int c)
{
    if(r < 1 || c < 1){
        fprintf(stderr, "Error initializing triangle\n");
        return;
    }
    //TODO
    triangle->pos.r = r;
    triangle->pos.c = c;
    triangle->type = determine_triangle_type(triangle->pos);
    triangle->borderValue = get_cell_value(map, r, c);
    triangle->mazeBoundary = determine_maze_boundary(map, *triangle);
}

//WORKS!
bool is_maze_boundary(Triangle triangle, Direction checkDirection)
{
    
    unsigned mazeBoundary = triangle.mazeBoundary;
    
    switch(checkDirection){
        case L:
            return isolate_bit_value(mazeBoundary, LEFT_BIT);
        break;
        case R:
            return isolate_bit_value(mazeBoundary, RIGHT_BIT);
        break;
        case U:
            return isolate_bit_value(mazeBoundary, UPDOWN_BIT);
        case D:
            return isolate_bit_value(mazeBoundary, UPDOWN_BIT);
        break;
    }

    // To avoid warnings
    fprintf(stderr, "Error triangle boundary could not be determined\n");
    return false;

}

//TODO remove if not needed for start_border later on
typedef struct{
    Position pos;
    Direction borderPos[3];
} BordersAtPos;

// Uses [Direction] as a means of changing the r and c values
// TODO put in a move function?
const Position directionVector[4] = {
    {0, -1},    // move LEFT
    {0, 1},     // move RIGHT
    {-1, 0},    // move UP
    {1, 0},     // move DOWN
};

// ? TRY
// Returns a new triangle moved over to the specified direction
Triangle switch_to_triangle_in_direction(Map *map, Triangle *triangle, Direction direction)
{
    Triangle resultingTriangle;
    
    // Uses direction - 1 to account for enum starting at L=1
    initialize_triangle(map, &resultingTriangle, triangle->pos.r + directionVector[direction-1].r,triangle->pos.c + directionVector[direction-1].c);
    return resultingTriangle;

}

// TODO continue
//int start_border(Map *map, int r, int c, int leftright)
//{
//    return -1;
//}
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

    //
    // CHECK VALID CORRESPONDING BORDERS
    //

    if(initialize_map(&map, fileName) == -1){
        return -1;
    }

    Triangle triangle; 
    Triangle iterTriangle;

    // Check left/right
    for(int row = 1; row <= map->rows; row++){
        for(int col = 2; col <= map->cols; col++){
            initialize_triangle(map, &triangle, row, col); // One ahead 
            initialize_triangle(map, &iterTriangle, row, col-1); // One behind
            if(isborder(map, iterTriangle.pos.r, iterTriangle.pos.c, R) != isborder(map, triangle.pos.r, triangle.pos.c, L)){
                fprintf(stderr, "Error borders aren't defined correctly\n");
                return -1;
            }
        }
    }
    
    // Check Up/Down
    for(int row = 2; row <= map->rows; row++){
        for(int col = 1; col <= map->cols; col++){
            initialize_triangle(map, &triangle, row-1, col); // above iterTriangle
            initialize_triangle(map, &iterTriangle, row, col); // underneath triangle
            if(determine_triangle_type(triangle.pos) == CONTAINS_DOWN && determine_triangle_type(iterTriangle.pos) == CONTAINS_UP){
                if(isborder(map, iterTriangle.pos.r, iterTriangle.pos.c, U) != isborder(map, triangle.pos.r, triangle.pos.c, D)){
                    fprintf(stderr, "Error borders aren't defined correctly\n");
                    return -1;
                }

            }
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if(argc < 2){
        return EXIT_FAILURE;
    }

    // Variables used, Global Variable Map *map; is also being used
    int posR = 0, posC = 0;
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
            printf("%s\n", test(fileName) == 0 ? "Valid" : "Invalid");
            return EXIT_SUCCESS;
        }
        if(argc == 5 && strcmp(argv[argNum], "--rpath") == 0){
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
            posR = atoi(argv[argNum+1]);
            posC = atoi(argv[argNum+2]);
            fileName = argv[argNum+3];
            Triangle triangle;
            if(initialize_map(&map, fileName) == -1){
                return EXIT_FAILURE;
            }
            initialize_triangle(map, &triangle, posR, posC);

            printf("%s", determine_triangle_type(triangle.pos) == CONTAINS_UP ? "UP": "DOWN");
            printf("%s", is_maze_boundary(triangle, L) ? "YES": "NO");


        }

        // TODO SOMETHING
    }

    return EXIT_SUCCESS;
}
