@echo off
echo ===================================================
echo   Starting KungFu Chess Multiplayer Test Environment
echo ===================================================

echo.
echo 1. Launching WebSocket++ Server in a new terminal window...
start "KungFu Chess Server" cmd /k "app.exe server"

echo Waiting for server to initialize...
timeout /t 2 /nobreak > nul

echo 2. Launching Player 1 (White)...
start "KungFu Chess - Player 1 (White)" app.exe client 127.0.0.1

echo Waiting...
timeout /t 1 /nobreak > nul

echo 3. Launching Player 2 (Black)...
start "KungFu Chess - Player 2 (Black)" app.exe client 127.0.0.1

echo.
echo All processes launched successfully!
echo Move the windows side-by-side to play against yourself in real time.
echo To exit: Close the game windows and press Enter in the server console window.
