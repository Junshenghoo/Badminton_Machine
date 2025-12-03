import serial
import time

arduino = serial.Serial(port='COM70', baudrate=9600, timeout=2)
time.sleep(2)

try:
    while True:
        cmd = input("Enter command: ")
        arduino.write((cmd + '\n').encode())

        while True:
            response = arduino.readline().decode().strip()
            if response:
                print("[Arduino]", response)
                if "completed" in response.lower():  # case-insensitive match
                    break
                if "Invalid" in response.lower():  # case-insensitive match
                    break

except KeyboardInterrupt:
    print("Exiting...")
    arduino.close()