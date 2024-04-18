import serial
import mariadb
import sys

# Specifica la porta seriale a cui è collegato l'ESP32
porta_seriale = '/dev/ttyACM0'
# Specifica la velocità di trasmissione (baud rate)
velocita_trasmissione = 115200
# Apre la connessione seriale
ser = serial.Serial(porta_seriale, velocita_trasmissione)

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
#cur.execute("select * from misure")
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
            new_Data = (data[6:])
            new_Temp = float(temperatura[13:])
            new_Pres = (float(pressione[11:]))
            new_RPM = (int(rpm[5:]))
           
            #chiamata della funzione
            add_measure(cur,new_Data,new_Temp,new_Pres,new_RPM)



except KeyboardInterrupt:
    # Gestisce l'interruzione da tastiera (CTRL+C) per chiudere la connessione seriale
    ser.close()
    conn.close()
    print("Connessione seriale e al db chiusi.")
