
@REM glslangValidator.exe src\shaders\shader.vert -V -o vert.spv

@REM TODO: Add Cmake step to automatically compile all shaders using glslc

mkdir bin\shaders
glslc.exe src\shaders\basic.vert -o bin\shaders\basic.vert.spv
glslc.exe src\shaders\shader.vert -o bin\shaders\shader.vert.spv
glslc.exe src\shaders\shader.frag -o bin\shaders\shader.frag.spv