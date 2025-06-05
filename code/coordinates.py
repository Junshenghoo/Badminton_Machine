import socket
import json

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect(('localhost', 9999))

buffer = ""

while True:
    data = client_socket.recv(1024).decode()
    if not data:
        break
    buffer += data
    while '\n' in buffer:
        line, buffer = buffer.split('\n', 1)
        coords = json.loads(line)
        cx = coords["cx"]#600
        cy = coords["cy"]#660
        if cx is not None and cy is not None:
            print(f"Received cx={cx}, cy={cy}")

