from telegram import Update
import asyncio
import serial
import mariadb
import sys
import chatbot_3
from datetime import datetime, timedelta
import os
import matplotlib.pyplot as plt
from telegram.ext import Application, CommandHandler, ContextTypes
import logging



# Specifica la porta seriale a cui è collegato l'ESP32
porta_seriale = '/dev/ttyACM0'
# Specifica la velocità di trasmissione (baud rate)
velocita_trasmissione = 115200
# Apre la connessione seriale
ser = serial.Serial(porta_seriale, velocita_trasmissione)
#trigger che viene messo a True se si supera la seconda soglia
trigger = False
contatore = 0

# Aggiungo una singola misurazione
def add_measure(cur, Data, Temperatura, Pressione, RPM):
     cur.execute("INSERT INTO misure(Data, Temperatura, Pressione, RPM) VALUES (?, ?, ?, ?)",
          (Data, Temperatura, Pressione, RPM) )


# Crea la connessione
try:
   conn = mariadb.connect(
      host="127.0.0.1",
      port=3306,
      user="root",
      password="luca",
      autocommit=True,
      database="sod_db")

except mariadb.Error as e:
   print(f"Error connecting to the database: {e}")
   sys.exit(1)

# Instanza il cursore
cur = conn.cursor()


#leggo la tabella "misure" nel db "sod_db" per verificare se effettivamente si riesce ad operare sul db
#cur.execute("select * from misure where Temperatura > 16")
#for (id,Data,Temperatura,Pressione, RPM) in cur:
#    print(f"{id} {Data} {Temperatura} {Pressione} {RPM}")



# Leggo i dati da seriale e li metto nel db
try:
    while True:
        # Legge una linea dalla porta seriale e la decodifica in UTF-8
        for i in range(0,4):
            data = ser.readline().decode('utf-8').strip()
            temperatura = ser.readline().decode('utf-8').strip()
            pressione = ser.readline().decode('utf-8').strip()
            rpm = ser.readline().decode('utf-8').strip()

            #salvo i valori nel db
            new_Data = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            new_Temp = float(temperatura[13:])
            new_Pres = (float(pressione[11:])/10)
            new_RPM = 0

            #controllo sulla seconda soglia (alert)
            if new_Temp > 25:
                contatore= contatore+1
                asyncio.run(chatbot_3.funz_alert(True, contatore))
            else:
                contatore = 0

            #chiamata della funzione per aggiungere i valori nel db
            add_measure(cur,new_Data,new_Temp,new_Pres,new_RPM)

           

except KeyboardInterrupt:
    # Gestisce l'interruzione da tastiera (CTRL+C) per chiudere la connessione seriale
    ser.close()
    conn.close()
    print("Connessione seriale e al db chiusi.")
