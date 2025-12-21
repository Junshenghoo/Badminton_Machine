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
# READ JSON
# =====================================================
def read_config():
    try:
        with open("config.json", "r") as f:
            positions = json.load(f)

        # Safety validation (recommended)
        for i in range(1, 10):
            key = str(i)
            if key not in positions:
                raise ValueError(f"Missing position {key}")

            for field in ("vertical_angle", "horizontal_angle", "motor_speed"):
                if field not in positions[key]:
                    raise ValueError(f"Missing '{field}' in position {key}")

        return positions

    except Exception as e:
        print("[CONFIG ERROR]", e)
        return {}

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

    def emo(self):
        self.quit()



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
    window.show()
    return window, ui
