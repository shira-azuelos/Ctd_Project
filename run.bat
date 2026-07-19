@echo off
if not "%DevEnvDir%" == "" goto skip_vcvars

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if %errorlevel% neq 0 (
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
)
if %errorlevel% neq 0 (
    call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
)

:skip_vcvars

echo Compiling game app...
cl.exe /EHsc /std:c++17 /DASIO_STANDALONE /DASIO_ENABLE_OLD_SERVICES /DBOOST_ASIO_ENABLE_OLD_SERVICES /D_WEBSOCKETPP_CPP11_STL_ /Ithirdparty/asio /Ithirdparty/websocketpp /Iinclude /Iinclude/view /I"C:\Users\shira\Downloads\opencv\build\include" main.cpp src\pubsub\*.cpp src\network\*.cpp src\model\*.cpp src\rules\*.cpp src\realtime\*.cpp src\engine\*.cpp src\input\*.cpp src\io\*.cpp src\view\*.cpp /Fe:app.exe /link /LIBPATH:"C:\Users\shira\Downloads\opencv\build\x64\vc16\lib" opencv_world4120.lib ws2_32.lib

if %errorlevel% equ 0 (
    echo Compilation successful! Running KungFu Chess...
    app.exe
) else (
    echo Compilation failed. Please fix errors before running.
)