import serial 
import time 
from time import sleep

port = '/dev/ttyUSB0'
baud_rate = 115200
#Apre connessione seriale
ser = serial.Serial(port, baud_rate)
sleep(6)

try:
    while True:
	# Leggo il messaggio e lo divido 
        line = ser.readline().decode("utf-8", "ignore").strip()
        line = line.split("#")

        if(len(line) != 1) and (line[0] == "M!"):
                print(line)
                new_Data = line[1]
                new_Temp = float(line[2])
                new_Pres = float(line[3])
                new_RPM = int(line[4])


except KeyboardInterrupt:

	ser.close()
	print("Serial connection closed.")
