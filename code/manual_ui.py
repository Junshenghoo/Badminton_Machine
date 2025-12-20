from PyQt6 import QtCore, QtWidgets
import json
import socket
import sys
import threading

import sys1
import sys2
import detection

launch_en = 1
# =====================================================
# SOCKET THREAD (SERVER)
# =====================================================
class SocketThread(QtCore.QThread):
    data_received = QtCore.pyqtSignal(int, int, int)  # row, col, target

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
                    with conn:
                        data = conn.recv(1024)
                        if not data:
                            continue

                        msg = json.loads(data.decode("utf-8"))
                        row = msg.get("row", -1)
                        col = msg.get("col", -1)
                        target = msg.get("target", -1)

                        self.data_received.emit(row, col, target)

                except socket.timeout:
                    continue
                except Exception as e:
                    print("Socket error:", e)

    def stop(self):
        self.running = False
        self.quit()
        self.wait()


# =====================================================
# READ JSON
# =====================================================
def read_config():
    with open("config.json", "r") as f:
        return json.load(f)["positions"]


# =====================================================
# MAIN WINDOW
# =====================================================
class ManualWindow(QtWidgets.QMainWindow):
    motorSpeedChanged = QtCore.pyqtSignal(int)
    PosYChanged = QtCore.pyqtSignal(int)
    PosXChanged = QtCore.pyqtSignal(int)

    def __init__(self):
        super().__init__()
        self.setWindowTitle("Manual Mode")
        self.setFixedSize(328, 440)


# =====================================================
# UI CLASS
# =====================================================
class Ui_MainWindow:
    def __init__(self, window, arduino1, arduino2, positions):
        self.window = window
        self.arduino1 = arduino1
        self.arduino2 = arduino2
        self.positions = positions
        self.buttons = {}

    def setupUi(self):
        self.centralwidget = QtWidgets.QWidget(self.window)
        self.window.setCentralWidget(self.centralwidget)

        x = [20, 120, 220]
        y = [20, 120, 220]
        pos = 1

        for r in range(3):
            for c in range(3):
                btn = QtWidgets.QPushButton(str(pos), self.centralwidget)
                btn.setGeometry(x[c], y[r], 81, 71)
                btn.clicked.connect(lambda _, p=pos: self.handle_position(p))
                self.buttons[pos] = btn
                pos += 1

        self.startButton = QtWidgets.QPushButton("Start", self.centralwidget)
        self.startButton.setGeometry(20, 320, 81, 71)
        self.startButton.clicked.connect(self.handle_start)

        self.randomButton = QtWidgets.QPushButton("Random", self.centralwidget)
        self.randomButton.setGeometry(120, 320, 81, 71)

        self.launch1Button = QtWidgets.QPushButton("Launch 1", self.centralwidget)
        self.launch1Button.setGeometry(220, 320, 81, 71)
        self.launch1Button.clicked.connect(self.handle_launch1)

    # -------------------------------
    def reset_button_colour(self):
        for btn in self.buttons.values():
            btn.setStyleSheet("")
        self.startButton.setStyleSheet("")
        self.randomButton.setStyleSheet("")
        self.launch1Button.setStyleSheet("")

    def highlight(self, btn):
        self.reset_button_colour()
        btn.setStyleSheet("background-color: lightgreen")

    # -------------------------------
    def handle_position(self, pos):
        if pos not in self.buttons:
            return

        self.highlight(self.buttons[pos])

        cfg = self.positions[str(pos)]
        v = cfg["vertical_angle"]
        h = cfg["horizontal_angle"]
        s = cfg["motor_speed"]

        sys2.main(self.arduino2, f"movePosY_{v}")
        sys1.main(self.arduino1, f"motorLeftRight_{h}")
        sys2.main(self.arduino2, f"motorSpeed_{s}")

        print(f"[POS {pos}] V={v} H={h} S={s}")

    def handle_start(self):
        self.highlight(self.startButton)
        sys2.main(self.arduino2, "system_all")
        self.reset_button_colour()

    def handle_launch1(self):
        self.highlight(self.launch1Button)
        sys2.main(self.arduino2, "system_one")
        self.reset_button_colour()

    def resetAutoMode(self):
        sys2.main(self.arduino2, "resetAutoMode_0")

    def autoMode(self):
        sys2.main(self.arduino2, "autoMode_0")

    def emo(self):
        self.quit()


# =====================================================
# SOCKET → UI HANDLER
# =====================================================
def handle_socket_data(ui, row, col, target):
    global launch_en
    #print(f"[SOCKET] row={row}, col={col}, target={target}")
    #print("emo:",sys2.emo)
    
    if 1 <= target <= 9:
        # if sys2.emo == 1:
        #     self.quit()
        if launch_en == 1:
            launch_en = 0
            ui.handle_position(target)
            ui.autoMode()
            launch_en = 1


# =====================================================
# ENTRY POINT
# =====================================================
def main(arduino1, arduino2, autoMode):
    positions = read_config()
    app = QtWidgets.QApplication.instance()
    if app is None:
        app = QtWidgets.QApplication(sys.argv)

    window = ManualWindow()
    ui = Ui_MainWindow(window, arduino1, arduino2, positions)
    ui.setupUi()
    ui.resetAutoMode()

    if autoMode == 1:

        window.socket_thread = SocketThread()
        window.socket_thread.data_received.connect(
            lambda r, c, t: handle_socket_data(ui, r, c, t)
        )
        window.socket_thread.start()

        # Run detection in background (IMPORTANT)
        threading.Thread(target=detection.main, daemon=True).start()
    else:
        window.show()
    return window, ui
