#!/bin/bash

# Indirizzo IP base della rete
base_ip="192.168.43."

# Range di IP da verificare
start=1
end=255

# Nome utente SSH
username="RaspBerry"

# Comando SSH per la verifica
ssh_command="ssh -o ConnectTimeout=2 $username@"

# Iterazione attraverso gli indirizzi IP
for ((i=start; i<=end; i++))
do
  ip="$base_ip$i"
  echo -n "Provo l'indirizzo IP: $ip"
  
  # Comando SSH per verificare la connessione
  if $ssh_command$ip exit >/dev/null 2>&1; then
    echo " -> Trovato! Indirizzo IP valido: $ip"
    break
  else
    echo " -> Non valido."
  fi
done
