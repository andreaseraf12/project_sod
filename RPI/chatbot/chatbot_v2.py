#from telegram import Update 
import telegram
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


TOKEN="6440858737:AAFPy60W_qshW-Cq2YkriSY1ErOgaTFDgGA"
CHAT_ID_LUCA="1026809033"
CHAT_ID_SILVIA="450288607"
CHAT_ID_ANDREA="162060054"
chat_utenti = [CHAT_ID_LUCA,CHAT_ID_SILVIA,CHAT_ID_ANDREA]

alert = "la temperatura ha superato la soglia critica"

# gestione dell'alert
async def funz_alert(trigger, contatore):
    bot = telegram.Bot(token=TOKEN)
    if trigger == True and contatore == 1:
        async with bot:
            for id in chat_utenti:
                await bot.send_message(chat_id=id, text=alert)


#gestione dei comandi
def gestione_comandi() -> None:
    #Run bot
    # Create the Application and pass it your bot's token.
    application = Application.builder().token("6440858737:AAFPy60W_qshW-Cq2YkriSY1ErOgaTFDgGA").build()

    # on different commands - answer in Telegram
    application.add_handler(CommandHandler(["grafico"], funz_grafo))

    # Run the bot until the user presses Ctrl-C
    application.run_polling(allowed_updates=telegram.Update.ALL_TYPES)



# Funzione per inviare un grafico tramite il chatbot
async def funz_grafo(update: telegram.Update, context: ContextTypes.DEFAULT_TYPE) -> None:   
 # Calcola il timestamp attuale e quello di un minuto fa
    now = datetime.now()
    one_minute_ago = now - timedelta(minutes=1)

    # Crea la connessione al database
    try:
        conn = mariadb.connect(
            host="127.0.0.1",
            port=3306,
            user="root",
            password="luca",
            autocommit=True,
            database="sod_db"
        )
    except mariadb.Error as e:
        print(f"Error connecting to the database: {e}")
        return

    # Istanzia il cursore
    cur = conn.cursor()

    # Esegue la query per ottenere i dati nell'intervallo di un minuto
    try:
        cur.execute("SELECT * FROM misure WHERE Data >= ? AND Data < ?", (one_minute_ago, now))
        rows = cur.fetchall()
    except mariadb.Error as e:
        print(f"Error executing the query: {e}")
        conn.close()
        return

    # Chiude la connessione al database
    conn.close()

    # Estrai dati dalla lista di tuple
    timestamps = [row[1] for row in rows]
    temperatura = [row[2] for row in rows]
    pressione = [row[3] for row in rows]
    rpm = [row[4] for row in rows]

    # Crea il grafico
    plt.figure(figsize=(10, 6))
    plt.plot(timestamps, temperatura, label='Temperatura')
    plt.plot(timestamps, pressione, label='Pressione')
    plt.plot(timestamps, rpm, label='RPM')
    plt.xlabel('Data e Ora')
    plt.ylabel('Valori')
    plt.title('Andamento delle misure')
    plt.legend()

    # Salva l'immagine su disco
    image_path = "/home/luca/progetto_sod/foto/grafico.png"
    plt.savefig(image_path)
    plt.close()

    #prendo l'id dello user
    user_id = update.message.from_user.id
    
    # Invia l'immagine tramite il chatbot
    await telegram.Bot(token=TOKEN).send_photo(chat_id=update.message.chat_id, photo=open(image_path, 'rb'))
    #send_photo(chat_id=user_id, photo=open(image_path, 'rb'))

    # Elimina l'immagine dal disco dopo l'invio
    os.remove(image_path)



#codice che non viene eseguito:
if __name__ == '__main__':
    gestione_comandi()
