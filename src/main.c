#include <stdio.h>
#include <stdlib.h>  // For malloc/free
#include <raylib.h>
#include "../include/chlog.h"
#include "../include/registers.h"
#include <direct.h> // For _getcwd on Windows

// Tries to load the beep sound from various locations
void Test_Sound(Sound* beep_sound){
    if(beep_sound->frameCount == 0){
        log_warn("Failed to load beep sound from current directory.");
        printf("Trying alternative locations...\n");
        
        // Try in the same directory as the executable
        char exe_path[1024] = {0};
        _getcwd(exe_path, sizeof(exe_path));
        char sound_path[1200] = {0};
        sprintf(sound_path, "%s\\beep.wav", exe_path);
        printf("Attempting to load from: %s\n", sound_path);
        
        *beep_sound = LoadSound(sound_path);
        
        // If still failed, try one directory up
        if(beep_sound->frameCount == 0) {
            printf("Failed. Trying one directory up...\n");
            char* last_slash = strrchr(exe_path, '\\');
            if (last_slash) {
                *last_slash = '\0'; // Trim to parent directory
                sprintf(sound_path, "%s\\beep.wav", exe_path);
                printf("Attempting to load from: %s\n", sound_path);
                *beep_sound = LoadSound(sound_path);
            }
        }
        
        // If still failed, try a fallback absolute path as last resort
        if(beep_sound->frameCount == 0) {
            printf("Failed. Trying fallback location...\n");
            *beep_sound = LoadSound("C:/Users/Brendan/Documents/Chip_8/beep.wav");
        }
        
        if(beep_sound->frameCount == 0) {
            log_err("Failed to load sound file from all locations. Sound timer will not produce audio.");
        } else {
            printf("Successfully loaded sound file: %d frames\n", beep_sound->frameCount);
        }
    } else {
        printf("Successfully loaded sound file: %d frames\n", beep_sound->frameCount);
    }
}

int main(int argc, char* argv[]){
    bool timed_run = false;
    float run_duration = 3.0f; // Default 3 seconds
    
    if (argc > 1 && strcmp(argv[argc-1], "--timed") == 0) {
        timed_run = true;
        printf("Timed run mode: Will run for %.1f seconds and exit\n", run_duration);
        argc--;
    }
    
    char cwd[1024];
    if (_getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    } else {
        perror("_getcwd() error");
    }
    
    InitAudioDevice();
    
    // First try to load the sound from the current directory
    Sound beep_sound = LoadSound("beep.wav");
    Test_Sound(&beep_sound);
    
    registers reg;
    Memory* memory = (Memory*)malloc(sizeof(Memory));
    memory->bytes = (uint8_t*)malloc(4096 * sizeof(uint8_t)); // Allocate 4KB of memory for CHIP-8
    if (!memory->bytes) {
        log_err("Failed to allocate memory");
        return 1;
    }
    c_display display;
    
    InitWindow(640, 320, "Bag of Chips - Emulator"); 
    SetTargetFPS(60);
    
    boot_cpu(&reg, memory, display, &beep_sound);
    
    float timer = 0.0f;
    
    bool rom_loaded = false;
    if (argc > 1) {
        log_info("Loading ROM: %s", argv[1]);
        rom_loaded = load_rom(&reg, argv[1], memory);
        if (!rom_loaded) {
            log_err("Failed to load ROM: %s", argv[1]);
        } else {
            log_info("ROM loaded successfully");
        }
    } else {
        log_warn("No ROM file provided. Usage: %s <rom_file>", argv[0]);
    }
    
    const int keymap[16] = {
        KEY_X,    // 0
        KEY_ONE,  // 1
        KEY_TWO,  // 2
        KEY_THREE,// 3
        KEY_Q,    // 4
        KEY_W,    // 5
        KEY_E,    // 6
        KEY_A,    // 7
        KEY_S,    // 8
        KEY_D,    // 9
        KEY_Z,    // A
        KEY_C,    // B
        KEY_FOUR, // C
        KEY_R,    // D
        KEY_F,    // E
        KEY_V     // F
    };
    
    while (!WindowShouldClose()) {
        // Update timed run timer if active
        if (timed_run) {
            timer += GetFrameTime();
            if (timer >= run_duration) {
                printf("Timed run completed after %.2f seconds\n", timer);
                break;  // Exit the game loop
            }
        }
        
        for (int i = 0; i < 16; i++) {
            reg.keypad[i] = IsKeyDown(keymap[i]) ? 1 : 0;
        }
        
        if (IsKeyPressed(KEY_SPACE)) {
            PlaySound(beep_sound);
            printf("BEEP!\n");
        }
        
        if (IsKeyPressed(KEY_ESCAPE)) {
            break;
        }
        
        if (rom_loaded) {
            for (int i = 0; i < 10; i++) {
                run_cycle(&reg, memory, display, &beep_sound);
            }
        }
        
        update_timers(&reg, &beep_sound);
        
        BeginDrawing();
        ClearBackground(BLACK);
        
        if (rom_loaded) {
            ClearBackground(BLACK);

            for (int y = 0; y < 32; y++) {
                for (int x = 0; x < 64; x++) {
                    if (display[y][x]) {
                        DrawRectangle(x * 10, y * 10, 10, 10, RED);
                    }
                }
            }
            
            if (timed_run) {
                DrawRectangle(0, 0, 150, 30, BLACK);
                DrawText(TextFormat("Time: %.1f/%.1f", timer, run_duration), 10, 30, 16, GREEN);
            }
        } else {
            DrawText("Bag of Chips Emulator", 200, 120, 20, BLUE);
            DrawText("Sorry Dude No Rom. Use CLI (Funny Command Thingie)", 110, 160, 16, GRAY);
            DrawText("Made By Brendan B. Press ESC to exit", 120, 180, 20, GRAY);
        }
        
        // Show FPS
        DrawFPS(10, 10);
        
        EndDrawing();
    }
    
    // Cleanup
    UnloadSound(beep_sound);
    free(memory->bytes);
    free(memory);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}