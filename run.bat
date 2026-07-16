@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if %errorlevel% neq 0 (
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
)
if %errorlevel% neq 0 (
    call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
)

if "%OPENCV_DIR%"=="" (
    echo ERROR: OPENCV_DIR environment variable is not set.
    echo Please set it to your OpenCV build directory.
    echo Example: C:\Users\YourName\Downloads\opencv\build
    exit /b 1
)

echo Compiling game app...
cl.exe /EHsc /std:c++17 /Iinclude /Iinclude/view /I"%OPENCV_DIR%\include" main.cpp src\app_runner.cpp src\model\*.cpp src\rules\*.cpp src\realtime\*.cpp src\engine\*.cpp src\input\*.cpp src\io\*.cpp src\view\*.cpp /Fe:app.exe /link /LIBPATH:"%OPENCV_DIR%\x64\vc16\lib" opencv_world4120.lib

if %errorlevel% equ 0 (
    echo Compilation successful! Running KungFu Chess...
    app.exe
) else (
    echo Compilation failed. Please fix errors before running.
)