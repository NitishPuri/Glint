
@REM glslangValidator.exe src\shaders\shader.vert -V -o vert.spv

mkdir bin\shaders
glslc.exe src\shaders\shader.vert -o bin\shaders\shader.vert.spv
glslc.exe src\shaders\shader.frag -o bin\shaders\shader.frag.spv