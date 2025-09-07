import sys
import cv2
import numpy as np
import time
from PyQt6 import QtWidgets, QtCore, QtGui
from PyQt6.QtOpenGLWidgets import QOpenGLWidget
from PyQt6.QtGui import QImage
from PyQt6.QtCore import QTimer
from ultralytics import YOLO
import torch

# --- Webcam OpenGL Widget ---
class WebcamGLWidget(QOpenGLWidget):
    def __init__(self, parent=None):
        super(WebcamGLWidget, self).__init__(parent)
        self.frame = None

    def set_frame(self, frame):
        self.frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        self.update()

    def paintGL(self):
        if self.frame is not None:
            h, w, ch = self.frame.shape
            img = QImage(self.frame.data, w, h, ch * w, QImage.Format.Format_RGB888)
            painter = QtGui.QPainter(self)
            painter.drawImage(self.rect(), img.scaled(self.size(), QtCore.Qt.AspectRatioMode.KeepAspectRatio))
            painter.end()

# --- Main Window ---
class MainWindow(QtWidgets.QMainWindow):
    def __init__(self):
        super(MainWindow, self).__init__()
        self.setWindowTitle("YOLOv10 + PyQt6 Viewer")
        self.resize(960, 720)

        self.central_widget = QtWidgets.QWidget()
        self.setCentralWidget(self.central_widget)

        self.layout = QtWidgets.QVBoxLayout(self.central_widget)

        self.webcam_widget = WebcamGLWidget(self.central_widget)
        self.layout.addWidget(self.webcam_widget)

        self.yolo_thread = YoloDetectionThread()
        self.yolo_thread.update_frame_signal.connect(self.webcam_widget.set_frame)
        self.yolo_thread.start()

    def closeEvent(self, event):
        self.yolo_thread.stop()
        self.yolo_thread.wait()
        super().closeEvent(event)

# --- Thread for YOLOv10 detection ---
class YoloDetectionThread(QtCore.QThread):
    update_frame_signal = QtCore.pyqtSignal(np.ndarray)

    def __init__(self):
        super().__init__()
        self.running = True
        self.cap = cv2.VideoCapture(0)
        self.model = YOLO("yolov10n.pt")
        device = 'cuda' if torch.cuda.is_available() else 'cpu'
        self.model.to(device)

    def run(self):
        while self.running and self.cap.isOpened():
            ret, frame = self.cap.read()
            if not ret:
                break

            start = time.time()
            resized_frame = cv2.resize(frame, (960, 640))

            results = self.model(resized_frame, verbose=False)
            boxes = results[0].boxes

            for box in boxes:
                x1, y1, x2, y2 = map(int, box.xyxy[0])
                cls = int(box.cls[0])
                conf = float(box.conf[0])
                label = f"{self.model.names[cls]} {conf:.2f}"

                cv2.rectangle(resized_frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
                cv2.putText(resized_frame, label, (x1, y1 - 10),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 255), 2)

            fps = 1 / (time.time() - start)
            cv2.putText(resized_frame, f"FPS: {fps:.2f}", (10, 25),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 0, 0), 2)

            self.update_frame_signal.emit(resized_frame)

        self.cap.release()

    def stop(self):
        self.running = False

# --- Entry Point ---
if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    win = MainWindow()
    win.show()
    sys.exit(app.exec())
