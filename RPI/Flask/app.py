from flask import Flask, render_template
import mariadb
import sys

#setup app
app = Flask(__name__)

# Definisce una rotta principale
@app.route('/')
def index():
    try:
        # Connessione al database MariaDB
        conn = mariadb.connect(
            host="127.0.0.1",
            port=3306,
            user="root",
            password="luca",
            autocommit=True,
            database="sod_db"
        )
    except mariadb.Error as e:
        print(f"Errore connessione al database: {e}")
        sys.exit(1)

    # Ottiene un cursore per eseguire query SQL
    cur = conn.cursor()
    
    # Esegue una query SQL per selezionare tutte le righe dalla tabella 'misure'
    cur.execute("SELECT * FROM misure ORDER BY id DESC")
    
    # Ottiene tutti i risultati della query
    data = cur.fetchall()
    
    # Chiude la connessione al database
    conn.close()

    # Passa i dati alla pagina HTML usando un template Flask
    return render_template('index.html', data=data)



# Punto di ingresso dell'app Flask
if __name__ == '__main__':
    # Avvia l'app in modalità debug
    app.run(host='0.0.0.0', port=5000, debug=True)
