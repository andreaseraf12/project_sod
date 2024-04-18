import serial
import mariadb
import sys
import chatbot
import asyncio
	
# Specifica la porta seriale a cui è collegato l'ESP32
porta_seriale = '/dev/ttyUSB0'
# Specifica la velocità di trasmissione (baud rate)
velocita_trasmissione = 115200
# Apre la connessione seriale
ser = serial.Serial(porta_seriale, velocita_trasmissione)
# Inizializzazione contatore
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
      password="root",
      autocommit=True,
      database="SensorData")

except mariadb.Error as e:
   print(f"Error connecting to the database: {e}")
   sys.exit(1)

# Instanza il cursore
cur = conn.cursor()


#leggo la tabella "misure" nel db "sod_db" per verificare se effettivamente si riesce ad operare sul db
#cur.execute("select * from misure")
#for (id,Data,Temperatura,Pressione, RPM) in cur:
#    print(f"{id} {Data} {Temperatura} {Pressione} {RPM}")


# Leggo i dati da seriale e li metto nel db
try:
    while True:
	# Leggo il messaggio e lo divido 
        line = ser.readline().decode("utf-8", "ignore").strip()
        line = line.split("#")
        if(len(line) != 1):
            	#salvo i valori nel db
                new_Data = line[0]
                new_Temp = float(line[1])
                new_Pres = float(line[2])
                new_RPM = int(line[3])
                #controllo sulla seconda soglia (alert)
                if new_Temp > 19:
                        contatore= contatore+1
                        asyncio.run(chatbot.funz_alert(True, contatore))
                else:
                        contatore = 0

                #chiamata della funzione
                add_measure(cur,new_Data,new_Temp,new_Pres,new_RPM)



except KeyboardInterrupt:
    # Gestisce l'interruzione da tastiera (CTRL+C) per chiudere la connessione seriale
    ser.close()
    print("Connessione al dispositivo ESP32 chiusa.")
    conn.close()
    print("Connessione al Database 'SensorData' chiusa.")

