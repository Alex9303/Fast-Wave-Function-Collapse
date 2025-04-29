#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include "cjson/cJSON.h"

extern cJSON* constraintsJSON;
extern char key[20];

char* readFile(const char* filename);
void loadJSON(const char* filename);
int countTiles();
void getNeighbors(const char* tileName, const char* direction, int neighborsArray[], int* size);
void setupNeighbors();

#endif