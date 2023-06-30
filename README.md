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
- ESP32
  -  src-->main.cpp: è il corpo principale del nostro progetto in cui andiamo a definire tutte le funzioni necessarie per l'acquisizione dei dati;
  -  platform.ini: viene definito il file di configurazione di progetto che specifica la piattaforma, il dispositivo, framework e librerie 
- RaspBerry: in questa directory troviamo i file relativi a:
  - ACCOUNTs: file relativo agli account che sono stati utilizzati
  


## Schema di funzionamento
![Schema di funzionamento](https://github.com/LucreziAntenucci98/Applicazione-IoT/blob/master/Schema%20rissuntivo.png)

## Installazioni necessarie per l'avvio 
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
   protocol webspckets
   socket_domain ipv4
   allow_anonymous true
   ```
   
 - Riavviare e abilitare l'avvio di mosquitto al boot del sistema operativo
   ```shell
   sudo systemctl restart mosquitto
   sudo sstemctl enable mosquitto
   ```
   
 - Per verificare lo stato del servizio
   ```shell
   sudo systemctl status mosquitto
   ```

 ### Salvataggio Dati
 il salvataggio dei dati viene fatto tramite MariaDB:
 
 - Installare MariaDB
   ```shell
   sudo apt install mariadb-server
   sudo systemctl start mariadb.service
   mysql -u root
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
   mkdir /etc/mqttwarn
   cd /etc/mqttwarn
   sudo nano mqttwarn.ini
   ```
   Dovranno essere specificati tutti quanti i servizi da lanciare, i topic da seguire e in particolare nel nostro caso il servizio mysql
   
 - Installare PHP
   ```shell
   sudo apt install php
   ```
   
 - Installare PhpMyAdmin
 - Creazione di un utente MySQL
 - Accedere da browser all'indirizzo http://localhost/phpmyadmin/ con le credenziali dell'utente appena creato

