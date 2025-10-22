# Simple Wave Function Collapse implementation in less than 70 lines (excluding empty lines) without all the extra fluff
# Pretty damn fast

import json
import random
import time

grid = []
queue = []

gridWidth = 20
gridHeight = 20

DIRS = {"up": (0, -1), "down": (0, 1), "left": (-1, 0), "right": (1, 0)}

randomGen = random
seed = int(time.time())
randomGen.seed(seed)

constraintsDict = json.loads(open("tileset.txt").read())
tileCount = len(constraintsDict.keys())

def inGrid(x, y):
    return 0 <= x < gridWidth and 0 <= y < gridHeight

def setupGrid():
    tilePositions = list(range(len(constraintsDict)))
    grid.extend([tilePositions.copy() for _ in range(gridWidth)] for _ in range(gridHeight))

def isFullyCollapsed():
    return all(len(tile) == 1 for row in grid for tile in row)

def getLowestEntropyCoords():
    lowestSet = []
    lowestTileLen = float('inf')
    for iy in range(gridHeight):
        for ix in range(gridWidth):
            tileLen = len(grid[iy][ix])
            if tileLen > 1:
                if tileLen < lowestTileLen:
                    lowestTileLen = tileLen
                    lowestSet = [[ix, iy]]
                elif tileLen == lowestTileLen:
                    lowestSet.append([ix, iy])
    return random.choice(lowestSet)

def collapseAtCoordsRandom(coords):
    x, y = coords
    grid[y][x] = [random.choice(grid[y][x])]
    queue.append([x, y])

def updateQueue():
    while queue:
        parentX, parentY = queue.pop(0)

        for dirName, (tempDirX, tempDirY) in DIRS.items():
            childX = parentX + tempDirX
            childY = parentY + tempDirY

            tileConstraints = set()
            for parentTile in grid[parentY][parentX]:
                parentTileName = "tile" + str(parentTile)
                for constraint in constraintsDict[parentTileName]["neighbors"][dirName]:
                    tileConstraints.add(constraint)

            if inGrid(childX, childY):
                currentChildTile = grid[childY][childX]
                if len(currentChildTile) > 1:
                    currentChildSuperpositions = set(currentChildTile)
                    childSuperpositionsCount = len(currentChildSuperpositions)
                    newChildSuperpositions = list(tileConstraints & currentChildSuperpositions)
                    grid[childY][childX] = newChildSuperpositions

                    if len(newChildSuperpositions) < childSuperpositionsCount:
                        queue.append([childX, childY])

                    if len(newChildSuperpositions) == 0:
                        print("ERROR")
                        print("The number of superpositions in a tile should never be 0")
                        exit()

def run():
    while not isFullyCollapsed():
        coords = getLowestEntropyCoords()
        collapseAtCoordsRandom(coords)
        updateQueue()


print(seed)

setupGrid()

startTime = time.time()
run()
endTime = time.time()
duration = endTime - startTime

print(duration)
print(grid)
print(tileCount)
