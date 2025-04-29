// super fast!

#include "cjson/cJSON.c"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
// #include "json.c"

// Grid Stuff
int* grid; // 3D grid (Y, X, Tiles)
int* gridSizes; // Stores the size of the tiles for each grid cell
int tileCount = 0;
const int gridWidth = 40;
const int gridHeight = gridWidth;
const int tileSize = 16;

int DIRS[4][2] = {{ 0, -1 }, { 0, 1 }, { -1, 0 }, { 1, 0 }};
static const char* DIRNAMES[] = {"up", "down", "left", "right"};

// Neighbors Stuff
int* neighbors;
int* neighborsSizes;

// JSON Stuff
cJSON* constraintsJSON = NULL;
char key[20] = "";

// Queue Stuff
typedef struct Node {
    int value;
    struct Node* next;
} Node;

Node* headNode = NULL;
Node* tailNode = NULL;

int rSeed = 0;

char* readFile(const char* filename) {
    char fullFilename[256];
    snprintf(fullFilename, 256, "%s.txt", filename);
    
    FILE* file = fopen(fullFilename, "r");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* data = malloc(length + 1);
    if (!data) {
        fclose(file);
        return NULL;
    }

    fread(data, 1, length, file);
    data[length] = '\0';
    fclose(file);
    return data;
}

void loadJSON(const char* filename) {
    char* data = readFile(filename);

    constraintsJSON = cJSON_Parse(data);
    free(data);
}

int countTiles() {
    int count = 0;
    cJSON* currentElement = NULL;
    cJSON_ArrayForEach(currentElement, constraintsJSON) { count++; }
    return count;
}

void getNeighbors(const char* tileName, const char* direction, int neighborsArray[], int* size) {
    cJSON* tile = cJSON_GetObjectItemCaseSensitive(constraintsJSON, tileName);
    if (cJSON_IsObject(tile)) {
        cJSON* neighbors = cJSON_GetObjectItemCaseSensitive(tile, "neighbors");
        if (cJSON_IsObject(neighbors)) {
            cJSON* direction_array = cJSON_GetObjectItemCaseSensitive(neighbors, direction);
            if (cJSON_IsArray(direction_array)) {
                *size = cJSON_GetArraySize(direction_array);
                for (int i = 0; i < *size && i < tileCount; i++) {
                    neighborsArray[i] = cJSON_GetArrayItem(direction_array, i)->valueint;
                }
            }
        }
    }
}

void setupNeighbors() {
    for (int i = 0; i < tileCount; i++) {
        int neighborsOffsetIndex = i * 4 * tileCount; 

        char* currentTileName = (char*)malloc(20 * sizeof(char));
        sprintf(currentTileName, "tile%d", i);

        for (int j = 0; j < 4; j++) {
            int neighborsDirOffsetIndex = neighborsOffsetIndex + j * tileCount;
            int currentNeighbors[tileCount];
            int currentNeighborsSize = 0;
            getNeighbors(currentTileName, DIRNAMES[j], currentNeighbors, &currentNeighborsSize);
            neighborsSizes[i * 4 + j] = currentNeighborsSize;

            for (int k = 0; k < currentNeighborsSize; k++) {
                neighbors[neighborsDirOffsetIndex + k] = currentNeighbors[k];
            }
        }
    }
}

int contains(int arr[], int size, int num) {
    for (int i = 0; i < size; i++) {
      if (arr[i] == num) {
        return 1;
      }
    }
    return 0;
}

char* arrayToString(int arr[], int size) {
    int maxLen = size * 11 + 2;
    char* result = (char*)malloc(maxLen);
    char* ptr = result;
    ptr += sprintf(ptr, "[");
    for (int i = 0; i < size; i++) {
        if (i != 0) {
            ptr += sprintf(ptr, ", ");
        }
        ptr += sprintf(ptr, "%d", arr[i]);
    }
    sprintf(ptr, "]");
    return result;
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
    // printArray(lowestIndexes, lowestIndexesSize);

    int randomIndex = rand() % lowestIndexesSize;

    return lowestIndexes[randomIndex];
}

void collapseAtTileRandom(int tileIndex) {
    int currentGridIndex = tileIndex * tileCount;

    // Pick a random superposition at tile and collapse
    int count = gridSizes[tileIndex];
    int randomIndex = rand() % count;

    int value = grid[currentGridIndex + randomIndex];

    gridSizes[tileIndex] = 1;
    grid[currentGridIndex] = value;

    enqueue(tileIndex);
}


void updateQueue() {
    while (headNode != NULL) {
        int tileIndex = dequeue();

        for (int dirIndex = 0; dirIndex < 4; dirIndex++) {
            int tileConstraints[tileCount];
            int tileConstraintsSize = 0;

            // Aggregate constraints from all parent tiles
            for (int parentTileIndex = 0; parentTileIndex < gridSizes[tileIndex]; parentTileIndex++) {
                int parentTile = grid[tileIndex * tileCount + parentTileIndex];
                int constraintsSize = neighborsSizes[parentTile * 4 + dirIndex];
                int neighborsDirOffsetIndex = parentTile * 4 * tileCount + dirIndex * tileCount;

                for (int i = 0; i < constraintsSize; i++) {
                    int currentConstraint = neighbors[neighborsDirOffsetIndex + i];
                    if (!contains(tileConstraints, tileConstraintsSize, currentConstraint)) {
                        tileConstraints[tileConstraintsSize] = currentConstraint;
                        tileConstraintsSize++;
                    }
                }
            }

            if (inGrid(tileIndex, dirIndex)) {
                int childTileIndex = tileIndex + (DIRS[dirIndex][1] * gridWidth + DIRS[dirIndex][0]);
                int childTileSize = gridSizes[childTileIndex];
                if (childTileSize > 1) {
                    int childTile[childTileSize];
                    for (int i = 0; i < childTileSize; i++) {
                        childTile[i] = grid[childTileIndex * tileCount + i];
                    }

                    int newChildSuperpositions[tileConstraintsSize + childTileSize];
                    int newChildSuperpositionsSize = 0;

                    // printf("tileConstraints %s\n", arrayToString(tileConstraints, tileConstraintsSize));
                    // printf("currentChildSuperpositions %s\n", arrayToString(childTile, childTileSize));

                    for (int i = 0; i < childTileSize; i++) {
                        if (contains(tileConstraints, tileConstraintsSize, childTile[i])) {
                            newChildSuperpositions[newChildSuperpositionsSize] = childTile[i];
                            newChildSuperpositionsSize++;
                        }
                    }
                    // printf("newChildSuperpositions %s\n\n", arrayToString(newChildSuperpositions, newChildSuperpositionsSize));

                    // Update grid with new superpositions
                    for (int i = 0; i < newChildSuperpositionsSize; i++) {
                        grid[childTileIndex * tileCount + i] = newChildSuperpositions[i];
                    }
                    gridSizes[childTileIndex] = newChildSuperpositionsSize;

                    if (newChildSuperpositionsSize < childTileSize) {
                        // printf("childTileIndex: %d\n\n", childTileIndex);
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




int main() {
    if (rSeed == 0) {
        rSeed = time(NULL);
    }

    srand(rSeed);
    printf("seed: %d\n", rSeed);

    loadJSON("tileset");

    tileCount = countTiles();
    printf("Tile Count: %d\n", tileCount);

    grid = (int*)malloc(gridWidth * gridHeight * tileCount * sizeof(int));
    gridSizes = (int*)malloc(gridWidth * gridHeight * sizeof(int));

    for (int i = 0; i < gridWidth * gridHeight; i++) {
        gridSizes[i] = tileCount;
    }


    setupGrid();

    neighbors = (int*)malloc(tileCount * 4 * tileCount * sizeof(int));
    neighborsSizes = (int*)malloc(tileCount * 4 * sizeof(int));
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

    free(grid);


    return 0;
}
