#include <Arduino.h>
#include <stdio.h>

#include "hqc-256/api.h"
#include "hqc-256/parameters.h"

#define WORD_NUMBER 1

const uint8_t parole_in_array[WORD_NUMBER][11] = {
   { 8, 98, 110, 118, 122, 117, 108, 120, 98, 113, 115 }, // bnvzulxbqs
   //...
};

TaskHandle_t taskHandleKeypair = NULL;
TaskHandle_t taskHandleEncrypt = NULL;
TaskHandle_t taskHandleDecrypt = NULL;

unsigned char pk[PUBLIC_KEY_BYTES];
unsigned char sk[SECRET_KEY_BYTES];
unsigned char ct[CIPHERTEXT_BYTES];
unsigned char key1[SHARED_SECRET_BYTES];
unsigned char key2[SHARED_SECRET_BYTES];

unsigned char k1;
unsigned char k2;

// Variabile per scegliere se calcolare tempo o cicli di clock
bool calcolaTempo = true;  // Imposta a 'true' per calcolare il tempo, 'false' per cicli di clock

// Variabili per i cicli di clock
uint32_t CLOCK1, CLOCK2;
uint32_t CLOCK_kp = 0, CLOCK_enc = 0, CLOCK_dec = 0;

// Variabili per il tempo (microsecondi)
unsigned long startTime;
unsigned long endTime;
unsigned long elapsedTime_kp = 0, elapsedTime_enc = 0, elapsedTime_dec = 0;

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
        k1 = crypto_kem_enc(ct, key1, pk);
        xQueueSend(encryptionQueue, ct, portMAX_DELAY);
        xQueueSend(encryptionQueue, key1, portMAX_DELAY);
    }
    vTaskDelete(NULL); // cancello il task chiamante
}

// Funzione del task per la decifratura
void taskDecrypt(void *pvParameters)
{
    if (xQueueReceive(encryptionQueue, ct, portMAX_DELAY) == pdTRUE)
    {
        xQueueReceive(keypairQueue, sk, portMAX_DELAY);
        k2 = crypto_kem_dec(key2, ct, sk);
        xQueueSend(decryptionQueue, key2, portMAX_DELAY);
    }
    vTaskDelete(NULL);
}

// Funzione per stampare l'array
void printArray(const unsigned char* array, size_t size)
{
    Serial.print("Array: ");
    for (size_t i = 0; i < size; i++)
    {
        Serial.print(array[i], HEX);
        Serial.print(" "); // Stampa uno spazio tra i valori
    }
    Serial.println(); // Vai a capo dopo aver stampato l'array
}

void setup()
{
    Serial.begin(115200);

    pinMode(12, OUTPUT); // Imposta il pin 13 come uscita

    // Creazione delle code
    keypairQueue = xQueueCreate(2, sizeof(uint8_t) * PUBLIC_KEY_BYTES);
    encryptionQueue = xQueueCreate(2, sizeof(uint8_t) * CIPHERTEXT_BYTES);
    decryptionQueue = xQueueCreate(2, sizeof(uint8_t) * SHARED_SECRET_BYTES);

    if (keypairQueue == NULL || encryptionQueue == NULL || decryptionQueue == NULL)
    {
        Serial.println("Failed to create queue");
        return;
    }

    digitalWrite(12, HIGH); // Accendi il pin 12 (imposta HIGH)

    for (int i = 0; i < WORD_NUMBER; i++)
    {
        if (i < WORD_NUMBER) { // Assicurati di non superare la dimensione dell'array
            memcpy(ct, parole_in_array[i], sizeof(parole_in_array[i]));
        }


        // Misurazione del tempo
        startTime = micros();
        xTaskCreate(taskKeypair, "KeypairTask", 30000, NULL, 1, &taskHandleKeypair);
        endTime = micros();
        if (taskHandleKeypair != NULL){
            vTaskDelete(taskHandleKeypair);
            taskHandleKeypair = NULL;
        }
        elapsedTime_kp += (endTime - startTime);

        startTime = micros();
        xTaskCreate(taskEncrypt, "EncryptTask", 50000, NULL, 1, &taskHandleEncrypt);
        endTime = micros();
        if (taskHandleEncrypt != NULL){
            vTaskDelete(taskHandleEncrypt);
            taskHandleEncrypt = NULL;
        }
        elapsedTime_enc += (endTime - startTime);

        startTime = micros();
        xTaskCreate(taskDecrypt, "DecryptTask", 38000, NULL, 1, &taskHandleDecrypt);
        endTime = micros();
        if (taskHandleDecrypt != NULL){
            vTaskDelete(taskHandleDecrypt);
            taskHandleDecrypt = NULL;
        }
        elapsedTime_dec += (endTime - startTime);
    }

    digitalWrite(12, LOW);  // Spegni il pin 12 (imposta LOW)

    // Misurazione dei cicli di CPU
    for (int i = 0; i < WORD_NUMBER; i++)
    {

        if (i < WORD_NUMBER) { // Assicurati di non superare la dimensione dell'array
            memcpy(ct, parole_in_array[i], sizeof(parole_in_array[i]));
        }


        CLOCK1 = ESP.getCycleCount();
        xTaskCreate(taskKeypair, "KeypairTask", 30000, NULL, 1, &taskHandleKeypair);
        CLOCK2 = ESP.getCycleCount();
        if (taskHandleKeypair != NULL){
            vTaskDelete(taskHandleKeypair);
            taskHandleKeypair = NULL;
        }
        CLOCK_kp += (CLOCK2 - CLOCK1);

        CLOCK1 = ESP.getCycleCount();
        xTaskCreate(taskEncrypt, "EncryptTask", 50000, NULL, 1, &taskHandleEncrypt);
        CLOCK2 = ESP.getCycleCount();
        if (taskHandleEncrypt != NULL){
            vTaskDelete(taskHandleEncrypt);
            taskHandleEncrypt = NULL;
        }
        CLOCK_enc += (CLOCK2 - CLOCK1);

        CLOCK1 = ESP.getCycleCount();
        xTaskCreate(taskDecrypt, "DecryptTask", 38000, NULL, 1, &taskHandleDecrypt);
        CLOCK2 = ESP.getCycleCount();
        if (taskHandleDecrypt != NULL){
            vTaskDelete(taskHandleDecrypt);
            taskHandleDecrypt = NULL;
        }
        CLOCK_dec += (CLOCK2 - CLOCK1);
    }

    // Stampa dei risultati
    Serial.println("Averages:");
    Serial.print("Cycle Count key_pair: ");
    Serial.println(CLOCK_kp / WORD_NUMBER);
    Serial.print("Cycle Count enc: ");
    Serial.println(CLOCK_enc / WORD_NUMBER);
    Serial.print("Cycle Count dec: ");
    Serial.println(CLOCK_dec / WORD_NUMBER);

    Serial.print("Time key_pair: ");
    Serial.print(elapsedTime_kp / WORD_NUMBER);
    Serial.println("µs");

    Serial.print("Time enc: ");
    Serial.print(elapsedTime_enc / WORD_NUMBER);
    Serial.println("µs");

    Serial.print("Time dec: ");
    Serial.print(elapsedTime_dec / WORD_NUMBER);
    Serial.println("µs");

    Serial.println("Total:");
    Serial.print("Cycle Count key_pair: ");
    Serial.println(CLOCK_kp );
    Serial.print("Cycle Count enc: ");
    Serial.println(CLOCK_enc );
    Serial.print("Cycle Count dec: ");
    Serial.println(CLOCK_dec );

    Serial.print("Time key_pair: ");
    Serial.print(elapsedTime_kp);
    Serial.println("µs");

    Serial.print("Time enc: ");
    Serial.print(elapsedTime_enc);
    Serial.println("µs");

    Serial.print("Time dec: ");
    Serial.print(elapsedTime_dec);
    Serial.println("µs");

    Serial.println("----- END HQC ------");
}

void loop()
{
}
