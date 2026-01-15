@echo off
REM MinPilot UI Simulator 실행 스크립트

echo [MinPilot UI Simulator] 시작 중...
echo.

REM WSL을 통해 빌드 및 실행
wsl -d Ubuntu -- bash -c "cd /mnt/e/c2/MinPilot/tools/ui_simulator && chmod +x build.sh && ./build.sh && cd build && ./ui_simulator"

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [오류] 실행에 실패했습니다.
    echo Qt5가 설치되어 있는지 확인하세요:
    echo   wsl -d Ubuntu -- sudo apt install qtbase5-dev
    echo.
    pause
)
