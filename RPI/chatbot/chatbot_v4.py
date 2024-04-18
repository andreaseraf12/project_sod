import telegram
import asyncio
from telegram import Update, InlineKeyboardButton, InlineKeyboardMarkup
from telegram.ext import Application, CommandHandler, ContextTypes, CallbackQueryHandler
import serial
import mariadb
import sys 
import os
from datetime import datetime, timedelta
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
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


async def start(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    user_id = update.message.from_user.id
    with open("bot_users.txt", "r") as users_f:
        users_list = [line.strip() for line in users_f.readlines()]
        if str(user_id) not in users_list: #controllo sull'id
            users_list.append(user_id)
            with open("bot_users.txt", "w") as users_file:
                for list_idx in users_list:
                    users_file.write(str(list_idx)+"\n")
        else:
            await update.message.reply_text("L'utente "+f'{user_id}'+" ha già inizializzato il bot.")


async def interval_options(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """Sends a message with three inline buttons attached."""
    keyboard = [
        [   InlineKeyboardButton("1 minuto", callback_data="1"),
            InlineKeyboardButton("5 minuti", callback_data="5")
        ],
        [InlineKeyboardButton("30 minuti", callback_data="30"),
         InlineKeyboardButton("60 minuti", callback_data="60")],
    ]

    reply_markup = InlineKeyboardMarkup(keyboard)

    await update.message.reply_text("Quale intervallo temporale vuoi selezionare?",reply_markup=reply_markup)



# Funzione per inviare un grafico tramite il chatbot
async def funz_grafico(minutes): #update: Update, context: ContextTypes.DEFAULT_TYPE, minutes) -> None:

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
     #   update.message.reply_text("Database non disponibile.")
        print(f"Error connecting to the database: {e}")
        return

    # Istanzia il cursore
    cur = conn.cursor()
    # Esegue la query per ottenere i dati nell'intervallo di un minuto
    try:
        match minutes:
            case "1":
                cur.execute("SELECT * FROM misure WHERE Data >= NOW() - INTERVAL 1 MINUTE AND Data < NOW()")
            case "5":
                cur.execute("SELECT * FROM misure WHERE Data >= NOW() - INTERVAL 5 MINUTE AND Data < NOW()")
            case "30":
                cur.execute("SELECT * FROM misure WHERE Data >= NOW() - INTERVAL 30 MINUTE AND Data < NOW()")
            case "60":
                cur.execute("SELECT * FROM misure WHERE Data >= NOW() - INTERVAL 60 MINUTE AND Data < NOW()")

        #cur.execute("SELECT * FROM misure WHERE Data >= NOW() - INTERVAL 15 MINUTE AND Data < NOW()") #, tuple((minutes)))
        rows = cur.fetchall()
        #print(rows)
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
    

    temp_len = len(temperatura)
    print(temp_len)   
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

    temp_msg = "TEMPERATURA:\n"+"Media: "+str(temp_mean)+'°C\n'+"Massima: "+str(temp_max)+'°C\n'+"Minima: "+str(temp_min)+'°C\n'
    pres_msg = "PRESSIONE:\n"+"Media: "+str(pres_mean)+'atm\n'+"Massima: "+str(pres_max)+'atm\n'+"Minima: "+str(pres_min)+'atm\n'
    rpm_msg = "VELOCITA' VENTOLA:\n"+"Media: "+str(rpm_mean)+'RPM\n'+"Massima: "+str(rpm_max)+'RPM\n'+"Minima: "+str(rpm_min)+'RPM\n'
    
    if (temp_len >= 59*int(minutes)):
        caption_msg_bot = temp_msg+"\n"+pres_msg+"\n"+rpm_msg
    else:
        nb=("N.B. Non ci sono abbastanza valori nel database per questa richiesta. Le principali principali cause potrebbero essere: \n"
        "1) Nel database non ci sono abbastanza valori per visualizzare l'intervallo richiesto.\n"
        "2) Malfunzionamento del sistema di acquisizione che comporta l'interpolazione dei dati all'interno per gli intervalli mancanti.")
        caption_msg_bot = temp_msg+"\n"+pres_msg+"\n"+rpm_msg+"\n"+nb


    # Crea il grafico
    plt.figure(figsize=(10,6))
    plt.plot(timestamps, temperatura, label='Temperatura')
    plt.plot(timestamps, pressione, label='Pressione')
    plt.plot(timestamps, rpm, label='RPM')
    plt.xlabel('Orario')
    plt.ylabel('Valori')
    plt.title('Andamento delle misure del giorno '+str(timestamps[1].strftime("%Y-%m-%d")))
    plt.legend()
    myFmt = mdates.DateFormatter('%H:%M:%S')
    plt.gca().xaxis.set_major_formatter(myFmt)

    # Salva l'immagine su disco
    image_path = "/home/rpi/grafico.png"
    plt.savefig(image_path)
    plt.close()
    
    return caption_msg_bot


async def button(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """Parses the CallbackQuery and updates the message text."""
    query = update.callback_query

    # CallbackQueries need to be answered, even if no notification to the user is needed
    # Some clients may have trouble otherwise. See https://core.telegram.org/bots/api#callbackquery
    await query.answer()

    await query.edit_message_text(text=f"Hai selezionato: {query.data} minuti")
    msg = await  funz_grafico(minutes = query.data)
    #prendo id dell'utente
    #user_id = update.message.from_user.id
    # Invia l'immagine tramite il bot
    #await telegram.Bot(token=TOKEN).send_photo(chat_id=update.message.chat_id, photo=open("/home/rpi/grafico.png", 'rb'), caption = caption_msg_bot)
    await query.message.reply_photo(photo= open("/home/rpi/grafico.png", 'rb'), caption=msg)
    #Rimuove l'immagine una volta inviata
    os.remove("/home/rpi/grafico.png")

def gestione_comandi() -> None:
    #Creazione dell'applicazione
    app_bot = Application.builder().token(TOKEN).build()
    
    #Associazione dei comandi inviati dal client (tipo "/start" o "/grafico") alle funzioni in python (sarebbero le funzionalità del bot)
    app_bot.add_handler(CommandHandler("start", start))
    app_bot.add_handler(CommandHandler(["grafico"], interval_options))
    app_bot.add_handler(CallbackQueryHandler(button))

    #Polling che attende il comando dall'utente
    app_bot.run_polling(allowed_updates=Update.ALL_TYPES)

if __name__ == '__main__':
    gestione_comandi()
