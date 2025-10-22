// super fast!

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "json_utils.h"
#include "wfc.h"

// Grid Stuff
int* grid;       // 3D grid (Y, X, Tiles)
int* gridSizes;  // Stores the size of the tiles for each grid cell
int tileCount = 0;
const int gridWidth = 10;
const int gridHeight = gridWidth;
const int tileSize = 16;

int DIRS[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
const char* DIRNAMES[] = {"up", "down", "left", "right"};

// Neighbors Stuff
int* neighbors;
int* neighborsSizes;

// Queue Stuff
typedef struct Node {
    int value;
    struct Node* next;
} Node;

Node* headNode = NULL;
Node* tailNode = NULL;

int rSeed = 0;

// Fast random number generator (Xorshift32)
static unsigned int xor_state = 1;
void init_xorshift(unsigned int seed) {
    xor_state = seed ? seed : (unsigned int)time(NULL);
}

unsigned int xorshift32() {
    unsigned int x = xor_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    xor_state = x;
    return x;
}

void enqueue(int value) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->value = value;
    newNode->next = NULL;

    if (tailNode == NULL) {
        // Queue is empty
        headNode = newNode;
        tailNode = newNode;
    } else {
        // Append to the end
        tailNode->next = newNode;
        tailNode = newNode;
    }
}

int dequeue() {
    Node* temp = headNode;
    int value = temp->value;
    headNode = headNode->next;

    if (headNode == NULL) {
        tailNode = NULL;
    }

    free(temp);
    return value;
}

int inGrid(int index, int dirIndex) {
    int x = (index % gridWidth) + DIRS[dirIndex][0];
    int y = (int)((index % (gridWidth * gridHeight)) / gridWidth) + DIRS[dirIndex][1];

    return 0 <= x && x < gridWidth && 0 <= y && y < gridHeight;
}

void setupGrid() {
    grid = (int*)malloc(gridWidth * gridHeight * tileCount * sizeof(int));
    gridSizes = (int*)malloc(gridWidth * gridHeight * sizeof(int));

    for (int i = 0; i < gridWidth * gridHeight; i++) {
        gridSizes[i] = tileCount;
    }

    for (int y = 0; y < gridHeight; y++) {
        for (int x = 0; x < gridWidth; x++) {
            int posIndex = (y * gridWidth + x);
            gridSizes[posIndex] = tileCount;
            for (int a = 0; a < tileCount; a++) {
                int index = posIndex * tileCount + a;
                grid[index] = a;
            }
        }
    }
}

int isFullyCollapsed() {
    for (int y = 0; y < gridHeight; y++) {
        for (int x = 0; x < gridWidth; x++) {
            if (gridSizes[y * gridWidth + x] != 1) {
                return 0;
            }
        }
    }
    return 1;
}

int getLowestEntropyTile() {
    int lowestIndexes[gridWidth * gridHeight];
    int lowestIndexesSize = 0;
    int lowestCount = tileCount * 2;
    for (int y = 0; y < gridHeight; y++) {
        for (int x = 0; x < gridWidth; x++) {
            int index = y * gridWidth + x;
            int tileLen = gridSizes[index];
            if (tileLen > 1) {
                if (tileLen < lowestCount) {
                    lowestCount = tileLen;
                    lowestIndexes[0] = index;
                    lowestIndexesSize = 1;
                } else if (tileLen == lowestCount) {
                    lowestIndexes[lowestIndexesSize] = index;
                    lowestIndexesSize++;
                }
            }
        }
    }

    int randomIndex = xorshift32() % lowestIndexesSize;
    return lowestIndexes[randomIndex];
}

void collapseAtTileRandom(int tileIndex) {
    int currentGridIndex = tileIndex * tileCount;

    // Pick a random superposition at tile and collapse
    int count = gridSizes[tileIndex];
    int randomIndex = xorshift32() % count;

    int value = grid[currentGridIndex + randomIndex];

    gridSizes[tileIndex] = 1;
    grid[currentGridIndex] = value;

    enqueue(tileIndex);
}

void updateQueue() {
    // Use an O(1) lookup table instead of using an O(n) search
    int validNeighborsLUT[tileCount];

    while (headNode != NULL) {
        int tileIndex = dequeue();

        for (int dirIndex = 0; dirIndex < 4; dirIndex++) {
            for(int i = 0; i < tileCount; ++i) {
                validNeighborsLUT[i] = 0;
            }

            // Aggregate constraints from all parent tiles
            for (int parentTileIndex = 0; parentTileIndex < gridSizes[tileIndex]; parentTileIndex++) {
                int parentTile = grid[tileIndex * tileCount + parentTileIndex];
                int constraintsSize = neighborsSizes[parentTile * 4 + dirIndex];
                int neighborsDirOffsetIndex = parentTile * 4 * tileCount + dirIndex * tileCount;

                for (int i = 0; i < constraintsSize; i++) {
                    int currentConstraint = neighbors[neighborsDirOffsetIndex + i];
                    validNeighborsLUT[currentConstraint] = 1;
                }
            }

            if (inGrid(tileIndex, dirIndex)) {
                int childTileIndex = tileIndex + (DIRS[dirIndex][1] * gridWidth + DIRS[dirIndex][0]);
                int childTileSize = gridSizes[childTileIndex];

                if (childTileSize > 1) {
                    int newChildSuperpositions[childTileSize];
                    int newChildSuperpositionsSize = 0;

                    for (int i = 0; i < childTileSize; i++) {
                        int childSuperposition = grid[childTileIndex * tileCount + i];
                        if (validNeighborsLUT[childSuperposition]) { 
                            newChildSuperpositions[newChildSuperpositionsSize++] = childSuperposition;
                        }
                    }

                    // Update grid with new superpositions
                    for (int i = 0; i < newChildSuperpositionsSize; i++) {
                        grid[childTileIndex * tileCount + i] = newChildSuperpositions[i];
                    }
                    gridSizes[childTileIndex] = newChildSuperpositionsSize;

                    if (newChildSuperpositionsSize < childTileSize) {
                        enqueue(childTileIndex);
                    }

                    if (newChildSuperpositionsSize == 0) {
                        printf("ERROR: The number of superpositions in a tile should never be 0\n");
                        printf("This means that something is wrong in the tileset constraints\n");
                        exit(1);
                    }
                }
            }
        }
    }
}

void printGridPython() {
    printf("[");
    for (int y = 0; y < gridHeight; y++) {
        printf("[");
        for (int x = 0; x < gridWidth; x++) {
            int index = (y * gridWidth + x);
            int count = gridSizes[index];
            printf("[");
            for (int a = 0; a < count; a++) {
                printf("%d", grid[index * tileCount + a]);
                if (a < count - 1) {
                    printf(", ");
                }
            }
            printf("]");
            if (x < gridWidth - 1) {
                printf(", ");
            }
        }
        printf("]");
        if (y < gridHeight - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <map json>\n", argv[0]);
        return 1;
    }

    if (rSeed == 0) {
        rSeed = (int)time(NULL);
    }

    init_xorshift(rSeed);
    printf("seed: %d\n", rSeed);

    loadJSON(argv[1]);

    tileCount = countTiles();
    printf("Tile Count: %d\n", tileCount);

    setupGrid();
    setupNeighbors();

    clock_t startTime = clock();

    while (!isFullyCollapsed()) {
        int lowestEntropyTile = getLowestEntropyTile();
        collapseAtTileRandom(lowestEntropyTile);

        updateQueue();
    }

    clock_t endTime = clock();
    double duration = (double)(endTime - startTime) / CLOCKS_PER_SEC;

    printf("done!\n");
    printf("duration: %f seconds\n\n", duration);

    printGridPython();

    // Free allocated memory
    free(grid);
    free(gridSizes);

    free(neighbors);
    free(neighborsSizes);

    return 0;
}
