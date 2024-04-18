import telegram
import asyncio

TOKEN="6440858737:AAFPy60W_qshW-Cq2YkriSY1ErOgaTFDgGA"
CHAT_ID_LUCA="1026809033"
CHAT_ID_SILVIA="450288607"
CHAT_ID_ANDREA="162060054"
chat_utenti = [CHAT_ID_LUCA,CHAT_ID_SILVIA,CHAT_ID_ANDREA]

alert = "la temperatura ha superato la soglia critica"


async def funz_alert(trigger, contatore):
    bot = telegram.Bot(token=TOKEN)
    if trigger == True and contatore == 1:
        async with bot:
            for id in chat_utenti:
                await bot.send_message(chat_id=id, text=alert)


if __name__ == '__main__':
    asyncio.run(funz_alert(True))
