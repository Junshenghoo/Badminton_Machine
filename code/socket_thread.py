from PyQt6 import QtCore
import socket
import json

class SocketThread(QtCore.QThread):
    data_received = QtCore.pyqtSignal(int, int, int)

    def __init__(self, host="0.0.0.0", port=5005):
        super().__init__()
        self.host = host
        self.port = port
        self.running = True

    def run(self):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.bind((self.host, self.port))
            s.listen(1)
            s.settimeout(1.0)

            print(f"[SOCKET] Listening on {self.host}:{self.port}")

            while self.running:
                try:
                    conn, addr = s.accept()
                    print("[SOCKET] Connected:", addr)

                    with conn:
                        buffer = b""
                        while self.running:
                            data = conn.recv(1024)
                            if not data:
                                break

                            buffer += data
                            while b"\n" in buffer:
                                line, buffer = buffer.split(b"\n", 1)
                                try:
                                    msg = json.loads(line.decode())
                                    r = int(msg.get("row", -1))
                                    c = int(msg.get("col", -1))
                                    t = int(msg.get("target", -1))
                                    self.data_received.emit(r, c, t)
                                except Exception as e:
                                    print("JSON error:", e)

                except socket.timeout:
                    continue
                except Exception as e:
                    print("Socket error:", e)

    def stop(self):
        self.running = False
        self.quit()
        self.wait()
