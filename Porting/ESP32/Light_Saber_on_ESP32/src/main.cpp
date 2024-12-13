#include <Arduino.h>
#include <esp_random.h>

#include "Saber/api.h"
#include "Saber/poly.h"
#include "Saber/rng.h"
#include "Saber/SABER_indcpa.h"
#include "Saber/verify.h"
#include "Saber/SABER_params.h"

#define WORD_NUMBER 1

uint8_t pk[SABER_PUBLICKEYBYTES];
uint8_t sk[SABER_SECRETKEYBYTES];
uint8_t c[SABER_BYTES_CCA_DEC];
uint8_t k_a[SABER_KEYBYTES];
uint8_t k_b[SABER_KEYBYTES];

TaskHandle_t taskHandleKeypair = NULL;
TaskHandle_t taskHandleEncrypt = NULL;
TaskHandle_t taskHandleDecrypt = NULL;

const uint8_t parole_in_array[WORD_NUMBER][12] = {
 { 8, 98, 110, 118, 122, 117, 108, 120, 98, 113, 115 }, // bnvzulxbqs
 //...
};

// Variabile per scegliere se calcolare tempo o cicli di clock
bool calcolaTempo = true; // Imposta a 'true' per calcolare il tempo, 'false' per i cicli di clock

// Variabili per i cicli di clock
uint32_t CLOCK1, CLOCK2;
uint32_t CLOCK_kp, CLOCK_enc, CLOCK_dec;

// Variabili per il tempo (microsecondi)
unsigned long startTime;
unsigned long endTime;
unsigned long elapsedTime_kp, elapsedTime_enc, elapsedTime_dec;

// Generatore casuale
unsigned char entropy_input[48];

// Dichiarazione delle code
QueueHandle_t keypairQueue;
QueueHandle_t encryptionQueue;
QueueHandle_t decryptionQueue;

// Funzione del task per la generazione delle chiavi
void taskKeypair(void *pvParameters)
{
    crypto_kem_keypair(pk, sk);
    xQueueSend(keypairQueue, pk, portMAX_DELAY);
    xQueueSend(keypairQueue, sk, portMAX_DELAY);
    vTaskDelete(NULL);
}

// Funzione del task per la cifratura
void taskEncrypt(void *pvParameters)
{
    if (xQueueReceive(keypairQueue, pk, portMAX_DELAY) == pdTRUE)
    {
        crypto_kem_enc(c, k_a, pk);
        xQueueSend(encryptionQueue, c, portMAX_DELAY);
        xQueueSend(encryptionQueue, k_a, portMAX_DELAY);
    }
    vTaskDelete(NULL); // cancello il task chiamante
}

// Funzione del task per la decifratura
void taskDecrypt(void *pvParameters)
{
    if (xQueueReceive(encryptionQueue, c, portMAX_DELAY) == pdTRUE)
    {
        xQueueReceive(keypairQueue, sk, portMAX_DELAY);
        crypto_kem_dec(k_b, c, sk);
        xQueueSend(decryptionQueue, k_b, portMAX_DELAY);
    }
    vTaskDelete(NULL);
}

// Funzione del task per la verifica
void taskVerify(void *pvParameters)
{
    if (xQueueReceive(decryptionQueue, k_b, portMAX_DELAY) == pdTRUE)
    {
        // Verifica funzionale: controllo se k_a == k_b
        for (int j = 0; j < SABER_KEYBYTES; j++)
        {
            if (k_a[j] != k_b[j])
            {
                Serial.println("----- ERR CCA KEM ------");
                break;
            }
        }

        Serial.println("----- END KEM SABER ------");
    }
    vTaskDelete(NULL);
}

void setup()
{
    Serial.begin(115200);

    pinMode(12, OUTPUT); // Imposta il pin 13 come uscita

    // Creazione delle code
    keypairQueue = xQueueCreate(2, sizeof(uint8_t) * SABER_PUBLICKEYBYTES);
    encryptionQueue = xQueueCreate(2, sizeof(uint8_t) * SABER_BYTES_CCA_DEC);
    decryptionQueue = xQueueCreate(2, sizeof(uint8_t) * SABER_KEYBYTES);

    if (keypairQueue == NULL || encryptionQueue == NULL || decryptionQueue == NULL)
    {
        Serial.println("Failed to create queue");
        return;
    }

    for (int i = 0; i < 48; i++)
    {
        entropy_input[i] = (esp_random()) % 256;
    }
    randombytes_init(entropy_input, NULL, 256);

    Serial.println("");
    Serial.print("SABER_INDCPA_PUBLICKEYBYTES=");
    Serial.println(SABER_INDCPA_PUBLICKEYBYTES);
    Serial.print("SABER_INDCPA_SECRETKEYBYTES=");
    Serial.println(SABER_INDCPA_SECRETKEYBYTES);
    Serial.print("SABER_PUBLICKEYBYTES=");
    Serial.println(SABER_PUBLICKEYBYTES);
    Serial.print("SABER_SECRETKEYBYTES=");
    Serial.println(SABER_SECRETKEYBYTES);
    Serial.print("SABER_KEYBYTES=");
    Serial.println(SABER_KEYBYTES);
    Serial.print("SABER_HASHBYTES=");
    Serial.println(SABER_HASHBYTES);
    Serial.print("SABER_BYTES_CCA_DEC=");
    Serial.print(SABER_BYTES_CCA_DEC);
    Serial.println("");

  digitalWrite(12, HIGH); // Accendi il pin 12 (imposta HIGH)

     for (int i = 0; i < WORD_NUMBER; i++)
    {
        // Imposta c all'i-esima parola nella matrice
        if (i < WORD_NUMBER) { // Assicurati di non superare la dimensione dell'array
            memcpy(c, parole_in_array[i], sizeof(parole_in_array[i]));
        }

        startTime = micros();
        xTaskCreate(taskKeypair, "KeypairTask", 10000, NULL, 1, &taskHandleKeypair);
        endTime = micros();
        if (taskHandleKeypair != NULL){
            vTaskDelete(taskHandleKeypair);
            taskHandleKeypair = NULL;
        }
        elapsedTime_kp = elapsedTime_kp + (endTime - startTime);

        startTime = micros();
        xTaskCreate(taskEncrypt, "EncryptTask", 12000, NULL, 1, &taskHandleEncrypt);
        endTime = micros();
        if (taskHandleEncrypt != NULL){
            vTaskDelete(taskHandleEncrypt);
            taskHandleEncrypt = NULL;
        }
        elapsedTime_enc = elapsedTime_enc + (endTime - startTime);

        startTime = micros();
        xTaskCreate(taskDecrypt, "DecryptTask", 13000, NULL, 1, &taskHandleDecrypt);
        endTime = micros();
        if (taskHandleDecrypt != NULL){
            vTaskDelete(taskHandleDecrypt);
            taskHandleDecrypt = NULL;
        }
        elapsedTime_dec = elapsedTime_dec + (endTime - startTime);

    }

        digitalWrite(12, LOW);  // Spegni il pin 12 (imposta LOW)


    //Cicli di cpu
    for (int i = 0; i < WORD_NUMBER; i++)
    {
        // Imposta c all'i-esima parola nella matrice
        if (i < WORD_NUMBER) { 
            memcpy(c, parole_in_array[i], sizeof(parole_in_array[i]));
        }

        CLOCK1 = ESP.getCycleCount();
        xTaskCreate(taskKeypair, "KeypairTask", 10000, NULL, 1, &taskHandleKeypair);
        CLOCK2 = ESP.getCycleCount();
        if (taskHandleKeypair != NULL){
            vTaskDelete(taskHandleKeypair);
            taskHandleKeypair = NULL;
        }
        CLOCK_kp = CLOCK_kp + (CLOCK2 - CLOCK1);

        CLOCK1 = ESP.getCycleCount();
        xTaskCreate(taskEncrypt, "EncryptTask", 12000, NULL, 1, &taskHandleEncrypt);
        CLOCK2 = ESP.getCycleCount();
        if (taskHandleEncrypt != NULL){
            vTaskDelete(taskHandleEncrypt);
            taskHandleEncrypt = NULL;
        }
        CLOCK_enc = CLOCK_enc + (CLOCK2 - CLOCK1);

        CLOCK1 = ESP.getCycleCount();
        xTaskCreate(taskDecrypt, "DecryptTask", 13000, NULL, 1, &taskHandleDecrypt);
        CLOCK2 = ESP.getCycleCount();
        if (taskHandleDecrypt != NULL){
            vTaskDelete(taskHandleDecrypt);
            taskHandleDecrypt = NULL;
        }
        CLOCK_dec = CLOCK_dec + (CLOCK2 - CLOCK1);

    }

        Serial.println("Averages:");
        Serial.print("Cycle Count key_pair: ");
        Serial.println(CLOCK_kp/WORD_NUMBER);
        Serial.print("Cycle Count enc: ");
        Serial.println(CLOCK_enc/WORD_NUMBER);
        Serial.print("Cycle Count dec: ");
        Serial.println(CLOCK_dec/WORD_NUMBER);
        Serial.println("");

        Serial.print("Time key_pair: ");
        Serial.print(elapsedTime_kp/WORD_NUMBER);
        Serial.println("µs");
        Serial.print("Time enc: ");
        Serial.print(elapsedTime_enc/WORD_NUMBER);
        Serial.println("µs");
        Serial.print("Time dec: ");
        Serial.print(elapsedTime_dec/WORD_NUMBER);
        Serial.println("µs");
        Serial.println("");

        Serial.println("Total:");
        Serial.print("Cycle Count key_pair: ");
        Serial.println(CLOCK_kp);
        Serial.print("Cycle Count enc: ");
        Serial.println(CLOCK_enc);
        Serial.print("Cycle Count dec: ");
        Serial.println(CLOCK_dec);
        Serial.println("");

        Serial.print("Time key_pair: ");
        Serial.print(elapsedTime_kp);
        Serial.println("µs");
        Serial.print("Time enc: ");
        Serial.print(elapsedTime_enc);
        Serial.println("µs");
        Serial.print("Time dec: ");
        Serial.print(elapsedTime_dec);
        Serial.println("µs");
        Serial.println("");

        Serial.println("----- END LIGHT SABER ------");
}

void loop()
{
}
