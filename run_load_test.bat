@echo off
echo ===================================================
echo   Compiling and Running Native C++ Load Tester
echo ===================================================

cl /EHsc /std:c++17 /Iinclude load_tester.cpp src\network\socket_client.cpp src\pubsub\message_bus.cpp src\model\board.cpp src\model\game_state.cpp src\model\piece.cpp src\model\position.cpp src\rules\piece_rules.cpp src\realtime\real_time_arbiter.cpp src\realtime\motion.cpp /link ws2_32.lib /out:load_tester.exe

if %ERRORLEVEL% EQU 0 (
    echo Compilation successful! Launching load test...
    load_tester.exe
) else (
    echo Compilation failed.
)
pause
