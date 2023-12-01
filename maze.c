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
    NUM_OF_DIRECTIONS,

} Direction;

typedef struct {
    int r;
    int c;
} Position;

// Used in determine_triangle_type func. 
// CONTAINS_UP = Triangle contains an UP border => DOWN pointing triangle
// CONTAINS_UP = Triangle contains a DOWN border => UP pointing triangle
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

// A Global variable that can be initialized by using map_ctor function
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
int map_ctor(Map **map, const char *fileName)
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
    
    fclose(file);
    return 0;
}

// Destructor for Map structure
void map_dtor(Map **map)
{
    if(map != NULL) {
        (*map)->rows = 0;
        (*map)->rows = 0;
        free((*map)->cells);
        free(map);
    } 
}


//
// Returns a value of a cell at row and column index like an array would
//
int get_cell_value(Map *map, int rowIndex, int columnIndex)
{
    // Index meaning 0 - x < rows 
    if(rowIndex > map->rows || columnIndex > map->cols || rowIndex < 1 || columnIndex < 1){
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

//
// Uses bitwise operations to determine a 0 / 1 state of a bit from a number
//
bool isolate_bit_value(unsigned eval, BitIndex bit)
{
    unsigned mask = 1U << bit;
    return eval & mask ? true : false;
}

//
// Determines if a border is present in a certain Direction 
//
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

        default:
            fprintf(stderr, "Error direction %d doesn't exist\n", border);

    }

    // To avoid warnings
    fprintf(stderr, "Error border could not be determined\n");
    return true;
}


//
// Returns a value of TriangleType
//
int determine_triangle_type(Position pos)
{
    if(pos.r < 1 || pos.c < 1){
        fprintf(stderr, "Error triangle type couldn't be determined pos.r or pos.c are out of bounds\n");
        return -1;
    }

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

// Initializes a triangle structure
int initialize_triangle(Map *map, Triangle *triangle, int r, int c)
{
    if(r < 1 || c < 1){
        fprintf(stderr, "Error initializing triangle\n");
        return -1;
    }
    // Initializes all necesarry values
    triangle->pos.r = r;
    triangle->pos.c = c;
    triangle->type = determine_triangle_type(triangle->pos);
    triangle->borderValue = get_cell_value(map, r, c);
    triangle->mazeBoundary = determine_maze_boundary(map, *triangle);
    return 0;
}

// Checks if a side in direction is a boundary of the maze using triangle.mazeBoundary values defined in determine_maze_boundary
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
        default:
            fprintf(stderr, "Error direction %d doesn't exist\n", checkDirection);

    }

    // To avoid warnings
    fprintf(stderr, "Error triangle boundary could not be determined\n");
    return false;
}

// Uses [Direction] as a means of changing the r and c values
const Position directionVector[4] = {
    {0, -1},    // move LEFT
    {0, 1},     // move RIGHT
    {-1, 0},    // move UP
    {1, 0},     // move DOWN
};

// Returns a value signifying whether a resultingTriangle has been moved and set in a chosen direction correctly
/* 
 *   -1 = ERROR
 *    0 = SUCCESS
 *    1 = triangle would've moved outside maze
*/
int triangle_move_in(Map *map, Triangle triangleToMove, Triangle *resultingTriangle, Direction direction)
{
    // Checks if a triangle even has a side to move to 
    if(direction > D || direction < L){
        fprintf(stderr, "Error direction value doesn't exist\n");
        return -2;
    }
    if(triangleToMove.type == CONTAINS_UP && direction == D){
        fprintf(stderr, "Error cannot move in that direction wrong TriangleType\n");
        return -2;
    } else if(triangleToMove.type == CONTAINS_DOWN && direction == U){
        fprintf(stderr, "Error cannot move in that direction wrong TriangleType\n");
        return -2;
    }

    bool isBorderInDirection = isborder(map, triangleToMove.pos.r, triangleToMove.pos.c, direction);

    if(isBorderInDirection){
        fprintf(stderr, "Error cannot move in that direction border is in a way\n");
        return -1;
    }
    
    // Illegal states
    if(triangleToMove.pos.c == 1 && triangleToMove.pos.r == 1 && (direction == L || direction == U)){
        fprintf(stderr, "Error cannot move in that direction\n");
        return 1;
    }
    if(triangleToMove.pos.c == map->cols && triangleToMove.pos.r == map->rows && (direction == R || direction == D)){
        fprintf(stderr, "Error cannot move in that direction\n");
        return 1;
    }

    // resultingTriangle would've moved outside the maze resulting in the completion of the maze
    if(triangleToMove.pos.c == map->cols && direction == R){
        return 1;
    }
    if(triangleToMove.pos.c == 1 && direction == L){
        return 1;
    }
    if(triangleToMove.pos.r == map->rows && direction == D){
        return 1;
    }
    if(triangleToMove.pos.r == 1 && direction == U){
        return 1;
    }
    
    // Matches array indexes to Direction enum by -1 to account for it starting at L=1
    initialize_triangle(map, resultingTriangle, triangleToMove.pos.r + directionVector[direction-1].r,triangleToMove.pos.c + directionVector[direction-1].c);
    return 0;

}

// Checks if the contents and format of a file is Valid or Invalid for defining a matrix
int test(const char *fileName)
{
    Map *map;

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

    fclose(file);

    // CHECK IF ADJACENED BORDERS ARE SET CORRECTLY

    if(map_ctor(&map, fileName) == -1){
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

// TODO better remake for mazeboundary limit iterations
int start_border(Map *map, int r, int c, int leftright)
{
    Triangle chosenTriangle;
    initialize_triangle(map, &chosenTriangle, r, c);
    if(chosenTriangle.mazeBoundary == 0){
        fprintf(stderr, "Error wrong args R and C -> cannot start in the middle\n");
        return -1;
    }

    Direction changeDirection[4];
    Direction dirOrder[] = {0, L, R, U, D};;
    Direction rpathOrder[] = {0, R, L, D};
    Direction lpathOrder[] = {0, L, R, U};

    // Adjusts for triangle properties and move orders rpath == lpath in downpointing triangle where .type == CONTAINS_UP 
    if(leftright == L){
        if(chosenTriangle.type == CONTAINS_UP){
            memcpy(changeDirection, rpathOrder, sizeof(changeDirection));
            changeDirection[3] = U;
        }
        else{
            memcpy(changeDirection, lpathOrder, sizeof(changeDirection));
            changeDirection[3] = D;
        }

    } else if(leftright == R){
        if(chosenTriangle.type == CONTAINS_UP){
            memcpy(changeDirection, lpathOrder, sizeof(changeDirection));
            changeDirection[3] = U;
        }
        else{
            memcpy(changeDirection, rpathOrder, sizeof(changeDirection));
            changeDirection[3] = D;
        }
    }
    
    Direction entryDirection;
    Direction resultDirection = 0;
    const int adjustForPath = 1; 
    for(int iterateDirs = 1; iterateDirs < NUM_OF_DIRECTIONS; iterateDirs++)
    {
        if(isborder(map, r, c, dirOrder[iterateDirs]) == 0 && is_maze_boundary(chosenTriangle, dirOrder[iterateDirs]) == 1){
            entryDirection = dirOrder[iterateDirs]; 
            if(!((chosenTriangle.type == CONTAINS_UP && entryDirection == D) || (chosenTriangle.type == CONTAINS_DOWN && entryDirection == U))){
                for(int directions = 1; directions < 4; directions++){
                    // Change direction by +1 index of changeDirection arr
                    if(entryDirection == changeDirection[directions]){
                        // Prevent overflowing
                        if(entryDirection == D || entryDirection == U){
                            resultDirection = changeDirection[adjustForPath];
                            return resultDirection;
                        }
                        // FOR R or L
                        resultDirection = changeDirection[directions+adjustForPath];
                        return resultDirection;
                    }

                }
            }
        }
    }

    fprintf(stderr, "Error starting border couldn't be determined\n");
    return -1;
}

// Used for --rpath a --lpath
int search_maze(Map *map, int r, int c, int leftRight)
{
    Triangle startPos;
    initialize_triangle(map, &startPos, r, c);

    int initialDirection = start_border(map, r, c, leftRight);
    if(initialDirection == -1){
        return -1;
    }

    Direction changeDirection[4];
    Direction rpathOrder[] = {0, R, L, D};
    Direction lpathOrder[] = {0, L, R, U};

    //FIRST TRIANGLE

    // Adjusts for triangle properties and move orders rpath == lpath in downpointing triangle where .type == CONTAINS_UP 
    if(leftRight == L){
        if(startPos.type == CONTAINS_UP){
            memcpy(changeDirection, rpathOrder, sizeof(changeDirection));
            changeDirection[3] = U;
        }
        else{
            memcpy(changeDirection, lpathOrder, sizeof(changeDirection));
            changeDirection[3] = D;
        }

    } else if(leftRight == R){
        if(startPos.type == CONTAINS_UP){
            memcpy(changeDirection, lpathOrder, sizeof(changeDirection));
            changeDirection[3] = U;
        }
        else{
            memcpy(changeDirection, rpathOrder, sizeof(changeDirection));
            changeDirection[3] = D;
        }
    }
    // finds index

    int initialIndex = 0;
    for(int formulateIndex = 1; formulateIndex < 5; formulateIndex++){
        if(initialDirection == changeDirection[formulateIndex]){
            initialIndex = formulateIndex;
        }
    }

    if(initialIndex == 0){
        return -1;
    }
    // Initial direction isn't in a border way
    while(isborder(map, r, c, changeDirection[initialIndex]) != 0){
        if(initialIndex == 3){
            initialIndex = 1;
        } else {
            initialIndex++;
        }

    }
    initialDirection = changeDirection[initialIndex];

    // Whole maze
    int foundPath = 0;
    int dirIndex = 0;
    for(int formulateIndex = 1; formulateIndex < 5; formulateIndex++){
        if(initialDirection == changeDirection[formulateIndex]){
            dirIndex = formulateIndex;
        }
    }
    
    Triangle newPos;

    if(initialize_triangle(map, &newPos, startPos.pos.r, startPos.pos.c) == -1){
        fprintf(stderr, "Error initializing inside path finding\n");
        return -1;
    }

    foundPath = triangle_move_in(map, newPos, &newPos, initialDirection);
    int prevDir = initialDirection;
    while(1){

        if(dirIndex == 0){
            fprintf(stderr, "Error path finding couldn't continue\n");
            return -1;
        }

        if(foundPath == -1){
            // At the end of arr
            if(dirIndex == 3){
                dirIndex = 1;
            } else {
                dirIndex++;
            }

        } else if(foundPath == 0){
            printf("%d, %d\n", newPos.pos.r, newPos.pos.c);
            // Set right changeDirection array
            if(leftRight == L){
                if(newPos.type == CONTAINS_UP){
                    memcpy(changeDirection, rpathOrder, sizeof(changeDirection));
                    changeDirection[3] = U;
                }
                else{
                    memcpy(changeDirection, lpathOrder, sizeof(changeDirection));
                    changeDirection[3] = D;
                }

            } else if(leftRight == R){
                if(newPos.type == CONTAINS_UP){
                    memcpy(changeDirection, lpathOrder, sizeof(changeDirection));
                    changeDirection[3] = U;
                }
                else{
                    memcpy(changeDirection, rpathOrder, sizeof(changeDirection));
                    changeDirection[3] = D;
                }
            }
            // Reroll back to the start
            for(int formulateIndex = 1; formulateIndex < 5; formulateIndex++){
                if(prevDir == changeDirection[formulateIndex]){
                    dirIndex = formulateIndex;
                }
            }
            if(dirIndex+1 == 3){
                dirIndex = 1;
            } else {
                dirIndex++;
            }
            

        } else if(foundPath == -2){
            fprintf(stderr, "Error path finding couldn't continue\n");
            return -1;
        }
        foundPath = triangle_move_in(map, newPos, &newPos, changeDirection[dirIndex]);
    }
    return 0;
    

}

int main(int argc, char *argv[])
{
    if(argc < 2){
        return EXIT_FAILURE;
    }

    // Variables used, Global Variable Map *map; is also being used
    int posR = 0, posC = 0; // check if they're set correctly
    const char *fileName;

    // REMAKE
    int argNum = 1;
        if(argc == 2 && strcmp(argv[argNum], "--help") == 0){
            printHelp();
            return EXIT_SUCCESS;
        }
        
        // checks for --test filename mandatory arg
        if(argc == 3 && strcmp(argv[argNum], "--test") == 0){
            fileName = argv[argNum+1];
            printf("%s\n", test(fileName) == 0 ? "Valid\n" : "Invalid\n");
            return EXIT_SUCCESS;
        }
        if(argc == 5 && strcmp(argv[argNum], "--rpath") == 0){
            posR = atoi(argv[argNum+1]);
            posC = atoi(argv[argNum+2]);
            fileName = argv[argNum+3];

            // TODO SOMETHING
            // JUST TRYING OUT 

            test(fileName);
            if(map_ctor(&map, fileName) == -1){
                return EXIT_FAILURE;
            }
            int x = get_cell_value(map, posR, posC);
            if(x == -1){
                return EXIT_FAILURE;
            }

        }
        if(argc == 5 && strcmp(argv[argNum], "--lpath") == 0){
            posR = atoi(argv[argNum+1]);
            posC = atoi(argv[argNum+2]);
            fileName = argv[argNum+3];

            // TESTING
            Triangle triangle;
            if(map_ctor(&map, fileName) == -1){
                map_dtor(&map);
                return EXIT_FAILURE;
            }
            initialize_triangle(map, &triangle, posR, posC);
            search_maze(map, posR,posC, L);
            

        }

    return EXIT_SUCCESS;
}


