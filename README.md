# Arduino Environmental Monitor

Sistema di monitoraggio ambientale basato su **Arduino Mega 2560**, che utilizza il sensore **BME280** per la rilevazione di temperatura, pressione e umidità, e un display **OLED 128×64 basato su controller SH1106** per la visualizzazione dei dati.  
La comunicazione tra i dispositivi avviene tramite **I²C**, mentre la connessione con il PC per la configurazione e il logging avviene tramite **UART**.

---

## ⚙️ Utilizzo del progetto

Il progetto è composto da due componenti principali:

- **Firmware Arduino**: gestisce la comunicazione I²C, il sensore BME280, il display SH1106 e i pulsanti di controllo.  
- **Client PC**: permette la configurazione dei parametri e la visualizzazione dei dati tramite terminale.

### Funzionamento

All’avvio, il firmware:
1. Inizializza le periferiche UART e I²C.  
2. Chiede all’utente, tramite terminale, di configurare:
   - Frequenza di campionamento (125 ms – 1000 ms)  
   - Unità di misura della temperatura (°C, K, °F)  
   - Unità di misura della pressione (hPa, bar)  
   - Abilitazione del log sul terminale  
3. Mostra un menu interattivo sul display OLED, navigabile tramite due pulsanti collegati ai pin:
   - PD2 → SELECT (scorre tra le voci)  
   - PD3 → CONFIRM (conferma la selezione)  
4. Visualizza i valori letti dal sensore sul display OLED e, se abilitato, anche sul terminale seriale.  
5. Per uscire, selezionare "Exit" dal menu.

---

## 🧰 Comandi principali

### 🔨 Compilazione e caricamento del firmware

Dalla directory principale del progetto eseguire:

```bash
make
```

Questo comando:
- Compila il firmware Arduino   
- Carica automaticamente il firmware sulla scheda **Arduino Mega 2560**  
- Compila anche il client per PC   

Nota: il caricamento su Arduino avviene tramite `avrdude` con le impostazioni già incluse nel progetto.

---

### 💻 Avvio del client seriale

Avviare il client interattivo con:

```bash
./client/client /dev/ttyACM0 19200
```

Dove:
- `/dev/ttyACM0` è la porta seriale esposta da Arduino  
- `19200` è il baud rate utilizzato dalla UART del firmware  

Il client:
- mostra i messaggi inviati da Arduino  
- accetta input da tastiera  
- inoltra i comandi/configurazioni al firmware  

Per terminare il client dal terminale, premere **Ctrl + C**.

---

### 🧹 Pulizia dei file generati

Per rimuovere i file temporanei di compilazione (file `.o`, eseguibili generati, `.hex`, ecc.):

```bash
make clean
```

Questo comando esegue la pulizia sia per il firmware (`src/`) sia per il client (`client/`).

---

## 🔚 Terminazione

- Per uscire dal menu sul dispositivo: selezionare **"Exit"** usando i pulsanti fisici.  
- Per chiudere la sessione seriale lato PC: premere **Ctrl + C** nel terminale dove è in esecuzione il client.

---




