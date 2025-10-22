
# Fast Wave Function Collapse
Wave Function Collapse algorithm written in C

This project is a high-performance C implementation of the Wave Function Collapse (WFC) algorithm. WFC is a procedural generation algorithm that can create complex and non-repetitive patterns from a small set of tiles based on their local adjacency rules. This implementation is designed for speed and efficiency.

## Features

-   **Blazing Fast**: Written in C and optimized for performance. It uses efficient data structures and algorithms, like a queue for propagation and lookup tables for constraint checking, to generate grids quickly.
-   **Memory Efficient**: Designed to handle large grids without consuming excessive amounts of memory.
-   **Portable**: Can be compiled and run on any system with a C compiler (`gcc`) and `make`.

## Project Structure

-   `src/`: Contains the C source code for the WFC algorithm, JSON parsing utilities, and the cJSON library.
-   `tools/`: Includes Python scripts for generating tilesets and a simpler reference implementation of WFC.
-   `Makefile`: The build script for compiling the C project.

## Dependencies

This project relies on the **cJSON** library for parsing the tile constraint files.

-   **cJSON**: [https://github.com/DaveGamble/cJSON](https://github.com/DaveGamble/cJSON)

## How to Build

1.  Clone the repository.
2.  Ensure you have `gcc` and `make` installed on your system.
3.  Place the `cJSON.c` and `cJSON.h` files into a newly created `src/cjson/` directory.
4.  Run the `make` command in the root of the project directory:
    ```bash
    make
    ```
5.  This will compile the source code and create an executable file named `wfc` in the root directory.

## How to Run

After successfully building the project, you can run the program from your terminal. You must provide the path to a valid tileset JSON file as a command-line argument.

```bash
./wfc path/to/your/tileset.json
```

The program will output the seed used for generation, the time it took to collapse the grid, and the final grid represented as a Python-style 2D list.

## Included Tools (Python Scripts)

The `tools/` directory contains helpful Python scripts to assist with creating tilesets and understanding the algorithm.

### `MapToTileset.py`

This script is a utility for automatically generating a tileset JSON file from a sample image.

-   **What it does**: It scans a source image, divides it into tiles of a specified size, and identifies all unique tiles. It then analyzes which tiles are adjacent to each other in the source image to build the neighbor constraint rules required by the WFC algorithm.
-   **Usage**:
    1.  Make sure you have Python and the Pillow library installed (`pip install Pillow`).
    2.  Open the script and set the `fileName` variable to the path of your sample image (without the extension) and `tileSize` to the desired tile dimensions.
    3.  Run the script: `python tools/MapToTileset.py`
    4.  The resulting JSON configuration will be printed to the console, which you can save to a file.

### `WaveFunctionCollapseSimple.py`

This script is a simple, easy-to-understand implementation of the Wave Function Collapse algorithm written in Python.

-   **What it does**: It serves as a clear and readable reference for the core logic of WFC. While not as fast as the C version, it's an excellent tool for learning, debugging, and verifying the behavior of the algorithm. It uses the same JSON tileset format as the C implementation.

## Credits

-   **cJSON**: The project uses the excellent [cJSON library](https://github.com/DaveGamble/cJSON) by Dave Gamble for all JSON parsing.
-   **Assets**: The animation below was created using graphics from the [Tiny 16 Basic asset pack](https://opengameart.org/content/tiny-16-basic) by Sharm, available on OpenGameArt.org.

![animated_fixed](https://github.com/user-attachments/assets/598c0efa-69cd-4c42-a852-9f5079217331)
