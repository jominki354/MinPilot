#!/bin/bash
# MinPilot UI 개발 모드 시작 스크립트
# 판다/차량 연결 없이 온로드 UI를 테스트합니다.

cd /data/openpilot

echo "=== MinPilot UI 개발 모드 ==="
echo "카메라 + UI를 판다 없이 띄웁니다."
echo ""

# DevMode 활성화
echo -n "1" > /data/params/d/UIDevMode
echo "[1/4] UIDevMode 활성화"

# 기존 프로세스 정리
pkill -f camerad 2>/dev/null
pkill -f ui_dev_publisher 2>/dev/null
pkill -f "selfdrive/ui/ui" 2>/dev/null
sleep 1

# 카메라 시작
echo "[2/4] 카메라 시작..."
./selfdrive/camerad/camerad &
CAMERAD_PID=$!
sleep 2

# 더미 퍼블리셔 시작
echo "[3/4] 더미 메시지 퍼블리셔 시작..."
python selfdrive/debug/ui_dev_publisher.py &
PUBLISHER_PID=$!
sleep 1

# UI 시작
echo "[4/4] UI 시작..."
echo ""
echo "종료하려면 Ctrl+C를 누르세요."
echo ""
./selfdrive/ui/ui

# 정리
echo ""
echo "UI 종료됨. 프로세스 정리 중..."
kill $CAMERAD_PID 2>/dev/null
kill $PUBLISHER_PID 2>/dev/null
echo -n "0" > /data/params/d/UIDevMode
echo "UIDevMode 비활성화. 완료!"
