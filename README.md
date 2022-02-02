Hardik Paliwal

commands to run:
premake4 gmake
make config=release
cp A4 Assets/.
./A4 test.lua

![example scene](/screenshot.png)

Simple ray tracer. Takes in sphere, cube or plane primitives. Also supports triangle meshes using .obj file. 

Rebder scene must be writtin in .lua files, see /Assets for examples. 

This is a multi threaded ray tracer, number of threads can be set in your .lua file.  
