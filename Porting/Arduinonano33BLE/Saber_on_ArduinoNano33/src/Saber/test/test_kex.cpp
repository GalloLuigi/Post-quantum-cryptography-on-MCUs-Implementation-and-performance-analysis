#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h> /* Fornisce funzioni per lavorare con il tempo, usata per inizializzare il generatore di numeri casuali.*/
#include <string.h> 

#include "../api.h"  
#include "../poly.h" /*  Include definizioni per lavorare con polinomi usati nell'algoritmo.*/
#include "../rng.h" /*  Include definizioni per il generatore di numeri casuali usato da SABER.*/
#include "../SABER_indcpa.h" /*  Include definizioni per lo schema crittografico a chiave pubblica IND-CPA di SABER.*/
#include "../verify.h" /*  Include funzioni per la verifica delle operazioni crittografiche.*/
#include "test_kex.h"
//#include <Esp.h>

/*
unsigned long clock1,clock2;
unsigned long clock_kp_mv,clock_cl_mv, clock_kp_sm, clock_cl_sm;
*/

/*Questa funzione esegue il test dell'algoritmo KEM (Key Encapsulation Mechanism) di SABER in modalità CCA (Chosen Ciphertext Attack).*/
int test_kem_cca()
{

/*Genera una coppia di chiavi segreta/pubblica (pk, sk).*/
  uint8_t pk[SABER_PUBLICKEYBYTES];
  uint8_t sk[SABER_SECRETKEYBYTES];

  uint8_t c[SABER_BYTES_CCA_DEC];	
  uint8_t k_a[SABER_KEYBYTES], k_b[SABER_KEYBYTES];
	
  unsigned char entropy_input[48];
	
  uint64_t i, j, repeat;
  repeat=1;

  //For time
/*
  unsigned long CLOCK1,CLOCK2;
  unsigned long CLOCK_kp,CLOCK_enc,CLOCK_dec;
  	CLOCK1 = 0;
        CLOCK2 = 0;
	CLOCK_kp = CLOCK_enc = CLOCK_dec = 0;
	clock_kp_mv=clock_cl_mv=0;
	clock_kp_sm = clock_cl_sm = 0;
*/

   
	time_t t;
   	// Intializes random number generator
   	srand((unsigned) time(&t));

    	for (i=0; i<48; i++){
        	//entropy_input[i] = rand()%256;
        	entropy_input[i] = i;
	}
    randombytes_init(entropy_input, NULL, 256);


	printf("SABER_INDCPA_PUBLICKEYBYTES=%d\n", SABER_INDCPA_PUBLICKEYBYTES);
	printf("SABER_INDCPA_SECRETKEYBYTES=%d\n", SABER_INDCPA_SECRETKEYBYTES);
	printf("SABER_PUBLICKEYBYTES=%d\n", SABER_PUBLICKEYBYTES);
	printf("SABER_SECRETKEYBYTES=%d\n", SABER_SECRETKEYBYTES);
	printf("SABER_KEYBYTES=%d\n", SABER_KEYBYTES);
	printf("SABER_HASHBYTES=%d\n", SABER_HASHBYTES);
 	printf("SABER_BYTES_CCA_DEC=%d\n", SABER_BYTES_CCA_DEC);
	printf("\n");
 

/*Esegue cicli di test ripetendo repeat volte*/
  	for(i=0; i<repeat; i++)
  	{
		//printf("The value of i is: %lld\n", i);

	    //Generazione della coppia di chiavi segrete sk e pubbliche pk
	    //CLOCK1=ESP.getCycleCount();	
	    crypto_kem_keypair(pk, sk);
	    //CLOCK2=ESP.getCycleCount();
	    //CLOCK_kp=CLOCK_kp+(CLOCK2-CLOCK1);
		/*Misura il tempo impiegato per la generazione della coppia di chiavi (CLOCK_kp).*/


	    //Key-Encapsulation call; input: pk; output: ciphertext c, shared-secret k_a;	
	    //CLOCK1=ESP.getCycleCount();	
	    //crypto_kem_enc(c, k_a, pk);
	    //CLOCK2=ESP.getCycleCount();
	    //CLOCK_enc=CLOCK_enc+(CLOCK2-CLOCK1);
		/*Esegue la crittografia usando la chiave pubblica (pk) e ottiene la chiave condivisa (k_a).
		 Misura il tempo impiegato (CLOCK_enc).*/


	    //Key-Decapsulation call; input: sk, c; output: shared-secret k_b;	
	    //CLOCK1=ESP.getCycleCount();	
	    //crypto_kem_dec(k_b, c, sk);
	    //CLOCK2=ESP.getCycleCount();
	    //CLOCK_dec=CLOCK_dec+(CLOCK2-CLOCK1);
		/*Esegue la decrittografia usando la chiave segreta (sk) e verifica che la chiave condivisa (k_b) 
		coincida con quella ottenuta in precedenza (k_a). Misura il tempo impiegato (CLOCK_dec).*/
	  

		/*
	    // Functional verification: check if k_a == k_b?
	    for(j=0; j<SABER_KEYBYTES; j++)
	    {
		//printf("%u \t %u\n", k_a[j], k_b[j]);
		if(k_a[j] != k_b[j])
		{
			printf("----- ERR CCA KEM ------\n");
			return 0;	
			break;
		}
	    }
		*/
		/*Stampa le dimensioni in byte dei parametri di SABER.
		  Stampa i tempi medi impiegati per generazione chiavi, crittografia e decrittografia su repeat iterazioni.
          Verifica la funzionalità controllando se le chiavi condivise k_a e k_b coincidono sempre.*/
  	}

	printf("Repeat is : %lld\n",repeat);
	/*
	printf("Average times key_pair: \t %llu \n",CLOCK_kp/repeat);
	printf("Average times enc: \t %llu \n",CLOCK_enc/repeat);
	printf("Average times dec: \t %llu \n",CLOCK_dec/repeat);
	printf("Average times kp mv: \t %llu \n",clock_kp_mv/repeat);
	printf("Average times cl mv: \t %llu \n",clock_cl_mv/repeat);
	printf("Average times sample_kp: \t %llu \n",clock_kp_sm/repeat);
	*/
  	return 0;
}

