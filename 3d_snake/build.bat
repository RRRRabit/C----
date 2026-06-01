@echo off
echo =====================================
echo  3D Snake Game - Build Script
echo =====================================
echo.

set RAYLIB_INCLUDE=third_party\raylib\include
set RAYLIB_LIB=third_party\raylib\lib

echo Compiling...
g++ *.cpp -o snake.exe ^
    -I%RAYLIB_INCLUDE% ^
    -L%RAYLIB_LIB% ^
    -lraylib ^
    -lopengl32 -lgdi32 -lwinmm ^
    -static-libgcc -static-libstdc++ ^
    -O2 ^
    -Wall

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Build FAILED!
    pause
    exit /b 1
)

echo.
echo Build SUCCESS!
echo Run snake.exe to play the game.
echo.
pause
