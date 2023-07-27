@echo off

setlocal enabledelayedexpansion

echo Compiling Engine Shaders...

if not exist Embedded mkdir Embedded

for %%F in (*.vert) do (
    "glslangValidator.exe" -g -V -x -o "Embedded\%%F.embed" "%%F"
)
for %%F in (*.frag) do (
    "glslangValidator.exe" -g -V -x -o "Embedded\%%F.embed" "%%F"
)

echo Finished compiling Engine Shaders

Pause