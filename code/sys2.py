import serial
import time

def sys2_init():
    arduino = serial.Serial(port='COM70', baudrate=9600, timeout=2)
    time.sleep(2)
    return arduino

def main(arduino, command):
    try:
        cmd = command
        arduino.write((cmd + '\n').encode())
        while True:
            response = arduino.readline().decode().strip()
            if response:
                print("[Arduino2]", response)
                if "completed" in response.lower():  # case-insensitive match
                    break
                if "Invalid" in response.lower():  # case-insensitive match
                    break

    except KeyboardInterrupt:
        print("Exiting...")
        arduino.close()

if __name__ == "__main__":
    arduino = sys2_init()
    cmd = input("Enter command: ")
    main(arduino, cmd)