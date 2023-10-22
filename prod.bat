make

rmdir build /Q /S

mkdir build
cd build
mkdir assets
mkdir Shaders
cd ..

move main.exe build
copy lib build
copy src\Rendering\Shaders build\Shaders
copy assets build\assets