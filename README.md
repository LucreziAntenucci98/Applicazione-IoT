# Progetto TeamBerry
Progetto del corso di laurea magistrale di Sistemi Operativi Dedicati 2022-2023

## Autori
- Antenucci Lucrecia
- Di Biase Alessandro
- Moretti Laura
- Procaccioli Valerio

## Descrizione del Progetto
L'obiettivo del seguente progetto è implementare un sistema IoT in grado di acquisire i dati realtivi alla luminosità, temperatura, pressione e altitudine tramite sensori, salvarli in un DBMS e creare un'interfaccia web per visualizzare sia i dati in tempo reale sia i dati salvati nel DB.  
Il sistema è composto dai seguenti dispositivi:
- Raspberry Pi 4;
- ESP32;
- sensore temperatura e pressione BMP280;
- sensore di luminosità BH1750;
- modulo RTC.

I sensori BMP280, BH1750, e il modulo RTC sono connessi alla scheda ESP grazie ad una breadboard e i jumper. In particolare i due sensori sono connessi attraverso i connettori SparkFun Qwiic. La scheda ESP 32 acquisisce le misure dei vari sensori e le comunica tramite protocollo MQTT alla Raspeberry che ha il ruolo di broker MQTT. Il coordinamento dei vari task che l'ESP32 svolge è affidato a FreeRTOS.

![Schema di funzionamento](https://github.com/LucreziAntenucci98/Applicazione-IoT/blob/master/Schema%20rissuntivo.png)

## Organizzazione Repository
La repository è composta da 2 directory:
- **ESP32**
  -  *src/main.cpp:*  
  è il corpo principale del progetto, in cui sono definite tutte le funzioni necessarie per l'acquisizione dei dati;
  -  *platform.ini:*  
  è il file di configurazione di progetto per ESP32 che specifica la piattaforma, il dispositivo, framework e librerie utilizzate. 
- **RaspBerry:** in questa directory troviamo i file relativi a:
  - *ACCOUNTs:*  
  è il file relativo agli account che sono stati utilizzati, specificando le credenziale per accedere ai servizi nel presente progetto;
  - *app.py:*  
  è il file che alla ricezione di messaggi attraverso il protocollo MQTT, inoltra il contenuto informativo tramite WSGI all'interfaccia grafica specifica per la visualizzazione in real-time dei dati;
  - *index.html_home*  
  è il file che contiene la descrizione dell'interfaccia web per introdurre l'utente alla visualizzazione dei dati in real-time o salvati in database. Il file è in realtà denominato *index.html*, ma presenta il suffisso *_home* per distinguirlo dal file relativo all'interfaccia web della visualizzazione real-time;  
  - *index.html_real-time*  
  è il file che contiene la descrizione dell'interfaccia web per visualizzare i dati in real-time. Il file è in realtà denominato *index.html*, ma presenta il suffisso *_real-time* per distinguirlo dal file relativo all'interfaccia web introduttiva per l'utente;
  - *launchWebInterface.sh*  
  è il file che lancia l'esecuzione dello script python *app.py*;
  - *mqttwarn.ini*  
  è il file di configurazione del servizio mqttwarn. Contiene le credenziali per accedere correttamente alla connessione MQTT, i serzivi che devono essere avviati, le loro impostazioni, e le azioni che devono essere svolte alla ricezione di un messaggio per ogni topic;
  - *mqttwarn.service*  
  è il file per gestire l'avvio del servizio mqttwarn all'avvio della Raspberry;
  - *webinterface.service*  
  è il file per gestire l'esecuzione di *launchWebInterface.sh* all'avvio della Raspberry;

## Realizzazione progetto
La realizzazione del progetto è suddivisa in due macro-sezioni. La prima riguarda la ESP32, la seconda la Raspberry Pi 4. Sono di seguito presenti le sezioni relative ad entrambe le macro-sezioni.

### Azioni da svolgere su ESP32
Il primo step riguarda la connessione dei diversi dispositivi sulla breadboard.  
<<SCHEMA>>  
Con l'hardware pronto, si può passare alla realizzazione di un nuovo progetto all'interno di Visual Studio Code. Una volta realizzato il progetto, devono essere assegnate le specifiche caratteristiche per il seguente progetto. Queste sono contenute all'interno del file *platformio.ini*. Devono essere incluse tutte le librerie necessarie per il corretto funzionamento del codice, presente in *src/main.cpp*. In particolare, le librerie richieste sono:
- Arduino.h  
libreria di default per il dispositivo ESP32;
- freertos/FreeRTOS.h  
libreria per introdurre e gestire in modo personalizzato i differenti task;
- PubSubClient.h  
libreria per utilizzare il protocollo MQTT;
- WiFi.h  
libreria per connettersi ad un segnale WiFi;
- Wire.h  
libreria per permettere una corretta connessione con i sensori e modulo RTC;
- Adafruit_Sensor.h  
libreria per un corretto funzionamento dei sensori dell'azienda Adafruit;
- Adafruit_BMP280.h  
libreria per il sensore BMP280 dell'azienda Adafruit;
- BH1750.h  
libreria per il sensore BH1750 dell'azienda Adafruit;
- RTClib.h  
libreria per il modulo RTC dell'azienda Adafruit;

Con il sistema correttamente configurato si può dunque realizzare il codice per soddisfare l'obiettivo prestabilito, come mostrato nel file *src/main.cpp*. Risulta quindi sufficiente eseguire l'upload sul dispositivo, connesso al computer e senza alcuna applicazione in ascolto sulla seriale specificata.

### Azioni da svolgere su Raspberry
Gli step da seguire sono numerosi, e toccano diversi ambiti. Sono dunque presentate le diverse azioni raggrupate in sottosezioni. Prima di eseguire qualsiasi operazione si consiglia di aggiornare il pacchetto apt:
```shell
sudo apt update
```
#### GESTIRE LA COMUNICAZIONE MQTT
Installare Mosquitto:
```shell
sudo apt install mosquitto mosquitto-clients -y
```
Aprire il file dì configurazione:
```shell
sudo nano etc/mosquitto/mosquitto.conf
```
Copiare all'interno il contenuto di *mosquitto.conf* nella cartella "Raspberry Pi 4", per semplicità qui riportato.
```shell
listener 1883
listener 9001
protocol websockets
socket_domain ipv4
allow_anonymous true
```
Chiudere e salvare il file attraverso i tasti CTRL+S e CTRL+X.  
Riavviare e abilitare l'avvio di mosquitto al boot del sistema operativo:
```shell
sudo systemctl restart mosquitto
sudo systemctl enable mosquitto
```
Per verificare lo stato del servizio:
```shell
sudo systemctl status mosquitto
```
#### SALVARE I DATI IN DATABASE
Installare MariaDB per il salvataggio dei dati:
```shell
sudo apt install mariadb-server
sudo systemctl start mariadb.service
sudo mysql
```
Entrati come root bisogna creare un nuovo utente con i privilegi necessari: 
```shell
CREATE USER 'berryuser'@'localhost' IDENTIFIED by '98765432';
GRANT ALL PRIVILEGES ON *.* TO 'berryuser'@'localhost' WITH GRANT OPTION;
FLUSH PRIVILEGES;
```
Uscire dal servizio mysql ed eseguire il riavvio del servizio:
```shell
sudo systemctl restart mariadb.service
```
Installare Apache2:
```shell
sudo apt-get install apache2
sudo apt-get install libapache2-mod-php
```
Verificare lo stato dei servizi:
```shell
sudo systemctl status mariadb
sudo systemctl status apache2
```
Installare python3 e pip per la corretta installazione del servizio mqttwarn:
```shell
sudo apt install python3
sudo apt install python3-pip
```
Installare mqttwarn:
```shell
sudo pip3 install mqttwarn
sudo mkdir /etc/mqttwarn
cd /etc/mqttwarn
sudo nano mqttwarn.ini
```
Dovendo utilizzare il servizio mySQL attraverso i moduli python di mqttwarn devono essere aggiunte ulteriori dipendenze:
```shell
pip3 install mysqlclient
sudo apt install python3-dev default-libmysqlclient-dev build-essential
sudo pip3 install mysql-connector-python
```
Verificare il corretto funzionamento:
```shell
mqttwarn --config-file /etc/mqttwarn/mqttwarn.ini
```
Se si è già nel medesimo path in cui è definito *mqttwarn.ini* (/etc/mqttwarn), non è necessario specificare il file di configurazione, ma è sufficiente digitare il comando:
```shell
mqttwarn
```
Per immagazzinare i dati internamente a MySQL tramite mqttwarn, bisogna assicurarsi di avere correttamente configurato il Database in MySQL. Tra le diverse modalità possibili, è qui mostrata la creazione del database attraverso il servizio PhpMyAdmin, successivamente utilizzato per la visione del database attraverso interfaccia web.  
Installare PHP:
```shell
sudo apt install php
sudo apt install phpmyadmin
```
Automaticamente viene mostrato un prompt, dove si deve selezionare la voce apache2 con la barra spaziatrice, successivamente confermare la scelta con il tasto invio. Alla richiesta di utilizzare dbconfig-common per eseguire il setup del database selezionare 'yes'. È infine richiesta la password per accedere a MySQL da phpmyadmin. Terminare quindi la configurazione:
```shell
sudo phpenmod mbstring
sudo systemctl restart apache2
```
  > N.B. Se si presenta un'errore di configurazione durante l'installazione del database (ERROR:1819) selezionare la voce abort e 
  > proseguire con i seguenti passaggi:
  > ```shell
  > sudo mysql
  > mysql> UNISTALL COMPONENT "file://component_validate_password";
  > mysql> exit
  > sudo apt install phpmyadmin 
  >```
  > Terminata l'installazione di PhpMyAdmin poi riaccedere a MySQL e passare il seguente comando:
  > ```shell
  > sudo mysql
  > mysql> INSTALL COMPONENT "file://component_validate_password";
  > mysql> exit
  > ```
Creare il Database da interfaccia PhpMyAdmin:
  1) Da interfaccia web accedere al servizio PhpMyAdmin con l'indirizzo http://xxx.xxx.xx.xxx/phpmyadmin/ (utilizzare l'ip della Raspberry Pi 4);
  2) Eseguire il login con le credenziali sopra create (utente: berryuser; password: 98765432);
  3) Sulla pagina inziale creare un nuovo database chiamato "SensorData", con codifica: utf8mb4_general_ci;
  4) In modo utomatico si apre una pagina "Struttura", in cui si deve inserire il nome della tabella da creare e il numero dei campi, per questo progetto il nome è "LetturaSensori" e i campi sono 3 (nome: topic, tipo: VARCHAR, lunghezza: 12; nome: value, tipo: FLOAT, lunghezza: 6; nome: timestamp, tipo: TIMESTAMP, lunghezza: 0);
  
#### MOSTRARE I DATI IN REAL-TIME
Installare Flask:
```shell
pip install Flask Flask-SocketIO
```
Creare i file *app.py* e *templates/index.html*, inserendo i codici contenuti rispettivamente in *app.py* e *index.html_real-time* nella cartella "Raspberry Pi 4":
```shell
nano app.py
mkdir templates
nano templates/index.html
```
Verificare il corretto funzionamento mandando in esecuzione lo script python e visualizzando sul web l'indirizzo http://xxx.xxx.xx.xxx:5000 (da sostituire con l'indirizzo ip della Raspberry Pi 4):
```shell
python app.py
```
#### RENDERE I SERVIZI DI VISUALIZZAZIONE DATI DEMONI
- Demone per la visualizzazione Database
  ```shell
  path: /usr/lib/system/systemd/mqttwarn.service
  systemctl enable mqttwarn
  sudo system start
  systemctl status mqttwarn
  ```
- Demone per la visualizzazione Real-Time
  In **/home/RaspBerry** creare un file:
  ```shell
  nano launchWebInterface.sh
  chmod +x launchWebInterface.sh
  ```
  Nel quale inserire il codice contenuto in *launchWebInterface.sh* nella cartella "Raspberry Pi 4", per semplicità qui riportato:
  ```shell
  #!/bin/bash
  sleep 10
  python3 /home/RaspBerry/app.py
  ```
  Posizionarsi nel path e creare il file webinterface.service:
  ```shell
  cd /usr/lib/system/systemd
  sudo nano webinterface.service
  ```
  Inserire nel file il codice che contenuto in *webinterface.service* nella cartella "Raspberry Pi 4". Abilitare dunque il servizio all'avvio del sistema, lanciare il servizio e verificare lo stato:
  ```shell
  systemctl enable webinterface
  sudo system start webinterface
  systemctl status webinterface
  ```
#### CREARE PAGINA WEB DI REINDIRIZZAMETO
Per facilitare l'utente nel raggiungere le due diverse tipologie di visualizzazione dati, realizzare un'interfaccia all'indirizzo http://xxx.xxx.xx.xxx (da sostituire con l'indirizzo ip della Raspberry Pi 4), che presenti due bottoni dai quali si reindirizza l'utente alla pagina desiderata. Modificare il file creato all'installazione di apache2:
```shell
cd /var/www/html
sudo nano index.html   
```
Nel file inserire il codice contenuto in *index.html_home* nella cartella "Raspberry Pi 4", ed inoltre aggiungere le immagini.
Le immagini per il funzionamento della pagina web per la visualizzazione Real-Time vanno inserite nella cartella */home/Raspberry/static*.
L'immagine per il funzionamento normale della pagina di reindirizzamento, va messa nello stesso path del file *index.html*.


