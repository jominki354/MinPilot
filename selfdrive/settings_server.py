#!/usr/bin/env python3
"""
CarrotMan 원격 설정 서버
- HTTP 8082 포트로 설정 읽기/쓰기 API 제공
- CarrotMan 앱에서 openpilot 설정 변경 가능
"""

import json
from http.server import HTTPServer, BaseHTTPRequestHandler
from common.params import Params

PORT = 8082

# 원격에서 변경 가능한 파라미터 목록 (manager.py 기준)
ALLOWED_PARAMS = [
    "ACCMADSCombo",
    "AutoLaneChangeTimer",
    "BrightnessControl",
    "CarModel",
    "CarrotSpeedControl",
    "CarrotMobileCamera",
    "CarrotSpeedBump",
    "CarrotSectionControl",
    "DevUI",
    "DisableMADS",
    "DisableOnroadUploads",
    "DynamicLaneProfile",
    "EndToEndToggle",
    "HandsOnWheelMonitoring",
    "MaxTimeOffroad",
    "NoOffroadFix",
    "OnroadScreenOff",
    "OnroadScreenOffBrightness",
    "OpenpilotEnabledToggle",
    "ShowDebugUI",
    "SpeedLimitControl",
    "SpeedLimitPercOffset",
    "TurnSpeedControl",
    "TurnVisionControl",
]


def get_param_safe(params, key):
    """파라미터 안전하게 읽기"""
    try:
        val = params.get(key)
        if val is None:
            return ""
        if isinstance(val, bytes):
            return val.decode("utf-8")
        return str(val)
    except Exception:
        return ""


class SettingsHandler(BaseHTTPRequestHandler):
    params = Params()

    def log_message(self, format, *args):
        # 로그 출력 (선택적)
        print(f"[settings_server] {args[0]}")

    def send_json(self, data, status=200):
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.end_headers()
        self.wfile.write(json.dumps(data).encode("utf-8"))

    def do_OPTIONS(self):
        """CORS preflight 요청 처리"""
        self.send_response(200)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.end_headers()

    def do_GET(self):
        if self.path == "/get_toggle_values":
            # 모든 파라미터 읽기
            values = {}
            for key in ALLOWED_PARAMS:
                values[key] = get_param_safe(self.params, key)
            self.send_json(values)

        elif self.path == "/":
            # 상태 확인용
            self.send_json({"status": "ok", "server": "SunnyPilot-C2 Settings"})

        else:
            self.send_json({"error": "Not Found"}, 404)

    def do_POST(self):
        if self.path == "/store_toggle_values":
            try:
                content_length = int(self.headers.get("Content-Length", 0))
                body = self.rfile.read(content_length)
                data = json.loads(body.decode("utf-8"))

                updated = []
                for key, value in data.items():
                    if key in ALLOWED_PARAMS:
                        try:
                            self.params.put(key, str(value))
                            updated.append(key)
                        except Exception as e:
                            print(f"[settings_server] Failed to set {key}: {e}")

                self.send_json({"success": True, "updated": updated})

            except json.JSONDecodeError:
                self.send_json({"error": "Invalid JSON"}, 400)
            except Exception as e:
                self.send_json({"error": str(e)}, 500)

        else:
            self.send_json({"error": "Not Found"}, 404)


def main():
    print(f"[settings_server] Starting on port {PORT}")
    server = HTTPServer(("0.0.0.0", PORT), SettingsHandler)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("[settings_server] Shutting down")
        server.shutdown()


if __name__ == "__main__":
    main()
