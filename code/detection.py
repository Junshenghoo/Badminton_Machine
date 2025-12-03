import cv2
import time
from ultralytics import YOLO
import torch
import numpy as np
import socket
import json
import subprocess

cx = 0
cy = 0
# Load YOLOv10 nano model
model = YOLO("yolov11n.pt")
device = 'cuda' if torch.cuda.is_available() else 'cpu'
model.to(device)

def setup_socket():
    import atexit

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(('localhost', 9999))
    server_socket.listen(1)
    print("Waiting for client connection...")

    # Launch subprocess and keep reference
    process = subprocess.Popen(["python", "coordinates.py"])

    # Ensure subprocess is killed when main program exits
    atexit.register(lambda: process.terminate())

    conn, addr = server_socket.accept()
    print(f"Connected by {addr}")
    return conn, process

def send_coordinates(cx, cy, conn):
    data = json.dumps({"cx": cx, "cy": cy})
    conn.sendall(data.encode() + b'\n')  # Send data with newline as delimiter

def overlay_image_alpha(img, overlay, x, y, alpha=0.5):
    h, w = overlay.shape[:2]
    if y + h > img.shape[0] or x + w > img.shape[1]:
        return  # Prevent overflow
    roi = img[y:y+h, x:x+w]
    blended = cv2.addWeighted(roi, 1 - alpha, overlay, alpha, 0)
    img[y:y+h, x:x+w] = blended

def  virtual_court_display(resized_frame, cx, cy):
    court_img = cv2.imread("court.png", cv2.IMREAD_UNCHANGED)

    # Resize the court to fit nicely in the corner (e.g., 120px tall)
    overlay_height = 240
    aspect_ratio = court_img.shape[1] / court_img.shape[0]
    overlay_width = int(overlay_height * aspect_ratio)
    court_resized = cv2.resize(court_img, (overlay_width, overlay_height))
    frame_h, frame_w = resized_frame.shape[:2]
    x_offset = frame_w - overlay_width - 10
    y_offset = 10
    
    # Resize court to match 3-channel BGR (drop alpha if present)
    court_rgb = cv2.cvtColor(court_resized, cv2.COLOR_BGRA2BGR) if court_resized.shape[2] == 4 else court_resized
    x = [9, 27, 115, 115 + (115 - 27), 115 + (115 - 9)] #221
    y = [8, 32, 162, 211]

    virtualCourtX = 212
    virtualCourtY = 203
    dot_x = int(((cx * 212 ) / 600) + 9)
    dot_y = int(((cy * 203 ) / 660) + 8)

    cv2.line(court_rgb, (int(10+(virtualCourtX)/3), 10), (int(10+(virtualCourtX)/3), virtualCourtY+10), color=(0, 255, 255), thickness=2)
    cv2.line(court_rgb, (int(10+(virtualCourtX)*2/3), 10), (int(10+(virtualCourtX)*2/3), virtualCourtY+10), color=(0, 255, 255), thickness=2)
    cv2.line(court_rgb, (10, int(10+(virtualCourtY)/3)), (virtualCourtX+10, int(10+(virtualCourtY)/3)), color=(0, 255, 255), thickness=2)
    cv2.line(court_rgb, (10, int(10+(virtualCourtY)*2/3)), (virtualCourtX+10, int(10+(virtualCourtY)*2/3)), color=(0, 255, 255), thickness=2)

    if cx != 0 and cy != 0:
        cv2.circle(court_rgb, (dot_x, dot_y), radius=4, color=(0, 0, 255), thickness=-1)

    overlay_image_alpha(resized_frame, court_rgb, x_offset, y_offset, alpha=0.5)

    return(virtualCourtX, virtualCourtY)

user_dots = []  # Global list for user clicked dots

def mouse_callback(event, x, y, flags, param):
    if event == cv2.EVENT_LBUTTONDOWN:
        if len(user_dots) < 4:
            print(f"User clicked at: ({x}, {y})")
            user_dots.append((x, y))
        else:
            print("Maximum of 4 dots reached. Cannot add more.")

def perspective_transform(tl, bl, tr, br, frame):
    pts1 = np.float32([tl, bl, tr, br])
    pts2 = np.float32([[0, 0], [0, 660], [600, 0], [600, 660]])

    matrix = cv2.getPerspectiveTransform(pts1, pts2)
    transformed_frame = cv2.warpPerspective(frame, matrix, (600, 660))
    return transformed_frame

def detect_red_dot_opencv(transformed_frame):
    # Create a mask for exactly red pixels (pure red in BGR)
    red_mask = cv2.inRange(transformed_frame, (0, 0, 255), (0, 0, 255))

    # Optional: Clean up very small noise
    kernel = np.ones((3, 3), np.uint8)
    red_mask = cv2.morphologyEx(red_mask, cv2.MORPH_OPEN, kernel)

    # Find connected components (blobs of red pixels)
    num_labels, labels, stats, centroids = cv2.connectedComponentsWithStats(red_mask)

    for i in range(1, num_labels):  # Skip background (label 0)
        pixel_count = stats[i, cv2.CC_STAT_AREA]
        if pixel_count >= 4:
            cx, cy = map(int, centroids[i])
            coords_text = f"({cx}, {cy})"
            cv2.putText(transformed_frame, coords_text, (cx + 10, cy),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 0, 255), 1)
            #cv2.imshow("Exact Red Dot Detection", transformed_frame)
            return cx, cy

    return None, None

def main_program(cap):
    conn, process  = setup_socket()
    cx, cy = 0, 0
    cv2.namedWindow("YOLOv10 Detection with Red Dot", cv2.WINDOW_NORMAL)
    # cv2.setWindowProperty("YOLOv10 Detection with Red Dot", cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)
    cv2.resizeWindow("YOLOv10 Detection with Red Dot", 1080, 640)
    cv2.setMouseCallback("YOLOv10 Detection with Red Dot", mouse_callback)
    try:
        while cap.isOpened() and cv2.getWindowProperty("YOLOv10 Detection with Red Dot", cv2.WND_PROP_VISIBLE) >= 1:
            ret, frame = cap.read()
            if not ret:
                break

            start_time = time.time()
            width = int(1080*1.5)
            height = int(640*1.5)
            resized_frame = cv2.resize(frame, (width, height))
            results = model(resized_frame, verbose=False)
            boxes = results[0].boxes

            # Draw YOLO detections
            for box in boxes:
                x1, y1, x2, y2 = map(int, box.xyxy[0])
                cls = int(box.cls[0])
                conf = float(box.conf[0])

                if model.names[cls] == "remote" or model.names[cls] == "person": #person
                    cv2.rectangle(resized_frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
                    label = f"{model.names[cls]} {conf:.2f}"
                    cv2.putText(resized_frame, label, (x1, y1 - 10),
                                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 255), 2)
                    mid_x = x1 + (x2 - x1) // 2
                    bottom_y = y2
                    cv2.circle(resized_frame, (mid_x, bottom_y), 4, (0, 0, 255), -1)
                    coordinates = f"({mid_x}, {bottom_y})"
                    # cv2.putText(resized_frame, coordinates, (mid_x + 5, bottom_y),
                    #             cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 1)

            # Draw user-added dots
            for idx, (dx, dy) in enumerate(user_dots, start=1):
                cv2.circle(resized_frame, (dx, dy), 6, (255, 0, 0), -1)  
                cv2.putText(resized_frame, f"{idx}", (dx + 8, dy),  # show the dot number
                            cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 0, 0), 2)

            fps = 1 / (time.time() - start_time)
            cv2.putText(resized_frame, f"FPS: {fps:.2f}", (10, 25),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 0, 0), 2)

            if len(user_dots) == 4:
                transformed_frame = perspective_transform(user_dots[0], user_dots[1], user_dots[2], user_dots[3], resized_frame)
                cx, cy = detect_red_dot_opencv(transformed_frame)
                send_coordinates(cx, cy, conn)
                if cx is not None and cy is not None:
                    pass
                    # print("Red dot detected at:", cx, cy)
                else:
                    cx, cy = 0, 0
            else:
                cx, cy = 0, 0

            virtual_court_display(resized_frame, cx, cy)    
            
            cv2.imshow("YOLOv10 Detection with Red Dot", resized_frame)

            key = cv2.waitKey(1) & 0xFF
            if key == 27:  # ESC to exit
                break
            elif key == ord('u'):  # Undo last dot
                print("user_dots=", user_dots)
                if user_dots:
                    removed = user_dots.pop()
                    print(f"Removed dot at: {removed}")
                else:
                    print("No dots to remove.")
    finally:
        # Clean up
        process.terminate()          # kill subprocess
        cap.release()
        cv2.destroyAllWindows()
        print("Program exited and subprocess terminated.")


def main():
    #choice = input("Choose an option:\n1) Stream\n2) Recorded Video\nEnter 1 or 2: ")
    choice = "2"

    if choice == "1":
        # Open webcam
        cap = cv2.VideoCapture(0)
        cap.set(cv2.CAP_PROP_FRAME_WIDTH, 320 * 3)
        cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 240 * 3)
        main_program(cap)

    elif choice == "2":
        # video_path = input("Enter path to the recorded video file: ")
        # video_path = f"{video_path}.mp4"
        video_path = "test.mp4"
        cap = cv2.VideoCapture(video_path)
        if not cap.isOpened():
            print("Failed to open video. Please check the path.")
        else:
            main_program(cap)

    else:
        print("Invalid choice. Please enter 1 or 2.")

if __name__ == "__main__":
    main()