# Scan an image, extract all unique tiles, and create a json tileset with the same name of the map

import json
from PIL import Image
import os

tileSize = 16

# Set this to the name of the image.
fileName = "map"
mapImage = Image.open(fileName + '.png').convert('RGB')

mapImageTileWidth = mapImage.width // tileSize
mapImageTileHeight = mapImage.height // tileSize

tileImageArray = []

DIRS = {"up": (0, -1), "down": (0, 1), "left": (-1, 0), "right": (1, 0)}

tileDict = {}

def inBounds(x, y):
    return 0 <= x < mapImageTileWidth and 0 <= y < mapImageTileHeight

def cropTile(x, y):
    return mapImage.crop((x * tileSize, y * tileSize, x * tileSize + tileSize, y * tileSize + tileSize))

def makeTileArray():
    for iy in range(mapImageTileHeight):
        for ix in range(mapImageTileWidth):
            currentTile = cropTile(ix, iy)

            if currentTile not in tileImageArray:
                tileImageArray.append(currentTile)

    print("Unique Tiles:", len(tileImageArray))

def findNeighbors():
    for iy in range(mapImageTileHeight):
        for ix in range(mapImageTileWidth):
            currentTile = cropTile(ix, iy)
            currentTileName = "tile" + str(tileImageArray.index(currentTile))

            if currentTileName not in tileDict.keys():
                tileDict[currentTileName] = {}

                tileDict[currentTileName]["location"] = [ix * tileSize, iy * tileSize]

                tileDict[currentTileName]["neighbors"] = {}
                tileDict[currentTileName]["neighbors"]["up"] = []
                tileDict[currentTileName]["neighbors"]["down"] = []
                tileDict[currentTileName]["neighbors"]["left"] = []
                tileDict[currentTileName]["neighbors"]["right"] = []

            for dirName, (tempDirX, tempDirY) in DIRS.items():
                childTileX, childTileY = tempDirX + ix, tempDirY + iy
                if inBounds(childTileX, childTileY):
                    currentChildTile = cropTile(childTileX, childTileY)
                    currentChildTileName = tileImageArray.index(currentChildTile)

                    if currentChildTileName not in tileDict[currentTileName]["neighbors"][dirName]:
                        tileDict[currentTileName]["neighbors"][dirName].append(currentChildTileName)

    print(json.dumps(tileDict))

makeTileArray()
findNeighbors()
