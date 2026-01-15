#!/bin/bash
# MinPilot UI Simulator 빌드 스크립트
# WSL에서 실행: cd /mnt/e/c2/MinPilot/tools/ui_simulator && ./build.sh

set -e

echo "🔨 MinPilot UI Simulator 빌드 시작..."
echo ""

# 빌드 디렉토리
mkdir -p build
cd build

# CMake 설정
echo "⚙️ CMake 설정 중..."
cmake ..

# 빌드
echo "🔧 컴파일 중..."
make -j$(nproc)

echo ""
echo "✅ 빌드 완료!"
echo ""
echo "📺 실행 방법:"
echo "  ./ui_simulator"
echo ""
echo "💡 Windows에서 더블클릭 실행: run_ui_simulator.bat"
