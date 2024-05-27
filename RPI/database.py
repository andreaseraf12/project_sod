import serial
import mariadb
import sys
import chatbot
import asyncio
import ntplib
from time import sleep
from datetime import datetime, timezone

# Aggiungo una singola misurazione
def add_measure(cur, Data, Temperatura, Pressione, RPM):
     cur.execute("INSERT INTO misure(Data, Temperatura, Pressione, RPM) VALUES (?, ?, ?, ?)",
          (Data, Temperatura, Pressione, RPM) )

# Funzione per ottenere l'orario dal server NTP
def get_ntp_time():
	client = ntplib.NTPClient()
	response = client.request(ntp_server)
	return response.tx_time

#Inizializzazione porta seriale
porta_seriale = '/dev/ttyUSB0'
# Specifica la velocità di trasmissione (baud rate)
velocita_trasmissione = 115200
# Apre la connessione seriale
ser = serial.Serial(porta_seriale, velocita_trasmissione)

# Indirizzo di un server NTP 
ntp_server = 'pool.ntp.org' 
# Ottiene l'orario iniziale dal server NTP e sincronizza il modulo RTC all'avvio del sistema 
start_time = get_ntp_time() #tipo float, sono il numero di secondi passati dall'inizio del tempo
date_msg2send = str(datetime.fromtimestamp(start_time)).split(".") 
date_msg2send = date_msg2send[0].replace(" ", "#").replace(":", "#").replace("-","#")
msg = "#sync#"+date_msg2send+"#"
ser.write(msg.encode()) #messaggio di sincronizzazione inviato da RPI a ESP32 è del tipo #sync#anno#mese#giorno#ora#minuti#secondi#

# Inizializzazione contatore per la gestione dell'alert
contatore = 0

# Crea la connessione database tramite connettore python
try:
   conn = mariadb.connect(
      host="127.0.0.1",
      port=3306,
      user="root",
      password="root",
      autocommit=True,
      database="SensorData")

except mariadb.Error as e:
   print(f"Error connecting to the database: {e}")
   sys.exit(1)

# Instanza il cursore
cur = conn.cursor()

# Lettura dei dati da seriale e successivo inserimento nel db
try:
    while True:
	# Lettura del messaggio e split  
        line = ser.readline().decode("utf-8", "ignore").strip()
        line = line.split("#")
        #controllo integrità del messaggio ricevuto
        if(len(line) != 1) and (line[0] == "M!"):
                print(line)
            	#assegnazione dei valori in variabili temporanee
                new_Data = line[1]
                new_Temp = float(line[2])
                new_Pres = float(line[3])
                new_RPM = int(line[4])

                #controllo per l'invio dell'alert
                if new_Temp > 30:
                        contatore= contatore+1
                        asyncio.run(chatbot.funz_alert(True, contatore))
                else:
                        contatore = 0

                #chiamata della funzione per il salvataggio dei dati nel db
                add_measure(cur,new_Data,new_Temp,new_Pres,new_RPM)

except KeyboardInterrupt:
    ser.close()
    print("Connessione al dispositivo ESP32 chiusa.")
    conn.close()
    print("Connessione al Database 'SensorData' chiusa.")

