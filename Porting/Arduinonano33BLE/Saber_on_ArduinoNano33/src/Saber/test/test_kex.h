#ifndef SABER_TEST_H
#define SABER_TEST_H

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "../rng.h"  // Include the random number generator definition

// Define key and ciphertext sizes based on SABER parameters (assuming they are in a separate header)
#include "../SABER_params.h"

// Function prototypes
 int test_kem_cca(void);

#endif  // SABER_TEST_H