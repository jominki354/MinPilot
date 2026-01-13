import sys
import os
import glob
import bz2

project_root = os.path.abspath(os.getcwd())
sys.path.append(project_root)

try:
    from cereal import log as capnp_log
except Exception as e:
    sys.exit(1)

log_dir = "e:/c2/sunnypilot_jominki354/rlog"
target_folders = [
    "2026-01-13--08-44-20--0",
    "2026-01-13--08-50-28--0",
    "2026-01-13--08-50-28--1",
]

output_file = "e:/c2/sunnypilot_jominki354/extracted_logs.txt"

with open(output_file, "w", encoding="utf-8") as outfile:
    for folder in target_folders:
        folder_path = os.path.join(log_dir, folder)
        for log_file in ["qlog.bz2", "rlog.bz2"]:
            path = os.path.join(folder_path, log_file)
            if not os.path.exists(path):
                continue

            outfile.write(f"\n\n--- FILE: {folder}/{log_file} ---\n")

            try:
                with open(path, "rb") as f:
                    raw = f.read()
                    try:
                        data = bz2.decompress(raw)
                    except:
                        data = raw

                    try:
                        events = capnp_log.Event.read_multiple_bytes(data)
                        for ent in events:
                            try:
                                w = ent.which()
                                if w == "logMessage":
                                    msg = ent.logMessage
                                    txt = (
                                        msg
                                        if isinstance(msg, str)
                                        else getattr(msg, "msg", str(msg))
                                    )
                                    if any(
                                        k in txt.lower()
                                        for k in [
                                            "error",
                                            "fail",
                                            "exception",
                                            "traceback",
                                            "killed",
                                            "controlsd",
                                        ]
                                    ):
                                        outfile.write(f"[LOG] {txt.strip()}\n")
                                elif w == "carEvents":
                                    for e in ent.carEvents:
                                        if e.name != "none":
                                            outfile.write(f"[EVENT] {e.name}\n")
                                elif w == "initData":
                                    outfile.write(
                                        f"[INIT] {getattr(ent.initData, 'carFingerprint', 'N/A')}\n"
                                    )
                            except:
                                continue
                    except Exception as e:
                        outfile.write(f"[ERROR] Parsing failed: {e}\n")
            except Exception as e:
                outfile.write(f"[ERROR] File read failed: {e}\n")

print("Logs extracted to extracted_logs.txt")
