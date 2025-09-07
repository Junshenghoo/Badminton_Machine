import sys
import cv2
import numpy as np
from PyQt6 import QtWidgets, QtCore, QtGui
from PyQt6.QtOpenGLWidgets import QOpenGLWidget
from PyQt6.QtGui import QImage
from PyQt6.QtCore import QTimer

from PyQt6 import uic

class WebcamGLWidget(QOpenGLWidget):
    def __init__(self, parent=None):
        super(WebcamGLWidget, self).__init__(parent)
        self.frame = None

    def set_frame(self, frame):
        self.frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        self.update()  # Trigger paintGL

    def paintGL(self):
        if self.frame is not None:
            h, w, ch = self.frame.shape
            img = QImage(self.frame.data, w, h, ch * w, QImage.Format.Format_RGB888)
            painter = QtGui.QPainter(self)
            painter.drawImage(self.rect(), img.scaled(self.size(), QtCore.Qt.AspectRatioMode.KeepAspectRatio))
            painter.end()

    def close(self):
        if self.cap.isOpened():
            self.cap.release()

# Replace Ui_MainWindow with the version you posted earlier
from badminton_ui import Ui_MainWindow  # or copy/paste your class definition here if not using .ui

class MainWindow(QtWidgets.QMainWindow):
    def __init__(self):
        super(MainWindow, self).__init__()
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)

        # Replace default openGLWidget with our webcam-enabled widget
        self.webcam_widget = WebcamGLWidget(self.ui.centralwidget)
        self.webcam_widget.setGeometry(self.ui.openGLWidget.geometry())
        self.webcam_widget.setObjectName("webcamWidget")

        # Hide the old openGLWidget and insert our new one
        self.ui.openGLWidget.hide()
        self.webcam_widget.show()

    def closeEvent(self, event):
        self.webcam_widget.close()
        super().closeEvent(event)


if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())
