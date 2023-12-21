@echo off
glslangValidator.exe -V ../Engine/Source/Shaders/simple_shader.vert -o ../Engine/Binaries/simple_shader.vert.spv
glslangValidator.exe -V ../Engine/Source/Shaders/simple_shader.frag -o ../Engine/Binaries/simple_shader.frag.spv