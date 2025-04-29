#include "json_utils.h"
#include "wfc.h"
#include <stdio.h>
#include <stdlib.h>
#include "cjson/cJSON.h"

cJSON* constraintsJSON = NULL;
char key[20] = "";

char* readFile(const char* filename) {
    char fullFilename[256];
    snprintf(fullFilename, 256, "%s.txt", filename);

    FILE* file = fopen(fullFilename, "r");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* data = (char*)malloc(length + 1);
    if (!data) {
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(data, 1, length, file);
    if (bytesRead != (size_t)length) {
        free(data);
        fclose(file);
        return NULL;
    }

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
        free(currentTileName);
    }
}
