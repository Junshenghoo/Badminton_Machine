import socket
import json
import random
from time import sleep

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect(('localhost', 9999))

buffer = ""
cx_max = 600
cy_max = 660

def target_zones(row, col):
    row_ls = [1, 2, 3]
    col_ls = [1, 2, 3]
    row_ls.remove(row) 
    col_ls.remove(col) 
    rand_row = random.choice(row_ls)
    rand_col = random.choice(col_ls)
    target = (rand_row - 1) * 3 + rand_col
    return target

def zone_tracking(cx, cy):
    x1 = cx_max/3
    x2 = cx_max*(2/3)
    y1 = cy_max/3
    y2 = cy_max*(2/3)

    if cx < x1:
        col = 1
    elif cx > x1 and cx < x2:
        col = 2
    else:
        col = 3
    if cy < y1:
        row = 1
    elif cy > y1 and cy < y2:
        row = 2
    else:
        row = 3

    target = target_zones(row, col)
    return row, col, target

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
            # print(f"Received cx={cx}, cy={cy}")
            row, col, target = zone_tracking(cx, cy)
            print(f"Received row={row}, col={col}, target={target}")
        sleep(1)