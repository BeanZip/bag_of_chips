#include "../include/registers.h"
#include "../include/chip8_font.h"
#include "../include/chlog.h"
#include <stdlib.h>
#include <raylib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>



void set_carry_flag(bool carry, registers *reg){
    reg->V[0xF] = carry ? 1 : 0;
}

void set_register(uint8_t idx, uint8_t value, registers *reg) {
    if(idx < 16){
        reg->V[idx] = value;
    }
}  

uint8_t get_register(uint8_t idx, registers *reg) {
    if(idx < 16){
        return reg->V[idx];
    }
    return 0; 
}

void reset_pc(registers *reg){
    reg->PC = 0x200;
}

uint16_t fetch(Memory *mem, registers* reg) {
    uint8_t high_byte = read_memory(mem, reg->PC);
    uint8_t low_byte = read_memory(mem, reg->PC + 1);
    return (high_byte << 8) | low_byte;
}

void clear_display(c_display display){
    size_t total_size = sizeof(uint8_t) * 32 * 64;
    memset(display, 0, total_size);
}

void update_timers(registers *reg, Sound* beep_sound){
    if(reg->delay_timer > 0){
        --reg->delay_timer;
    }

    // Play sound only when the sound timer first becomes active
    if(reg->sound_timer == 1){
        PlaySound(*beep_sound);
    }
    
    if(reg->sound_timer > 0){
        --reg->sound_timer;
    }
}

//TODO: Add Boot Function for CPU




void boot_cpu(registers *reg, Memory *mem, c_display display, Sound* beep_sound) {
    reg->I = 0;
    reg->delay_timer = 0;
    reg->sound_timer = 0;
    reg->SP = 0;
    memset(reg->V, 0, 16);
    memset(reg->stack, 0, 16);
    memset(reg->keypad, 0, 16);
    memset(mem->bytes, 0 , 4096);
    reset_pc(reg);
    for(int i = 0; i < 80; i++){
        mem->bytes[i] = fontset[i];
    }
    clear_display(display);
    
    srand(time(NULL));

    if(beep_sound == NULL){
        log_warn("Beep sound is NULL during CPU boot.");
    }
}

void execute(registers *reg, Memory *mem, c_display display) {
    uint16_t instruction = fetch(mem, reg);

    uint16_t nnn = instruction & 0x0FFF;
    uint8_t nn = instruction & 0x00FF;
    uint8_t n = instruction & 0x000F; 
    uint8_t x = (instruction & 0x0F00) >> 8;
    uint8_t y = (instruction & 0x00F0) >> 4;
    uint8_t op = instruction >> 12;

    reg->PC += 2;
    switch(op){
        case 0x0:
        switch(instruction){
            case 0x00E0:
                clear_display(display);
                break;
            case 0x00EE:
                if(reg->SP > 0){
                    reg->SP--;
                    reg->PC = reg->stack[reg->SP];
                }
                break;
        }
        break;

        case 0x1:
        reg->PC = nnn;
        break;
        case 0x2:
        if(reg->SP < 16){
            reg->stack[reg->SP] = reg->PC;
            reg->SP++;
            reg->PC = nnn;
        }
        break;
        case 0x3:
        if(reg->V[x] == nn){
            reg->PC += 2;
        }
        break;
        case 0x4:
        if(reg->V[x] != nn){
            reg->PC += 2;
        }
        break;

        case 0x5:
        if(reg->V[x] == reg->V[y]){
            reg->PC += 2;
        }
        break;
        case 0x6:
        reg->V[x] = nn;
        break;
        case 0x7:
        reg->V[x] += nn;
        break;
        case 0x8:
        switch(n){
            case 0x0:
            reg->V[x] = reg->V[y];
            break;
            case 0x1:
            reg->V[x] |= reg->V[y];
            break;
            case 0x2:
            reg->V[x] &= reg->V[y];
            break;
            case 0x3:
            reg->V[x] ^= reg->V[y];
            break;
            case 0x4:
            reg->V[x] += reg->V[y];
            set_carry_flag(reg->V[x] < reg->V[y], reg);
            break;
            case 0x5:
            reg->V[x] -= reg->V[y];
            set_carry_flag(reg->V[x] > reg->V[y], reg);
            break;
            case 0x6:
            reg->V[x] >>= 1;
            set_carry_flag(reg->V[x] & 0x1, reg);
            break;
           case 0x7:
           reg->V[x] = reg->V[y] - reg->V[x];
           set_carry_flag(reg->V[y] > reg->V[x], reg);
           break; 
           case 0xE:
              reg->V[x] <<= 1; //Shift left 1 bitty
              set_carry_flag((reg->V[x] & 0x80) != 0, reg);
           break;
        }
        break;
        case 0x9:
        if(reg->V[x] != reg->V[y]){
            reg->PC += 2;
        }
        break;
        case 0xA:
        reg->I = nnn;
        break;
        case 0xB:
        reg->PC = nnn + reg->V[0];
        break;
        case 0xC:
        reg->V[x] = (rand() % 256) & nn;
        break;
        case 0xD:{
            uint8_t xCord = reg->V[x] % 64;
            uint8_t yCord = reg->V[y] % 32;
            uint8_t height = n;
            reg->V[0xF] = 0;
            for(int row = 0; row < height; row++){
                if(yCord + row >= 32) break;
                uint8_t sprite_byte = read_memory(mem, reg->I + row);
                for(int col = 0; col < 8; col++){
                    if(xCord + col >= 64) break;
                    uint8_t sprite_pixel = (sprite_byte >> (7 - col)) & 0x1;
                    if(sprite_pixel){
                        if(display[yCord + row][xCord + col] == 1){
                            reg->V[0xF] = 1;
                        }
                        display[yCord + row][xCord + col] ^= 1;
                    }
                }
            }
        break;
        }
        case 0xE:
        switch(nn){
            case 0x9E:
            if(reg->keypad[reg->V[x]]){
                reg->PC += 2;
            }
            break;
            case 0xA1:
            if(!reg->keypad[reg->V[x]]){
                reg->PC += 2;
            }
            break;
        }
        break;
        case 0xF:
        switch(nn){
            case 0x07:
            reg->V[x] = reg->delay_timer;
            break;
            case 0x0A:
            {
                bool key_pressed = false;
                for(int i = 0; i < 16; i++){
                    if(reg->keypad[i]){
                        reg->V[x] = i;
                        key_pressed = true;
                        break;
                    }
                }
                if(!key_pressed){
                    reg->PC -= 2;
                }
            }
            break;
            case 0x15:
            reg->delay_timer = reg->V[x];
            break;
            case 0x18:
            reg->sound_timer = reg->V[x];
            break;
            case 0x1E:
            reg->I += reg->V[x];
            break;
            case 0x29:
            reg->I = reg->V[x] * 5; //Each sprite is 5 bytes long
            break;
            case 0x33:
            {
                uint8_t value = reg->V[x];
                mem->bytes[reg->I] = value / 100;
                mem->bytes[reg->I + 1] = (value / 10) % 10;
                mem->bytes[reg->I + 2] = value % 10;
            }
            break;
            case 0x55:
            for(int i = 0; i <= x; i++){
                mem->bytes[reg->I + i] = reg->V[i];
            }
            break;
            case 0x65:
            for(int i = 0; i <= x; i++){
                reg->V[i] = mem->bytes[reg->I + i];
            }
            break;
        }
    }
}

//NOTE: If this goes wrong check this function first
bool load_rom(registers *reg, const char* rom_name, Memory *mem){
    FILE* rom = fopen(rom_name, "rb");
    if(!rom){
        log_err("Failed to open ROM: %s", rom_name);
        return false;
    } 
    fseek(rom, 0, SEEK_END);
    long rom_size = ftell(rom);
    fseek(rom, 0, SEEK_SET);

    log_info("Loading ROM: %s of size %ld bytes", rom_name, rom_size);

    // Check if ROM can fit in memory (CHIP-8 has 4KB total, with program starting at 0x200)
    if (rom_size > (4096 - 0x200)) {
        log_err("ROM too large: %ld bytes. Maximum size is %d bytes", rom_size, 4096 - 0x200);
        fclose(rom);
        return false;
    }

    // Read directly into memory at the correct location (0x200)
    size_t bytes_read = fread(&mem->bytes[0x200], 1, rom_size, rom);
    if (bytes_read != rom_size) {
        log_err("Failed to read ROM file completely. Expected %ld bytes, read %zu bytes", rom_size, bytes_read);
        fclose(rom);
        return false;
    }

    fclose(rom);
    reset_pc(reg);
    return true;
}

void run_cycle(registers *reg, Memory *mem, c_display display, Sound* beep_sound){
    execute(reg, mem, display);
}