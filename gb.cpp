#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <cstring>
#include <vector>
#include <chrono>
#include <raylib.h>  // For graphical rendering

using namespace std;

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144
#define SCALE 4
#define MEMORY_SIZE 0x10000
#define ROM_SIZE 0x8000
#define MAX_ROM_BANKS 128
#define MAX_RAM_BANKS 4

// CPU structure
struct CPU {
    uint8_t memory[MEMORY_SIZE]; // 64KB memory space (Game Boy)
    uint16_t pc;                 // Program Counter
    uint16_t sp;                 // Stack Pointer
    uint8_t a, b, c, d, e, h, l; // Registers
    uint8_t zf, nf, hf, cf;      // Flags
    uint8_t ie;                  // Interrupt Enable Register
    uint8_t iflags;              // Interrupt Flags Register
    uint8_t timer_counter;       // Timer Counter Register
    uint8_t timer_divider;       // Timer Divider Register
    uint8_t timer_modulo;        // Timer Modulo Register
    uint8_t timer_control;       // Timer Control Register
    uint8_t selected_bank;       // Currently selected ROM bank
    uint8_t selected_ram_bank;   // Currently selected RAM bank
};

// ROM header structure
struct ROMHeader {
    char title[16];
    uint8_t cartridge_type;
    uint8_t rom_size;
    uint8_t ram_size;
    uint8_t destination_code;
    uint8_t old_licensee_code;
    uint8_t mask_rom_version;
    uint8_t header_checksum;
    uint8_t global_checksum[2];
};

// Define the types of Memory Bank Controllers (MBC)
enum class MBCType {
    NONE = 0,
    MBC1,
    MBC2,
    MBC3,
    MBC5
};

// Graphics structure (simplified)
struct Graphics {
    uint8_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];  // Pixel data
};

// Initialize the CPU and memory
void initialize_cpu(CPU &cpu) {
    memset(cpu.memory, 0, MEMORY_SIZE);
    cpu.pc = 0x0100;  // ROM execution starts at 0x0100
    cpu.sp = 0xFFFE;  // Stack starts at 0xFFFE
    cpu.ie = 0;       // Interrupt enable register
    cpu.iflags = 0;   // Interrupt flags
    cpu.timer_counter = 0;
    cpu.timer_divider = 0;
    cpu.timer_modulo = 0;
    cpu.timer_control = 0;
    cpu.selected_bank = 1; // Default to first ROM bank
    cpu.selected_ram_bank = 0; // Default to first RAM bank
}

// Load the ROM into memory
bool load_rom(CPU &cpu, const string &filename, MBCType &mbc_type) {
    ifstream rom_file(filename, ios::binary);
    if (!rom_file.is_open()) {
        cerr << "Failed to open ROM file." << endl;
        return false;
    }

    // Read the ROM header
    ROMHeader header;
    rom_file.read(reinterpret_cast<char*>(&header), sizeof(ROMHeader));
    if (!rom_file) {
        cerr << "Failed to read ROM header." << endl;
        return false;
    }

    // Store ROM title
    cout << "ROM Title: ";
    for (int i = 0; i < 16; ++i) {
        if (header.title[i] != 0) {
            cout << header.title[i];
        }
    }
    cout << endl;

    // Determine the MBC type based on the cartridge type field
    switch (header.cartridge_type) {
        case 0x00: mbc_type = MBCType::NONE; break;
        case 0x01: mbc_type = MBCType::MBC1; break;
        case 0x02: mbc_type = MBCType::MBC2; break;
        case 0x03: mbc_type = MBCType::MBC3; break;
        case 0x05: mbc_type = MBCType::MBC5; break;
        default: mbc_type = MBCType::NONE; break;
    }

    // Load ROM into memory (starting at 0x0100)
    rom_file.seekg(0x0100);
    rom_file.read(reinterpret_cast<char*>(&cpu.memory[0x0100]), ROM_SIZE);

    return true;
}

// MBC1 - Memory Bank Controller 1 functions
void handle_mbc1(CPU &cpu, uint8_t opcode) {
    // MBC1-specific functionality, handling bank switching
    switch (opcode) {
        case 0x0A:  // Example: Bank switch operation (simplified)
            cpu.selected_bank = cpu.memory[0x2000];  // Just a simple example
            break;
        default:
            break;
    }
}

// MBC2 - Memory Bank Controller 2 functions
void handle_mbc2(CPU &cpu, uint8_t opcode) {
    // MBC2-specific functionality
    // MBC2 typically handles 512KB ROMs, and simple RAM handling
    switch (opcode) {
        case 0x0A:  // Example: Bank switch operation for MBC2
            cpu.selected_bank = cpu.memory[0x2000];
            break;
        default:
            break;
    }
}

// MBC3 - Memory Bank Controller 3 functions (with RTC support)
void handle_mbc3(CPU &cpu, uint8_t opcode) {
    // MBC3-specific functionality, handles up to 64KB RAM and RTC
    switch (opcode) {
        case 0x0A:  // Example: Bank switch operation for MBC3
            cpu.selected_bank = cpu.memory[0x2000];
            break;
        default:
            break;
    }
}

// MBC5 - Memory Bank Controller 5 functions
void handle_mbc5(CPU &cpu, uint8_t opcode) {
    // MBC5-specific functionality, 32KB RAM and up to 2MB ROM
    switch (opcode) {
        case 0x0A:  // Example: Bank switch operation for MBC5
            cpu.selected_bank = cpu.memory[0x2000];
            break;
        default:
            break;
    }
}

// Memory Access - based on selected ROM bank
uint8_t read_memory(CPU &cpu, uint16_t address) {
    if (address < 0x8000) {
        return cpu.memory[address]; // Read ROM directly
    }
    else if (address >= 0x8000 && address < 0xA000) {
        // Handle ROM bank switching based on MBC type
        switch (cpu.selected_bank) {
            case 1: // Example: Handle MBC1
                handle_mbc1(cpu, cpu.memory[address]);
                break;
            case 2: // Example: Handle MBC2
                handle_mbc2(cpu, cpu.memory[address]);
                break;
            case 3: // Example: Handle MBC3
                handle_mbc3(cpu, cpu.memory[address]);
                break;
            case 4: // Example: Handle MBC5
                handle_mbc5(cpu, cpu.memory[address]);
                break;
            default:
                break;
        }
        return cpu.memory[address]; // Simplified memory access
    }
    return cpu.memory[address];
}

// Simple opcode handler (for demonstration, needs more opcodes)

void execute_opcode(CPU &cpu) {
    uint8_t opcode = cpu.memory[cpu.pc++];
    switch (opcode) {
        case 0x00:  // NOP (No operation)
            break;
        case 0x01:  // LD BC, nn (Load 16-bit immediate into BC)
            cpu.b = cpu.memory[cpu.pc++];
            cpu.c = cpu.memory[cpu.pc++];
            break;
        case 0x02:  // LD (BC), A (Store A into BC memory)
            cpu.memory[(cpu.b << 8) | cpu.c] = cpu.a;
            break;
        case 0x03:  // INC BC (Increment BC)
            cpu.c++;
            if (cpu.c == 0) cpu.b++;
            break;
        case 0x04:  // INC B (Increment B)
            cpu.b++;
            cpu.zf = (cpu.b == 0);
            cpu.nf = 0;
            cpu.hf = ((cpu.b & 0x0F) == 0);
            break;
        case 0x05:  // DEC B (Decrement B)
            cpu.b--;
            cpu.zf = (cpu.b == 0);
            cpu.nf = 1;
            cpu.hf = ((cpu.b & 0x0F) == 0x0F);
            break;
        case 0x06:  // LD B, n (Load 8-bit immediate into B)
            cpu.b = cpu.memory[cpu.pc++];
            break;
        case 0x07:  // RLCA (Rotate A left, old bit 7 to Carry flag)
            cpu.cf = (cpu.a & 0x80) >> 7;
            cpu.a = (cpu.a << 1) | cpu.cf;
            cpu.zf = 0;
            cpu.nf = 0;
            cpu.hf = 0;
            break;
        case 0x08:  // LD (nn), SP (Store SP at address nn)
            {
                uint16_t addr = cpu.memory[cpu.pc++] | (cpu.memory[cpu.pc++] << 8);
                cpu.memory[addr] = cpu.sp & 0xFF;
                cpu.memory[addr + 1] = (cpu.sp >> 8) & 0xFF;
            }
            break;
        case 0x09:  // ADD HL, BC (Add BC to HL)
            {
                uint32_t hl = (cpu.h << 8) | cpu.l;
                uint32_t bc = (cpu.b << 8) | cpu.c;
                uint32_t result = hl + bc;
                cpu.h = (result >> 8) & 0xFF;
                cpu.l = result & 0xFF;
                cpu.cf = (result > 0xFFFF);
                cpu.hf = ((hl & 0xFFF) + (bc & 0xFFF) > 0xFFF);
                cpu.nf = 0;
            }
            break;
        case 0x0A:  // LD A, (BC) (Load A from address BC)
            cpu.a = cpu.memory[(cpu.b << 8) | cpu.c];
            break;
        case 0x0B:  // DEC BC (Decrement BC)
            cpu.c--;
            if (cpu.c == 0xFF) cpu.b--;
            break;
        case 0x0C:  // INC C (Increment C)
            cpu.c++;
            cpu.zf = (cpu.c == 0);
            cpu.nf = 0;
            cpu.hf = ((cpu.c & 0x0F) == 0);
            break;
        case 0x0D:  // DEC C (Decrement C)
            cpu.c--;
            cpu.zf = (cpu.c == 0);
            cpu.nf = 1;
            cpu.hf = ((cpu.c & 0x0F) == 0x0F);
            break;
        case 0x0E:  // LD C, n (Load 8-bit immediate into C)
            cpu.c = cpu.memory[cpu.pc++];
            break;
        case 0x0F:  // RRCA (Rotate A right, old bit 0 to Carry flag)
            cpu.cf = cpu.a & 0x01;
            cpu.a = (cpu.a >> 1) | (cpu.cf << 7);
            cpu.zf = 0;
            cpu.nf = 0;
            cpu.hf = 0;
            break;
        // Add more opcodes as needed
        default:
            cerr << "Unhandled opcode: 0x" << hex << (int)opcode << endl;
            break;
    }
}


// Handle interrupts (for demonstration)
void handle_interrupts(CPU &cpu) {
    if (cpu.iflags & 0x01) {
        // Handle V-Blank interrupt
        cpu.iflags &= ~0x01;  // Clear interrupt flag
    }
    if (cpu.iflags & 0x02) {
        // Handle Timer interrupt
        cpu.iflags &= ~0x02;  // Clear interrupt flag
    }
}

// Graphics rendering (simplified, using Raylib)
void render_graphics(const CPU &cpu, Graphics &graphics) {
    // Clear framebuffer (set all pixels to 0)
    memset(graphics.framebuffer, 0, sizeof(graphics.framebuffer));

    // Handle background tiles (assuming tilemap is at 0x9800)
    for (int tile_y = 0; tile_y < 18; ++tile_y) {  // 18 tiles vertically (144 / 8)
        for (int tile_x = 0; tile_x < 20; ++tile_x) { // 20 tiles horizontally (160 / 8)
            uint16_t tile_index = cpu.memory[0x9800 + (tile_y * 32) + tile_x];  // Tile map points to VRAM tile index

            // Get tile data (8x8 pixels)
            for (int y = 0; y < 8; ++y) {
                uint8_t tile_data_low = cpu.memory[0x8000 + (tile_index * 16) + y * 2];   // Low byte of tile data
                uint8_t tile_data_high = cpu.memory[0x8000 + (tile_index * 16) + y * 2 + 1]; // High byte of tile data

                // Each bit represents a pixel in the tile row (0 or 1)
                for (int x = 0; x < 8; ++x) {
                    bool pixel = ((tile_data_low >> (7 - x)) & 1) | (((tile_data_high >> (7 - x)) & 1) << 1);
                    int screen_x = tile_x * 8 + x;
                    int screen_y = tile_y * 8 + y;

                    // Ensure within bounds
                    if (screen_x < SCREEN_WIDTH && screen_y < SCREEN_HEIGHT) {
                        graphics.framebuffer[screen_y * SCREEN_WIDTH + screen_x] = pixel * 255; // Set to white or black
                    }
                }
            }
        }
    }

    // Handle sprites (OAM at 0xFE00 - 0xFE9F)
    for (int i = 0; i < 40; ++i) {  // Up to 40 sprites
        uint8_t sprite_y = cpu.memory[0xFE00 + i * 4]; // Y position
        uint8_t sprite_x = cpu.memory[0xFE00 + i * 4 + 1]; // X position
        uint8_t sprite_tile = cpu.memory[0xFE00 + i * 4 + 2]; // Tile index
        uint8_t sprite_attributes = cpu.memory[0xFE00 + i * 4 + 3]; // Attributes (flip, priority, etc.)

        if (sprite_y >= 0 && sprite_y < SCREEN_HEIGHT && sprite_x >= 0 && sprite_x < SCREEN_WIDTH) {
            // Extract sprite flipping and other attributes
            bool flip_x = sprite_attributes & 0x20;
            bool flip_y = sprite_attributes & 0x40;

            // Get the sprite tile data (8x8 or 8x16)
            for (int y = 0; y < 8; ++y) {
                uint8_t tile_data_low = cpu.memory[0x8000 + (sprite_tile * 16) + y * 2];
                uint8_t tile_data_high = cpu.memory[0x8000 + (sprite_tile * 16) + y * 2 + 1];

                for (int x = 0; x < 8; ++x) {
                    // Flip the sprite horizontally or vertically if necessary
                    int sprite_x_flip = flip_x ? 7 - x : x;
                    int sprite_y_flip = flip_y ? 7 - y : y;

                    bool pixel = ((tile_data_low >> (7 - sprite_x_flip)) & 1) | (((tile_data_high >> (7 - sprite_x_flip)) & 1) << 1);
                    int screen_x = sprite_x + sprite_x_flip;
                    int screen_y = sprite_y + sprite_y_flip;

                    // Ensure within bounds and set pixel
                    if (screen_x < SCREEN_WIDTH && screen_y < SCREEN_HEIGHT) {
                        graphics.framebuffer[screen_y * SCREEN_WIDTH + screen_x] = pixel * 255;  // Set to white or black
                    }
                }
            }
        }
    }

    // Raylib rendering (draw scaled pixels)
    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            // Use the framebuffer to scale up the pixels for display
            unsigned char color_value = graphics.framebuffer[y * SCREEN_WIDTH + x]; // Use unsigned char instead of int
            DrawPixel(x * SCALE, y * SCALE, Color{color_value, color_value, color_value, 255});
        }
    }
}

// Game loop (simplified)
void game_loop(CPU &cpu, MBCType mbc_type) {
    Graphics graphics;
    initialize_cpu(cpu);

    InitWindow(SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, "CoolBoy Emulator");

    while (!WindowShouldClose()) {
        // Handle input and interrupts
        handle_interrupts(cpu);

        // Execute CPU cycles (simplified for demonstration)
        execute_opcode(cpu);

        // Render graphics
        render_graphics(cpu, graphics);

        BeginDrawing();
        ClearBackground(BLACK);

        // Draw the framebuffer to the screen
        for (int y = 0; y < SCREEN_HEIGHT; ++y) {
            for (int x = 0; x < SCREEN_WIDTH; ++x) {
                unsigned char color_value = graphics.framebuffer[y * SCREEN_WIDTH + x];
                DrawPixel(x * SCALE, y * SCALE, Color{color_value, color_value, color_value, 255});
            }
        }

        EndDrawing();
    }

    CloseWindow();
}

int main() {
    // Initialize Raylib window
    InitWindow(SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, "Game Boy Emulator");
    SetTargetFPS(60);

    CPU cpu;
    MBCType mbc_type = MBCType::NONE;

    // Load ROM and handle errors
    if (!load_rom(cpu, "game.gb", mbc_type)) {
        cerr << "Error loading ROM!" << endl;
        return 1;
    }

    // Start the game loop
    game_loop(cpu, mbc_type);

    // Close window and clean up
    CloseWindow();

    return 0;
}
