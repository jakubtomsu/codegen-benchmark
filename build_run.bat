@echo off
cl particles.cpp /O2 /Feparticles_msvc.exe
clang particles.cpp -O3 -o particles_clang.exe
odin build . -o:speed -no-bounds-check -disable-assert -out:particles_odin.exe

echo MSVC
particles_msvc.exe
echo Clang
particles_clang.exe
echo Odin
particles_odin.exe