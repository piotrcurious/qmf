//A bidirectional LFSR is a shift register that can shift bits either to the left or to the right depending on a control signal. The input bit is a linear function of some bits of the previous state, usually XOR1. A bidirectional LFSR can be used for generating pseudo-random numbers or for encryption/decryption2.

//Here is a possible C code for implementing a bidirectional LFSR optimized for 8-bit bit shift operations with LFSR position tracking:

#include <stdio.h>
#include <stdint.h>

// Define an 8-bit primitive polynomial x^8 + x^4 + x^3 + x^2 + 1
#define POLY 0x1D

// Define a global variable to store the current state of the LFSR
uint8_t lfsr = 0x01;

// Define a global variable to store the current direction of shifting (0 for left, 1 for right)
uint8_t dir = 0;

// Define a function to perform one step of shifting and return the output bit
uint8_t shift() {
    uint8_t out; // Output bit
    if (dir == 0) { // Shift left
        out = (lfsr >> 7) & 1; // Get the most significant bit as output
        lfsr <<= 1; // Shift left by one bit
        if (out) { // If output is 1, XOR with polynomial
            lfsr ^= POLY;
        }
    } else { // Shift right
        out = lfsr & 1; // Get the least significant bit as output
        lfsr >>= 1; // Shift right by one bit
        if (out) { // If output is 1, XOR with reversed polynomial
            lfsr ^= POLY << (8 - sizeof(POLY));
        }
    }
    return out;
}

// Define a function to change the direction of shifting based on a control signal
void change_dir(uint8_t ctrl) {
    dir = ctrl & 1; // Get the least significant bit of ctrl as direction
}

// Define a function to print the current state and direction of the LFSR
void print_lfsr() {
    printf("LFSR: %02X\n", lfsr); // Print state in hexadecimal format with leading zeros
    printf("DIR: %d\n", dir); // Print direction as binary digit
}

int main() {
    int i;
    print_lfsr(); // Print initial state and direction

    for (i = 0; i < 10; i++) { // Perform ten steps of shifting and print output bits
        printf("OUT: %d\n", shift());
        print_lfsr();
    }

    change_dir(0x55); // Change direction to right using an arbitrary control signal

    for (i = 0; i < 10; i++) { // Perform ten more steps of shifting and print output bits 
        printf("OUT: %d\n", shift());
        print_lfsr();
    }

    return 0;
}
