#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <SPI.h>
#include <SD.h>

Adafruit_INA219 ina219;

File currentFile;


#define CURRENT_CORRECTION 0.80 // Fattore di correzione della corrente misurata
#define SD_CS_PIN 10  // Pin CS per il modulo SD, potrebbe variare in base al modulo usato


// Funzione per stampare il tempo trascorso
void printTime(long time)
{
  uint32_t secs = time / 1000;
  uint32_t ms = time - (secs * 1000);

  Serial.print(F("TaskTime: "));
  Serial.print(secs);
  Serial.print(".");
  if (ms < 100)
    Serial.print(F("0"));
  if (ms < 10)
    Serial.print(F("0"));
  Serial.print(ms);
  Serial.println(F("s"));
}

// Funzione per stampare la corrente media e totale
void printCurrent(float current, long time)
{
  Serial.print(F("Average Current: "));
  Serial.print(current);
  Serial.println(F("mA"));
  Serial.print(F("Total Current: "));
  Serial.print((current / ((float)time * 1000)) * 60);
  Serial.println(F("mA/min"));

  // Salva la corrente su file
  currentFile = SD.open("corrente.txt", FILE_WRITE);
  if (currentFile)
  {
    currentFile.print(current);
    currentFile.println("mA");
    currentFile.close();
  }
}

void setup(void)
{
  Serial.begin(9600); // Avvia la comunicazione seriale

  if (!ina219.begin())
  { // Controlla se il sensore INA219 è connesso
    Serial.println("Errore: sensore INA219 non trovato!");
    while (1)
      ; // Rimani bloccato qui se il sensore non è trovato
  }
  Serial.println("Sensore INA219 pronto.");

  // Inizializza la scheda SD
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("Errore: MicroSD non trovata o impossibile da inizializzare.");
    while (1);  // Blocca l'esecuzione se la microSD non è rilevata
  }
  Serial.println("MicroSD inizializzata correttamente.");
}

void loop(void)
{
  if (digitalRead(7) == HIGH)
  { // Se il pin è acceso
    int i = 0;
    float current = 0;
    long time = millis(); // Inizia il conteggio del tempo
    while (1)
    {
      i++;
      current += ina219.getCurrent_mA() - CURRENT_CORRECTION; // Accumula la corrente misurata
      if (digitalRead(7) == LOW)
      {                         // Se riceve 'LOW', termina la misurazione
        time = millis() - time; // Calcola il tempo trascorso
        current = current / i;  // Calcola la corrente media

        printTime(time);             // Stampa il tempo impiegato
        printCurrent(current, time); // Stampa la corrente media e totale
        break;
      }
    }
  }
}
