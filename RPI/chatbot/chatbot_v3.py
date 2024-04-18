import telegram
import asyncio
from telegram import Update
from telegram.ext import Application, CommandHandler, ContextTypes
import serial
import mariadb
import sys 
import os
from datetime import datetime, timedelta
import matplotlib.pyplot as plt
import logging
from numpy import mean, min, max, float16, float32

TOKEN="6440858737:AAFPy60W_qshW-Cq2YkriSY1ErOgaTFDgGA"
alert = "La temperatura ha superato la soglia critica"


async def funz_alert(trigger, contatore):
    bot = telegram.Bot(token=TOKEN)
    if trigger == True and contatore == 1:
        async with bot:
            with open("bot_users.txt", "r") as users_f:
                chat_users = [line.strip() for line in users_f.readlines()]
                for id in chat_users:
                    await bot.send_message(chat_id=id, text=alert)

# Funzione per aggiornare il file degli utenti già registrati
async def start(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    user_id = update.message.from_user.id
    # Apertura del file txt in LETTURA per generare su cui effettuare il controllo sugli utenti già registrati
    with open("bot_users.txt", "r") as users_f:
        users_list = [line.strip() for line in users_f.readlines()] #Creazione della variabile "list" contenente gli utenti già presenti nel file txt
        print(users_list)
        if str(user_id) not in users_list: #controllo dell'esintenza del nuovo id all'interno della lista
            users_list.append(user_id) #Se l'utente NON era già registrato allora lo aggiungiamo in append alla lista

            # Apertura del file di test in modalità SCRITTURA. In questa modalità andremo a sovrascrivere il file txt con
            # la lista degli utenti (precedentemente creata) aggiornata con il nuovo id.
            with open("bot_users.txt", "w") as users_file:
                for list_idx in users_list: #ciclo per la scrittura degli elementi della lista nel file
                    users_file.write(str(list_idx)+"\n")
        else:
            await update.message.reply_text("L'utente "+f'{user_id}'+" ha già inizializzato il bot.")

# Funzione per inviare un grafico tramite il chatbot
async def funz_grafico(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
# Calcola il timestamp attuale e quello di un minuto fa
    now = datetime.now()
    one_minute_ago = now - timedelta(minutes=1)

    # Crea la connessione al database
    try:
        conn = mariadb.connect(
            host="127.0.0.1",
            port=3306,
            user="root",
            password="root",
            autocommit=True,
            database="SensorData"
        )
    except mariadb.Error as e:
        update.message.reply_text("Database non disponibile.")
        print(f"Error connecting to the database: {e}")
        return

    # Istanzia il cursore
    cur = conn.cursor()

    # Esegue la query per ottenere i dati nell'intervallo di un minuto
    try:
        #PROBLEMA IN QUESTA RIGA cur.execute(....
        cur.execute("SELECT * FROM misure") #WHERE Data >= ? AND Data < ?", (one_minute_ago, now))
        rows = cur.fetchall()
        print(rows)
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
    
    #Calcolo dei valori statistici
    temp_mean = mean(temperatura, dtype=float16)
    temp_min = min(temperatura)
    temp_max = max(temperatura)

    pres_mean = mean(pressione, dtype=float16)
    pres_min = min(pressione)
    pres_max = max(pressione)

    rpm_mean = mean(rpm, dtype=float16)
    rpm_min = min(rpm)
    rpm_max = max(rpm)

    # Generazione e concatenazione dei messaggi da inviare come caption della foto
    temp_msg = "TEMPERATURA:\n"+"Media: "+str(temp_mean)+'°C\n'+"Massima: "+str(temp_max)+'°C\n'+"Minima: "+str(temp_min)+'°C\n'
    pres_msg = "PRESSIONE:\n"+"Media: "+str(pres_mean)+'atm\n'+"Massima: "+str(pres_max)+'atm\n'+"Minima: "+str(pres_min)+'atm\n'
    rpm_msg = "VELOCITA' VENTOLA:\n"+"Media: "+str(rpm_mean)+'RPM\n'+"Massima: "+str(rpm_max)+'RPM\n'+"Minima: "+str(rpm_min)+'RPM\n'

    caption_msg_bot = temp_msg+"\n"+pres_msg+"\n"+rpm_msg #Messaggio che verrà inviato come "descrizione" della foto

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
    image_path = "/home/rpi/grafico.png"
    plt.savefig(image_path)
    plt.close()

    #prendo l'id dello user
    user_id = update.message.from_user.id
    
    # Invia l'immagine tramite il chatbot
    await telegram.Bot(token=TOKEN).send_photo(chat_id=update.message.chat_id, photo=open(image_path, 'rb'), caption = caption_msg_bot)

    # Elimina l'immagine dal disco dopo l'invio
    os.remove(image_path)



def gestione_comandi() -> None:
    #Creazione dell'applicazione
    app_bot = Application.builder().token(TOKEN).build()
    
    #Associazione dei comandi inviati dal client (tipo "/start" o "/grafico") alle funzioni in python (sarebbero le funzionalità del bot)
    app_bot.add_handler(CommandHandler("start", start))
    app_bot.add_handler(CommandHandler(["grafico"], funz_grafico))

    #Polling che attende il comando dall'utente
    app_bot.run_polling(allowed_updates=Update.ALL_TYPES)

if __name__ == '__main__':
    gestione_comandi()
