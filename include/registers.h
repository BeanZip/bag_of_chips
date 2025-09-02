#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <raylib.h>
#include "../include/bytes.h"

/**
 * Display buffer for the Chip-8
 */
typedef uint8_t c_display[32][64];


typedef struct{
    uint8_t V[16]; // 16 registers (V0-VF)
    uint16_t I;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint16_t PC;
    uint8_t SP;
    uint16_t stack[16];
    uint8_t keypad[16];
} registers;

void set_register(uint8_t idx, uint8_t value, registers *reg);

uint8_t get_register(uint8_t idx, registers *reg);

void clear_display(c_display display);

void boot_cpu(registers* reg, Memory *mem, c_display display, Sound* beep_sound);

void reset_pc(registers *reg);

void update_timers(registers *reg, Sound* beep_sound);

bool load_rom(registers *reg, const char* rom_name, Memory *mem);

void run_cycle(registers *reg, Memory *mem, c_display display, Sound* beep_sound);

void set_carry_flag(bool carry, registers *reg);

/**
 * Fetch the next instruction from memory.
 */
uint16_t fetch(Memory *mem, registers* reg);

/**
 * Execute the current instruction.
 */
void execute(registers *reg, Memory *mem, c_display display);