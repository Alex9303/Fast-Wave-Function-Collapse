#include "cjson/cJSON.c"


int* neighbors;
int* neighborsSizes;
int largestNeighborCount = 0;



char fileName[] = "map8";
cJSON* constraintsJSON = NULL;
char key[20] = "";

int tileCount;


char* indexToName(int index) {
    char* name = (char*)malloc(20 * sizeof(char));
    sprintf(name, "tile%d", index);
    return name;
}

const char* dirIndexToName(int index) {
    const char* directions[] = { "up", "down", "left", "right" };
    return directions[index];
}

void loadJSON(const char* filename) {
    char* data = readFile(filename);

    constraintsJSON = cJSON_Parse(data);
    free(data);
}

int countKeys() {
    int count = 0;
    cJSON* current_element = NULL;
    cJSON_ArrayForEach(current_element, constraintsJSON) { count++; }
    return count;
}

void JSONneighbors(const char* tileName, const char* direction, int neighborsArray[], int* size) {
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
        int neighborsOffsetIndex = i * 4 * (tileCount + 1); 
        const char* currentTileName = indexToName(i);

        for (int j = 0; j < 4; j++) {
            int neighborsDirOffsetIndex = neighborsOffsetIndex + j * (tileCount + 1);
            int currentNeighbors[tileCount];
            int currentNeighborsSize = 0;
            JSONneighbors(currentTileName, dirIndexToName(j), currentNeighbors, &currentNeighborsSize);
            neighbors[neighborsDirOffsetIndex] = currentNeighborsSize;

            for (int k = 0; k < currentNeighborsSize; k++) {
                neighbors[neighborsDirOffsetIndex + 1 + k] = currentNeighbors[k];
            }
        }
    }
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

void printNeighbors() {
    for (int i = 0; i < tileCount; i++) {
        printf("tile%d:\n", i);

        int neighborsOffsetIndex = i * 4 * (tileCount + 1); 
        for (int j = 0; j < 4; j++) {
            printf("\t%s: ", dirIndexToName(j));
            int neighborsDirOffsetIndex = neighborsOffsetIndex + j * (tileCount + 1);
            int currentNeighborsSize = neighbors[neighborsDirOffsetIndex];
            printf("%s", arrayToString(neighbors + neighborsDirOffsetIndex + 1, currentNeighborsSize));
            // printf(" length: %d", currentNeighborsSize);
            printf("\n");
        }
        

        // arrayToString(arr, size)
    }
}