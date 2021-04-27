# 3D-Engine-2.0
A realtime triangle restrizer and shader written in C++. Supports object files only for rendering and have been tested to render upto 100,000 triangles in realtime. Gives full angle 3D perspective with mouse movements.

# Pre-requisites
* MingW32 g++ Compiler
* SDL 2.0
* OpenMP

# How To Run
Compile binary using following cmd:

``
g++ -g *.cpp -lmingw32 -lSDL2main -lSDL2 -fopenmp
``

After invoking the executable, it will ask for an .obj filename to load.
## Humpback Rendering
![alt text](https://github.com/ShamsArfeen/3D-Engine-2.0/blob/main/ExampleSS.JPG?raw=true)


## Lucy Rendering
![alt text](https://github.com/ShamsArfeen/3D-Engine-2.0/blob/main/Alucy2.JPG?raw=true)
