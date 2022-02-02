## How to get started:

````
premake4 gmake
make config=release
cp A4 Assets/.
./A4 test.lua
````
![example scene](/screenshot.png)

Simple ray tracer. Takes in sphere, cube or plane primitives. Also supports triangle meshes using .obj file. 

Render scenes must be writtin in .lua files, see /Assets for examples. 

This is a multi threaded ray tracer, number of threads can be set in your scene file.  
