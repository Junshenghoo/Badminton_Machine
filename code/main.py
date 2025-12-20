import subprocess

num = input("number")

if num == 1:
    subprocess.run(["python", "detection.py"])
elif num == 2:
    