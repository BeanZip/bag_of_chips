#include "../include/bytes.h"

//Just a get/set for memory

void write_memory(Memory *memory, uint16_t address, uint8_t value) {
    memory->bytes[address] = value;
}

uint8_t read_memory(Memory* memory, uint16_t address){
    return memory->bytes[address];  
}
