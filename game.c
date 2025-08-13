#include <stdio.h>
#include <windows.h>
#include <time.h>

#define GRID_ROWS 20
#define GRID_COLS 10
#define DEFAULT_CURSOR_ROW 21
#define DEFAULT_CURSOR_COL 1
#define EMPTY_CELL_CHARACTER '.'
#define TILE_CHARACTER 'X'
#define KEY_LEFT 'A'
#define KEY_RIGHT 'D'
#define KEY_DOWN 'S'
#define KEY_UP 'W'
#define FLAG_KEYDOWN 0x0001
#define FLAG_KEYPRESSED 0x8000
#define DEFAULT_DROP_SPEED 1000
#define FAST_DROP_SPEED 50
#define GRID_GAP_LEFT 51
#define SCORE_SCREEN_ROW 4
#define SCORE_SCREEN_COL 83
#define LINE_POINTS_1 4
#define LINE_POINTS_2 10
#define LINE_POINTS_3 30
#define LINE_POINTS_4 120

const int SHAPE_PATTERN_I[4][4] = {
    {0, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 0, 0}
};
const int SHAPE_PATTERN_O[4][4] = {
    {0, 1, 1, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
};
const int SHAPE_PATTERN_L[4][4] = {
    {0, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0}
};
const int SHAPE_PATTERN_Z[4][4] = {
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 0}
};

typedef enum {
    SHAPE_I,
    SHAPE_O,
    SHAPE_L,
    SHAPE_Z
} ShapeType;

typedef enum {
    LEFT,
    RIGHT
} MoveDirection;

typedef struct {
    int x;
    int y;
} ScreenPoint;

typedef struct {
    ScreenPoint origin;
    ScreenPoint tilesPos[4];
    int tileCount;
    ShapeType shapeType;
    int pattern[4][4];
} Shape;

char grid[GRID_ROWS][GRID_COLS]; // board with symbols
ScreenPoint gridCellPositions[GRID_ROWS][GRID_COLS]; // just saves the on screen positions of each cell
CONSOLE_SCREEN_BUFFER_INFO csbi;
long unsigned int pauseDuration = 1000;
int score = 0;
int gameOver = 0;
const int lineScores[4] = {LINE_POINTS_1, LINE_POINTS_2, LINE_POINTS_3, LINE_POINTS_4};
const char tetrisTitle[] = {
    "\n"
    "                                              ╔════════════════════════════╗\n"
    "                                                     ╔╦╗╔═╗╔╦╗╦═╗╦╔═╗\n"
    "                                                      ║ ║╣  ║ ╠╦╝║╚═╗\n"
    "                                                      ╩ ╚═╝ ╩ ╩╚═╩╚═╝\n"
    "                                              ╚════════════════════════════╝"
};
const char gameOverText[7][250] = {
    " ╔════╗      ╔═╗    ╔╗     ╔╗ ╔╦══════     ╔═════╗  ╔╗     ╔╗ ╔╦══════ ╔══════╗ ",
    "╔╣    ╚═    ╔╝ ╚╗   ║╚══╦══╝║ ║║          ╔╣     ╠╗ ║║     ║║ ║║       ╠╗     ╠╗",
    "║║         ╔╝   ╚╗  ║╔═╗ ╔═╗║ ║║          ║║     ║║ ║║     ║║ ║║       ║║     ╠╝",
    "║║   ═╦╦═ ╔╣     ╠╗ ║║ ╚═╝ ║║ ║╠════      ║║     ║║ ╚╣     ╠╝ ║╠════   ║╠═══╦╦╝ ",
    "║║    ║║  ╠╬═════╬╣ ║║     ║║ ║║          ║║     ║║  ╚╗   ╔╝  ║║       ║║   ╚╬╗ ",
    "╚╣    ║║  ║║     ║║ ║║     ║║ ║║          ╚╣     ╠╝   ╚╗ ╔╝   ║║       ║║    ╚╬╗",
    " ╚════╝   ╚╝     ╚╝ ╚╝     ╚╝ ╚╩══════     ╚═════╝     ╚═╝    ╚╩══════ ╚╝     ╚╝"
};

void startScreen();
void initGameGrid();
void printInfo();
void printUI();
void gameOverScreen();
void printGameGrid();
void setGridCell(int, int, char);
void spawnShape(ShapeType);
void dropShape(Shape, int);
int checkCanDrop(Shape);
void moveLeftRight(Shape*, MoveDirection);
int checkCanMoveLeftRight(Shape, MoveDirection);
int isTileInShape(Shape, int, int);
void seedRandom();
int randomInRange(int, int);
void rotateMatrix(int[4][4]);
void rotateShape(Shape*);
void checkFullRows();
void increaseScore(int);

int main() {
    SetConsoleOutputCP(CP_UTF8);
    printf("\x1b[?25l");

    startScreen();

    initGameGrid();
    printInfo();
    printUI();
    printGameGrid();

    seedRandom();

    while(!gameOver) {
        int randomShape = randomInRange(0, 3);
        spawnShape(randomShape);
        checkFullRows();
    }

    gameOverScreen();

    return 0;
}

void startScreen() {
    printf(tetrisTitle);

    printf(
        "\n\n\n\n\n"
        "                                                     [Space] Start..."
    );

    DWORD start = GetTickCount();
    while (GetTickCount() - start < 10000000) {
        if(GetAsyncKeyState(VK_SPACE)) {
            system("cls");
            return;
        }
    }
    
    system("cls");
    return;
}

void initGameGrid() {
    for(int i = 0; i < GRID_ROWS; i++) {
        for(int j = 0; j < GRID_COLS; j++) {
            grid[i][j] = '.';
        }
    }
}

void printInfo() {
    printf("\033[1;55H");
    printf("T E T R I S");
    printf("\033[2;53H");
    printf("Made by Shrinky");
    printf("\033[3;%dH", GRID_GAP_LEFT-2);
    printf("=======================");

    printf("\033[1;1H");
    printf("Controls:\n");
    printf(" - [A] -> Left\n");
    printf(" - [D] -> Right\n");
    printf(" - [W] -> Rotate\n");
    printf(" - [S] -> Fast Drop");
}

void printUI() {
    printf("\033[%d;%dH", SCORE_SCREEN_ROW, SCORE_SCREEN_COL-7);
    printf("Score: %d", score);
}

void gameOverScreen() {
    system("cls");
    printf(tetrisTitle);
    int finalStartRow = 16; // where bottom row will end
    int col = 22;           // horizontal offset

    for (int i = 6; i >= 0; i--) { // from bottom to top
        int finalRow = finalStartRow - (6 - i);
        int prevRow = -1;

        for (int r = 10; r <= finalRow; r++) {
            // erase old position if not first frame
            if (prevRow >= 0) {
                printf("\033[%d;%dH", prevRow, col);
                printf("%-*s", (int)strlen(gameOverText[i]), " "); // clear
            }

            // draw at new position
            printf("\033[%d;%dH", r, col);
            printf("%s", gameOverText[i]);

            prevRow = r;
            Sleep(250);
        }
    }
    printf("\033[19;0H");
    printf(
        "                                                      Score: %d\n\n"
        "                                               Press [space] to exit..."
    , score);
    DWORD start = GetTickCount();
    while (GetTickCount() - start < 10000000) {
        if(GetAsyncKeyState(VK_SPACE)) {
            exit(0);
        }
    }
}

void printGameGrid() {
    for(int i = 0; i < GRID_ROWS; i++) {
        printf("\033[%d;%dH", i+4, GRID_GAP_LEFT-2);
        printf("| ");
        for(int j = 0; j < GRID_COLS; j++) {
            ScreenPoint point;
            HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
            if (!GetConsoleScreenBufferInfo(hStdout, &csbi)) {
                printf("Failed to get console info. Error: %lu\n", GetLastError());
                return;
            }
            point.x = csbi.dwCursorPosition.X;
            point.y = csbi.dwCursorPosition.Y;
            gridCellPositions[i][j] = point;

            printf("%c ", grid[i][j]);
        }
        printf("|\n");
    }
    printf("\033[24;%dH", GRID_GAP_LEFT-2);
    printf("=======================");
}

void setGridCell(int row, int col, char character) {
    grid[row][col] = character;
    ScreenPoint cellPoint = gridCellPositions[row][col];

    printf("\033[%d;%dH%c", cellPoint.y+1, cellPoint.x+1, character);
    fflush(stdout);
    printf("\033[%d;%dH", DEFAULT_CURSOR_ROW, DEFAULT_CURSOR_COL);
    fflush(stdout);
}

void spawnShape(ShapeType type) {
    Shape newShape;
    newShape.origin.x = 3;
    newShape.origin.y = 0;
    newShape.tileCount = 4;
    newShape.shapeType = type;

    switch(type) {
        case SHAPE_I:
            for(int i = 0; i < 4; i++) {
                for(int j = 0; j < 4; j++) {
                    newShape.pattern[i][j] = SHAPE_PATTERN_I[i][j];
                }
            }
            break;
        case SHAPE_L:
            for(int i = 0; i < 4; i++) {
                for(int j = 0; j < 4; j++) {
                    newShape.pattern[i][j] = SHAPE_PATTERN_L[i][j];
                }
            }
            break;
        case SHAPE_Z:
            for(int i = 0; i < 4; i++) {
                for(int j = 0; j < 4; j++) {
                    newShape.pattern[i][j] = SHAPE_PATTERN_Z[i][j];
                }
            }
            break;
        case SHAPE_O:
            for(int i = 0; i < 4; i++) {
                for(int j = 0; j < 4; j++) {
                    newShape.pattern[i][j] = SHAPE_PATTERN_O[i][j];
                }
            }
            break;
    }

    int tileCounter = 0;
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(newShape.pattern[i][j]) {
                if(tileCounter > newShape.tileCount) {
                    break;
                }
                setGridCell(i+newShape.origin.y, j+newShape.origin.x, TILE_CHARACTER);
                newShape.tilesPos[tileCounter].x = j+newShape.origin.x;
                newShape.tilesPos[tileCounter].y = i+newShape.origin.y;
                ++tileCounter;
            }
        }
    }

    dropShape(newShape, 1);
}

void dropShape(Shape shape, int dropCount) {
    if(dropCount == 1 && !checkCanDrop(shape)) {
        gameOver = 1;
        return;
    }

    DWORD start = GetTickCount();
    while (GetTickCount() - start < pauseDuration) {
        if (GetAsyncKeyState(KEY_LEFT) & FLAG_KEYDOWN) {
            moveLeftRight(&shape, LEFT);
        }
        if (GetAsyncKeyState(KEY_RIGHT) & FLAG_KEYDOWN) {
            moveLeftRight(&shape, RIGHT);
        }
        if(GetAsyncKeyState(KEY_DOWN) & FLAG_KEYPRESSED) {
            pauseDuration = FAST_DROP_SPEED;
        }
        else {
            pauseDuration = DEFAULT_DROP_SPEED;
        }
        if(GetAsyncKeyState(KEY_UP) & FLAG_KEYDOWN) {
            rotateShape(&shape);
        }
        Sleep(10);
    }

    if(checkCanDrop(shape)) {
        for(int i = shape.tileCount-1; i >= 0; i--) {
            setGridCell(shape.tilesPos[i].y, shape.tilesPos[i].x, EMPTY_CELL_CHARACTER);
            shape.tilesPos[i].y += 1;
            setGridCell(shape.tilesPos[i].y, shape.tilesPos[i].x, TILE_CHARACTER);
        }
        shape.origin.y += 1;

        dropShape(shape, dropCount+1);
    }
}

int checkCanDrop(Shape shape) {
    for(int i = shape.tileCount-1; i >= 0; i--) {
        int nextRow = shape.tilesPos[i].y + 1;
        int col = shape.tilesPos[i].x;

        if(nextRow >= GRID_ROWS) {
            return 0;
        }

        if(grid[nextRow][col] != '.' && !isTileInShape(shape, nextRow, col)) {
            return 0;
        }
    }
    return 1;
}

int isTileInShape(Shape shape, int row, int col) {
    for(int i = shape.tileCount-1; i >= 0; i--) {
        if(shape.tilesPos[i].x == col && shape.tilesPos[i].y == row) {
            return 1;
        }
    }
    return 0;
}

void moveLeftRight(Shape *shape, MoveDirection dir) {
    if(checkCanMoveLeftRight(*shape, dir)) {
        if(dir == LEFT) {
            for(int i = 0; i < (*shape).tileCount; i++) {
                setGridCell((*shape).tilesPos[i].y, (*shape).tilesPos[i].x, EMPTY_CELL_CHARACTER);
                (*shape).tilesPos[i].x -= 1;
                setGridCell((*shape).tilesPos[i].y, (*shape).tilesPos[i].x, TILE_CHARACTER);
            }
            (*shape).origin.x -= 1;
        } else {
            for(int i = (*shape).tileCount-1; i >= 0; i--) {
                setGridCell((*shape).tilesPos[i].y, (*shape).tilesPos[i].x, EMPTY_CELL_CHARACTER);
                (*shape).tilesPos[i].x += 1;
                setGridCell((*shape).tilesPos[i].y, (*shape).tilesPos[i].x, TILE_CHARACTER);
            }
            (*shape).origin.x += 1;
        }
        
    }
}

int checkCanMoveLeftRight(Shape shape, MoveDirection dir) {
    if(dir == LEFT) {
        for(int i = 0; i < shape.tileCount; i++) {
            int row = shape.tilesPos[i].y;
            int leftCol = shape.tilesPos[i].x - 1;

            if(shape.tilesPos[i].x <= 0) {
                return 0;
            }

            if(grid[row][leftCol] != '.' && !isTileInShape(shape, row, leftCol)) {
                return 0;
            }
        }
    } else {
        for(int i = 0; i < shape.tileCount; i++) {
            int row = shape.tilesPos[i].y;
            int rightCol = shape.tilesPos[i].x + 1;

            if(shape.tilesPos[i].x >= GRID_COLS-1) {
                return 0;
            }

            if(grid[row][rightCol] != '.' && !isTileInShape(shape, row, rightCol)) {
                return 0;
            }
        }
    }

    return 1;
}

void seedRandom() {
    srand((unsigned int)time(NULL));
}

int randomInRange(int min, int max) {
    return rand() % (max - min + 1) + min;
}

void rotateMatrix(int matrix[4][4]) {
    for(int i = 0; i < 4; i++) {
        for(int j = i+1; j < 4; j++) {
            int temp = matrix[i][j];
            matrix[i][j] = matrix[j][i];
            matrix[j][i] = temp;
        }
    }

    for(int i = 0; i < 4; i++) {
        int temp = matrix[i][0];
        matrix[i][0] = matrix[i][3];
        matrix[i][3] = temp;

        temp = matrix[i][1];
        matrix[i][1] = matrix[i][2];
        matrix[i][2] = temp;
    }
}

void rotateShape(Shape* shape) {
    // clear current tiles
    for(int i = 0; i < shape->tileCount; i++) {
        setGridCell(shape->tilesPos[i].y, shape->tilesPos[i].x, EMPTY_CELL_CHARACTER);
    }

    // rotate pattern 90 degrees
    rotateMatrix(shape->pattern);

    // update tile positions after rotation
    int tileCounter = 0;
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(shape->pattern[i][j]) {
                shape->tilesPos[tileCounter].y = shape->origin.y + i;
                shape->tilesPos[tileCounter].x = shape->origin.x + j;
                tileCounter++;
            }
        }
    }

    // draw updated tiles
    for(int i = 0; i < shape->tileCount; i++) {
        setGridCell(shape->tilesPos[i].y, shape->tilesPos[i].x, TILE_CHARACTER);
    }
}

void checkFullRows() {
    int rowTileCount = 0;
    int updatedGridRowIndex = GRID_ROWS-1;
    int fullRowsCount = 0;

    for(int i = GRID_ROWS-1; i >= 0; i--) {
        rowTileCount = 0;
        for(int j = 0; j < GRID_COLS; j++) {
            if(grid[i][j] != EMPTY_CELL_CHARACTER) {
                rowTileCount++;
            }
        }
        
        if(rowTileCount < GRID_COLS) {
            for(int j = 0; j < GRID_COLS; j++) {
                setGridCell(updatedGridRowIndex, j, grid[i][j]);
            }
            updatedGridRowIndex--;
        }
        else {
            fullRowsCount++;
        }
    }

    increaseScore(fullRowsCount);
}

void increaseScore(int fullRows) {
    score += lineScores[fullRows-1];

    printf("\033[%d;%dH", SCORE_SCREEN_ROW, SCORE_SCREEN_COL);
    printf("%d", score);
    printf("\033[%d;%dH", DEFAULT_CURSOR_ROW, DEFAULT_CURSOR_COL);
}