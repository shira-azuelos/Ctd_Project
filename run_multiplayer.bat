@echo off
echo ===================================================
echo   Starting KungFu Chess Multiplayer Test Environment
echo ===================================================

if not "%DevEnvDir%" == "" goto skip_vcvars
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if %errorlevel% neq 0 (
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
)
if %errorlevel% neq 0 (
    call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
)
:skip_vcvars

echo.
echo Compiling...
cl.exe /EHsc /std:c++17 /DASIO_STANDALONE /DASIO_ENABLE_OLD_SERVICES /DBOOST_ASIO_ENABLE_OLD_SERVICES /D_WEBSOCKETPP_CPP11_STL_ /Ithirdparty/asio /Ithirdparty/websocketpp /Iinclude /Iinclude/view /I"C:\Users\shira\Downloads\opencv\build\include" main.cpp src\pubsub\*.cpp src\network\*.cpp src\model\*.cpp src\rules\*.cpp src\realtime\*.cpp src\engine\*.cpp src\input\*.cpp src\io\*.cpp src\view\*.cpp /Fe:app.exe /link /LIBPATH:"C:\Users\shira\Downloads\opencv\build\x64\vc16\lib" opencv_world4120.lib ws2_32.lib winmm.lib

if %errorlevel% neq 0 (
    echo.
    echo COMPILATION FAILED. Fix errors above before running.
    pause
    exit /b 1
)

echo Compilation successful!
echo.

echo 1. Launching WebSocket++ Server in a new terminal window...
start "KungFu Chess Server" cmd /k "app.exe server > server_stdout.log 2>&1"

echo Waiting for server to initialize...
ping 127.0.0.1 -n 3 > nul

echo 2. Launching Player 1 (shira)...
start "KungFu Chess - Player 1 (shira)" app.exe client 127.0.0.1 shira 123

echo Waiting...
ping 127.0.0.1 -n 2 > nul

echo 3. Launching Player 2 (uria)...
start "KungFu Chess - Player 2 (uria)" app.exe client 127.0.0.1 uria 123

echo Waiting...
ping 127.0.0.1 -n 2 > nul

echo 4. Launching Player 3 (yael)...
start "KungFu Chess - Player 3 (yael)" app.exe client 127.0.0.1 yael 123

echo.
echo All 4 processes (1 Server + 3 Clients: White, Black, Spectator) launched successfully!
echo Move the windows side-by-side:
echo   - Player 1 Creates or Joins a Room (White)
echo   - Player 2 Joins the same Room ID (Black - Game Starts!)
echo   - Player 3 Joins the same Room ID (Spectator Mode - Live Stream!)
echo.
echo To exit: Close the game windows.
