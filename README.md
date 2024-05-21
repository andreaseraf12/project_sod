# Implementazione di un sistema di acquisizione dati per il monitoraggio ambientale

Progetto del corso di laurea magistrale di Sistemi Operativi Dedicati 2022-2023


## Descrizione del Progetto

## Organizzazione Repository 

## Installazione locale e utilizzo 

## Autori

|Nome autore 
|-----
|'Ciuffreda Silvia'
|'Liberatore Luca'
|'Serafini Andrea'


Nel contesto del corso "Sistemi Operativi Dedicati", il presente progetto si propone di realizzare un sistema completo per il monitoraggio, la visualizzazione e il controllo dei parametri ambientali. Utilizzando una combinazione di hardware e software, il sistema è progettato per acquisire, elaborare e visualizzare dati ottenuti tramite molteplici sensori. In particolare, il sistema monitora costantemente i parametri ambientali tramite il sensore BMP280; quando la condizione termica supera una soglia predefinita, la ventola viene attivata per raffreddare il sistema e mantenere la temperatura entro il range desiderato. La velocità della ventola è proporzionale allo scostamento della temperatura attuale rispetto a quella desiderata. Le informazioni acquisite di temperatura, pressione e velocità della ventola vengono visualizzate sul display OLED e inviate alla Raspberry Pi insieme al timestamp corrispondente. La Raspberry Pi memorizza i dati all'interno di un database e genera notifiche di alert qualora la temperatura superi una ulteriore soglia critica. Gli alert vengono inviati tramite il bot Telegram "SensorDataRPI_bot" a tutti gli utenti registrati. Inoltre, la Raspberry Pi mette a disposizione i dati memorizzati nel database attraverso una semplice pagina web.


Per ulteriore dettagli riferirsi al file pdf contenuto in "/project_SOD/gruppo4_consegnaSOD".


