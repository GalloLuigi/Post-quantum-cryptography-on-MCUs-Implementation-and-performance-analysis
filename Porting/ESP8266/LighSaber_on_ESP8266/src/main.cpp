#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266TrueRandom.h>

#include "Saber/api.h"
#include "Saber/poly.h"         /* Include definizioni per lavorare con polinomi usati nell'algoritmo. */
#include "Saber/rng.h"          /* Include definizioni per il generatore di numeri casuali usato da SABER. */
#include "Saber/SABER_indcpa.h" /* Include definizioni per lo schema crittografico a chiave pubblica IND-CPA di SABER. */
#include "Saber/verify.h"       /* Include funzioni per la verifica delle operazioni crittografiche. */

#define WORD_NUMBER 1

// Variabile per scegliere se calcolare il tempo o i cicli di clock
bool calcolaTempo = false; // Imposta a 'true' per calcolare il tempo, 'false' per cicli di clock

 uint8_t parole_in_array[WORD_NUMBER][11] = {
    { 8, 98, 110, 118, 122, 117, 108, 120, 98, 113, 115 }, // bnvzulxbqs
    //...
};

void setup()
{
    Serial.begin(115200); // Initialize serial communication (115200 baud standard for ESP8266)

    pinMode(12, OUTPUT);       // Imposta GPIO 2 come uscita


    uint8_t pk[SABER_PUBLICKEYBYTES];
    uint8_t sk[SABER_SECRETKEYBYTES];
    uint8_t c[SABER_BYTES_CCA_DEC];
    uint8_t k_a[SABER_KEYBYTES], k_b[SABER_KEYBYTES];

    uint64_t i, j;

    // Variables for measuring cpu cycles
    uint32_t CLOCK1, CLOCK2;
    uint32_t CLOCK_kp, CLOCK_enc, CLOCK_dec;

    // Variables for measuring time (microseconds)
    unsigned long startTime, endTime;
    unsigned long elapsedTime_kp, elapsedTime_enc, elapsedTime_dec;

    unsigned char entropy_input[48];

    ESP.wdtDisable(); // Disabilita il watchdog

    // Random Generator
    randomSeed(analogRead(A0));

    for (i = 0; i < 48; i++)
    {
        entropy_input[i] = ESP8266TrueRandom.random(0, 256); // random number 0/255
    }

    randombytes_init(entropy_input, NULL, 256);

    ESP.wdtEnable(5000); // Riabilita il watchdog dopo 5 secondi


  digitalWrite(12, HIGH);    // Accendi il pin GPIO 12

  for (int i = 0; i < WORD_NUMBER; i++)
    {
        // Imposta c all'i-esima parola nella matrice
        if (i < WORD_NUMBER) { 
            memcpy(c, parole_in_array[i], sizeof(parole_in_array[i]));
        }

    // Measure key pair generation
    if (calcolaTempo)
    {
        startTime = micros();
        crypto_kem_keypair(pk, sk);
        endTime = micros();
        elapsedTime_kp = endTime - startTime;
    }
    else
    {
        CLOCK1 = ESP.getCycleCount();
        crypto_kem_keypair(pk, sk);
        CLOCK2 = ESP.getCycleCount();
        CLOCK_kp = CLOCK2 - CLOCK1;
    }

    delay(0);

    // Measure encryption
    if (calcolaTempo)
    {
        startTime = micros();
        crypto_kem_enc(c, k_a, pk);
        endTime = micros();
        elapsedTime_enc = endTime - startTime;
    }
    else
    {
        CLOCK1 = ESP.getCycleCount();
        crypto_kem_enc(c, k_a, pk);
        CLOCK2 = ESP.getCycleCount();
        CLOCK_enc = CLOCK2 - CLOCK1;
    }

    // Measure decryption
    if (calcolaTempo)
    {
        startTime = micros();
        crypto_kem_dec(k_b, c, sk);
        endTime = micros();
        elapsedTime_dec = endTime - startTime;
    }
    else
    {
        CLOCK1 = ESP.getCycleCount();
        crypto_kem_dec(k_b, c, sk);
        CLOCK2 = ESP.getCycleCount();
        CLOCK_dec = CLOCK2 - CLOCK1;
    }

  digitalWrite(12, LOW);    // Accendi il pin GPIO 12

    }
    // Functional verification: check if k_a == k_b?
    //
    for (j = 0; j < SABER_KEYBYTES; j++)
    {
        if (k_a[j] != k_b[j])
        {
            Serial.println(F("----- ERR CCA KEM ------"));
            break;
        }
    }
    //

    ESP.wdtEnable(5000); // Riabilita il watchdog dopo 5 secondi

    delay(0);

    // Result
    Serial.println(F(""));
    if (calcolaTempo)
    {
        Serial.print(F("Time key_pair:"));
        Serial.println(elapsedTime_kp);
        Serial.print(F("Time enc:"));
        Serial.println(elapsedTime_enc);
        Serial.print(F("Time dec:"));
        Serial.println(elapsedTime_dec);
    }
    else
    {
        Serial.print(F("Cycles key_pair:"));
        Serial.println(CLOCK_kp);
        Serial.print(F("Cycles enc:"));
        Serial.println(CLOCK_enc);
        Serial.print(F("Cycles dec:"));
        Serial.println(CLOCK_dec);
    }
    
    Serial.println(F("----- END KEM SABER ------"));
}

void loop()
{    
}
