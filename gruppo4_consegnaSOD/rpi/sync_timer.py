import ntplib
from time import sleep
import serial
from datetime import datetime, timezone

# Indirizzo di un server NTP (puoi utilizzare un server NTP locale o uno pubblico)
ntp_server = 'pool.ntp.org' 

# Funzione per ottenere l'orario dal server NTP
def get_ntp_time():
	client = ntplib.NTPClient()
	response = client.request(ntp_server)
	return response.tx_time

#Inizializzazione porta seriale
port = '/dev/ttyUSB0'
# Specifica la velocità di trasmissione (baud rate)
baud_rate = 115200
# Apre la connessione seriale
ser = serial.Serial(port, baud_rate)

# Ottiene l'orario iniziale dal server NTP e syncronizza il modulo RTC all'avvio del sistema 
start_time = get_ntp_time() #tipo float, sono il numero di secondi passati dall'inizio del tempo
date_msg2send = str(datetime.fromtimestamp(start_time)).split(".") 
date_msg2send = date_msg2send[0].replace(" ", "#").replace(":", "#").replace("-","#")
msg = "#sync#"+date_msg2send+"#"
ser.write(msg.encode()) #messaggio di sincronizzazione inviato da RPI a ESP32 è del tipo #sync#anno#mese#giorno#ora#minuti#secondi#


try:
	# Loop principale del contatore
	while True:
		
    	#Ottiene l'orario corrente dal server NTP
		current_time = get_ntp_time()

    	#Calcola la differenza di tempo in secondi
		elapsed_time = current_time - start_time
	
		if elapsed_time >= 10:
			#Preparazione della data da inviare. Viene prima usato il modulo NPC
			# per richiedere l'orario al server, poi avviene una conversione
			# DateTime2String e successivamente si tolgono i millesimi.
			date_msg2send = str(datetime.fromtimestamp(current_time)).split(".") 
			date_msg2send = date_msg2send[0].replace(" ", "#").replace(":", "#").replace("-","#")
			
			#Creazione della variabile contenente il messaggio da inviare #sync#anno#mese#giorno#ora#minuti#secondi#
			msg = "#sync#"+date_msg2send+"#"
			#Scrittura su seriale per l'inivio del messaggio
			ser.write(msg.encode())
			
			start_time = get_ntp_time()
		print(f"Contatore temporale: {elapsed_time:.2f} secondi")
		sleep(10) #Pausa per non incorrere dell'imporchettamento del sistema 

except KeyboardInterrupt:
	#Chiusura comunicazione
	ser.close()
	print("Contatore spento")
