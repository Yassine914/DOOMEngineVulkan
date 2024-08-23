@echo off

SetLocal EnableDelayedExpansion

pushd res\shaders

echo vertex shaders to compile =^>

for /r %%f in (*.vert) do (
    set vertFile=%%~f
    set vertFile=!vertFile:~54!
    echo res/shaders/!vertFile!
    echo     compiled: res/shaders/!vertFile!.spv
    rem compile
    glslc !vertFile! -o ./!vertFile!.spv
)

echo -------------------------------

echo vertex shaders to compile =^>

for /r %%f in (*.frag) do (
    set fragFile=%%~f
    set fragFile=!fragFile:~54!
    echo res/shaders/!fragFile!
    echo     compiled: res/shaders/!fragFile!.spv
    rem compile
    glslc !fragFile! -o ./!fragFile!.spv
)

popd
exit