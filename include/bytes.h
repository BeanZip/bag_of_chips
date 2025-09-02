#pragma once


#include <stddef.h>
#include <stdint.h>

typedef struct{
    uint8_t *bytes;
} Memory;


/**
 * Write a byte to the specified memory address.
 */
void write_memory(Memory *memory, uint16_t address, uint8_t value);

/**
 * Reads the byte to the specified memory address.
 */
uint8_t read_memory(Memory *memory, uint16_t address);