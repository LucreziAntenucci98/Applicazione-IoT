# Progetto TeamBerry
Progetto Magistrale di Sistemi Operativi Dedicati 2022-2023

## Autori

- Antenucci Lucrecia
- Di Biase Alessandro
- Moretti Laura
- Procaccioli Valerio

## Descrizione del Progetto

L'obiettivo del seguente progetto è stato quello di implementare un sistema IoT in grado di acquisire dei dati tramite sensori (Luminosità, Pressione, Temperatura e Altitudine), salvarli in un DBMS e infine creare un'interfaccia web per visualizzarli sia in tempo reale sia quelli che si trovano nel DB.

Il sistema è composto dai seguenti dispositivi:
- Raspberry Pi 4
- ESP32
- sensore temperatura, umidità e pressione BMP280
- sensore di luminosità BH1750
- modulo RTC

La scheda ESP 32 acquisisce le misure dei vari sensori e le comunica tramite protocollo MQTT alla Raspeberry che ha il ruolo di broker MQTT. Il coordinamento dei vari task che l'ESP32 dovrà svolgere è affidato a FreeRTOS.

## Organizzazione Repository
La repository è composta da 2 directory:
- **ESP32**
  -  *src/main.cpp:* è il corpo principale del nostro progetto in cui andiamo a definire tutte le funzioni necessarie per l'acquisizione dei dati;
  -  *platform.ini:* viene definito il file di configurazione di progetto che specifica la piattaforma, il dispositivo, framework e librerie 
- **RaspBerry:** in questa directory troviamo i file relativi a:
  - *ACCOUNTs:* file relativo agli account che sono stati utilizzati
- **File Utili**: in cui si possono trovare tutti i file di configurazione per l'avvio dell'applicazione.
  


## Schema di funzionamento
![Schema di funzionamento](https://github.com/LucreziAntenucci98/Applicazione-IoT/blob/master/Schema%20rissuntivo.png)

## Azioni da svolgere su Raspberry 

- ```shell
    sudo apt update
    ```
 - Installazione Mosquitto
   ```shell
    sudo apt install mosquitto mosquitto-clients -y
    ```
   
 - Modificare il file dì configurazione
   ```shell
   sudo nano etc/mosquitto/mosquitto.conf
   ```
   
 - Copiare il contenuto e incollarlo nell'editor appena aperto. Chiudere e salvare il file attraverso i   tasti CTRL+S e CTRL+X
   
 - Caricare il file di configurazione appena modificato
   ```shell
   listener 1883
   listener 9001
   protocol websockets
   socket_domain ipv4
   allow_anonymous true
   ```
   
 - Riavviare e abilitare l'avvio di mosquitto al boot del sistema operativo
   ```shell
   sudo systemctl restart mosquitto
   sudo systemctl enable mosquitto
   ```
   
 - Per verificare lo stato del servizio
   ```shell
   sudo systemctl status mosquitto
   ```

 ### Gestione Visualizzazione Database
 il salvataggio dei dati viene fatto tramite MariaDB:
 
 - Installare MariaDB
   ```shell
   sudo apt install mariadb-server
   sudo systemctl start mariadb.service
   sudo mysql
   ```
   
   Una volta entrato come root dobbiamo creare un nuovo utente 
   ```shell
   CREATE USER 'berryuser'@'localhost' IDENTIFIED by '98765432';
   GRANT ALL PRIVILEGES ON *.* TO 'berryuser'@'localhost' WITH GRANT OPTION;
   FLUSH PRIVILEGES;
   ```
   
   Si esce dal servizio mysql e si riesegue il riavvio del servizio
   ```shell
   sudo systemctl restart mariadb.service
   ```
   
 - Installare Apache2
   ```shell
   sudo apt-get install apache2
   sudo apt-get install libapache2-mod-php
   ```
   
 - Verificare lo stato dei servizi
   ```shell
   sudo systemctl status mariadb
   sudo systemctl status apache2
   ```
   
 - Installare python3 e pip per la corretta installazione del servizio mqttwarn
   ```shell
   sudo apt install python3
   sudo apt install python3-pip
   ```
  
 - Installare mqttwarn
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
   A questo punto verificare il corretto funzionamento attraverso il comando
    ```shell
    mqttwarn --config-file /etc/mqttwarn/mqttwarn.ini
    ```
   Se sono già nel path /etc/mqttwarn non è necessario specificare il file di configurazione, basta digitare il comando
    ```shell
    mqttwarn
    ```
 - Per immagazzinare i dati internamente a MySQL tramite mqttwarn, bisogna assicurarsi di avere correttamente configurato il Database in MySQL.
   Per eseguire questa configurazione si è deciso di sfruttare il servizio PhpMyAdmin, che mostrando il servizio su interfaccia web, facilita le operazioni.
   
 - Installare PHP
   ```shell
   sudo apt install php
   sudo apt install phpmyadmin
   ```
   Appare un prompt dove dobbiamo selezionare la voce apache2 con la barra spaziatrice, successivamente confermare la scelta con il tasto   invio;
   Selezionare Sì alla richiesta di utilizzare dbconfig-common per fare il setup del database;
   Può essere richiesta la password per accedere a MySQL da phpmyadmin.
   ```shell
   sudo phpenmod mbstring
   sudo systemctl restart apache2
   ```
   > N.B. Se si presenta un'errore di configurazione durante l'installazione del database (ERROR:1819) selezionare la voce abort e 
   > proseguire con i seguenti passaggi:
   > ```shell
    > mysql -u root
    > mysql> UNISTALL COMPONENT "file://component_validate_password";
    > mysql> exit
    > sudo apt install phpmyadmin 
    >```
   > Terminata l'installazione di PhpMyAdmin poi riaccedere a MySQL e passare il seguente comando:
    > ```shell
    > mysql -u root
    > mysql> INSTALL COMPONENT "file://component_validate_password";
    > mysql> exit
    > ```
    
  - Creazione del Database da interfaccia PhpMyAdmin
    1) Da interfaccia web accedere al servizio PhpMyAdmin con l'indirizzo http://xxx.xxx.xx.xxx/phpmyadmin/;
    2) Eseguire il login con le credenziali sopra create (utente: berryuser; password: 98765432);
    3) Sulla pagina inziale creare un nuovo Database chiamato "SensorData", con codifica: utf8mb4_general_ci;
    4) Automaticamente si apre una pagina "Struttura", in cui dobbiamo inserire il nome della tabella da creare e il numero dei campi, nel nostro caso "LetturaSensori" e 3 campi (topic(VARCHAR)(12), value(FLOAT)(6) e timestamp(TIMESTAMP)(0));
  
### Gestione Visualizzazione Real-Time
- Installazione Flask
  ```shell
    pip install Flask Flask-SocketIO
    nano app.py
    mkdir templates
    nano templates/index.html
    python app.py
    ```
### Rendere i servizi di visualizzazione Demoni
- Demone per la visualizzazione Database
  ```shell
    path: /usr/lib/system/systemd/mqttwarn.service
    systemctl enable mqttwarn
    sudo system start
    systemctl status mqttwarn
    ```
- Demone per la visualizzazione Real-Time
  In **/home/RaspBerry** creiamo un file
  ```shell
    nano launch WebInterface.sh
    chmod +x launch WebInterface.sh
    ```
  Ed inseriamo questo codice:
  ```shell
    #!/bin/bash
    sleep 10
    python3 /home/RaspBerry/app.py
    ```
  Posizionarsi nel path e modificare o creare il file webinterface.service:
  ```shell
    /usr/lib/system/systemd/mqttwarn.service
    sudo nano webinterface.service
    ```
  All'interno di questo file dobbiamo inserire il codice che si può trovare nella directory denominato webinterface.service.
  ```shell
    systemctl enable webinterface
    sudo system start webinterface
    systemctl status webinterface
    ```
### Gestione Unica Visualizzazione Web
Per facilitare l'utente nel raggiungere le due diverse tipologie di visualizzazione dati, si è realizzata un'interfaccia all'indirizzo http://xxx.xxx.xx.xxx dai quali attraverso i bottoni ci si può reindirizzare alla pagina desiderata.
  ```shell
    cd /var/www/html
    sudo nano index.html   
  ```
Sostituire tutto quello che si trova nel file index.html con il codice riportato nella directory principale, inoltre si dovranno aggiungere le immagini.
Le immagini per il funzionamento Real-Time vanno inserite nella cartella ```shell /home/Raspberry/static ```.
Le immagini per il funzionamento normale della Home, va messa nello stesso path dell'index.html.


