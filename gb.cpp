#include <iostream>
#include <vector>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MEMORY_SIZE 0x10000 // Game Boy has 64KB of addressable memory
#define ROM_BANK_SIZE 0x4000 // 16KB ROM bank size
#define VRAM_SIZE 0x2000     // 8KB for VRAM
#define EXTERNAL_RAM_SIZE 0x2000 // 8KB for external RAM
#define OAM_SIZE 0xA0        // 160 bytes for Sprite OAM
#define HRAM_SIZE 0x7F       // 127 bytes for HRAM
#define IORAM_SIZE 0x80      // 128 bytes for I/O Registers

// CPU structure with registers and flags
struct CPU {
    uint8_t A, F; // Accumulator & Flags
    uint8_t B, C; // General-purpose
    uint8_t D, E; // General-purpose
    uint8_t H, L; // General-purpose
    uint16_t PC;  // Program Counter
    uint16_t SP;  // Stack Pointer
    bool halt;    // Halt state

    CPU() : A(0x01), F(0xB0), B(0x00), C(0x13), D(0x00), E(0xD8), H(0x01), L(0x4D), PC(0x0100), SP(0xFFFE), halt(false) {}
};

// Memory bank information (MBC1)
struct MBC1 {
    uint8_t ram_enable;   // RAM enable flag
    uint8_t ram_bank;     // Selected RAM bank (MBC1 only)
    uint8_t rom_bank_low; // Lower 5 bits of the selected ROM bank
    uint8_t rom_bank_high; // Upper 2 bits of the selected ROM bank

    MBC1() : ram_enable(0), ram_bank(0), rom_bank_low(1), rom_bank_high(0) {}
};

// Global objects
std::vector<uint8_t> memory(MEMORY_SIZE); // Game Boy memory
CPU cpu; // CPU instance
MBC1 mbc1; // MBC1 controller instance
std::vector<std::vector<uint8_t>> rom_banks(2); // ROM bank storage
uint32_t cpu_cycles = 0; // Variable to track CPU cycles

// Function to initialize MBC1 memory controller
void initialize_mbc1() {
    mbc1.ram_enable = 0;
    mbc1.ram_bank = 0;
    mbc1.rom_bank_low = 1; // Start with ROM bank 1
    mbc1.rom_bank_high = 0; // Start with ROM bank high bits 0
}

// Function to load ROM into memory
int load_rom(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        std::cerr << "Error: Could not open ROM file " << filename << std::endl;
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long rom_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Calculate total ROM banks and load ROM data
    int total_rom_banks = (rom_size + ROM_BANK_SIZE - 1) / ROM_BANK_SIZE;
    rom_banks.resize(total_rom_banks);

    for (int i = 0; i < total_rom_banks; i++) {
        rom_banks[i].resize(ROM_BANK_SIZE);
        fread(rom_banks[i].data(), 1, ROM_BANK_SIZE, file);
    }
    fclose(file);

    std::cout << "ROM loaded successfully. Size: " << rom_size << " bytes" << std::endl;
    return 0;
}

// Memory read function considering MBC1
uint8_t read_mbc1_memory(uint16_t address) {
    if (address < 0x4000) {
        return rom_banks[0][address]; // Fixed ROM bank (Bank 0)
    } else if (address < 0x8000) {
        uint32_t rom_bank_address = (mbc1.rom_bank_high << 14) | (mbc1.rom_bank_low << 13) | (address - 0x4000);
        return rom_banks[1][rom_bank_address % ROM_BANK_SIZE]; // Switchable ROM bank
    }

    // Handle RAM (0xA000-0xBFFF)
    if (mbc1.ram_enable && address >= 0xA000 && address < 0xC000) {
        uint32_t ram_bank_address = (mbc1.ram_bank * EXTERNAL_RAM_SIZE) + (address - 0xA000);
        return memory[ram_bank_address];
    }

    return 0xFF; // Unmapped memory region
}

// Memory write function considering MBC1
void write_mbc1_memory(uint16_t address, uint8_t value) {
    if (address < 0x2000) {
        mbc1.ram_enable = (value & 0x0A) == 0x0A ? 1 : 0;
    } else if (address < 0x4000) {
        mbc1.rom_bank_low = value & 0x1F;
    } else if (address < 0x6000) {
        mbc1.rom_bank_high = value & 0x03;
    } else if (address < 0x8000) {
        mbc1.ram_bank = value & 0x03; // Select RAM bank (if applicable)
    }
}

// Memory read function considering memory mapping
uint8_t read_memory(uint16_t address) {
    return read_mbc1_memory(address); // Handle MBC1 memory reading
}

// Memory write function considering memory mapping
void write_memory(uint16_t address, uint8_t value) {
    write_mbc1_memory(address, value); // Handle MBC1 memory writing
}

// Fetch the next opcode from memory
uint8_t fetch_opcode() {
    return read_memory(cpu.PC++); // Increment PC after fetching the opcode
}

// Test CPU loop to fetch and decode opcodes
void cpu_test_loop() {
    while (cpu.PC < MEMORY_SIZE && !cpu.halt) {
        uint8_t opcode = fetch_opcode();
        printf("PC: 0x%04X, Opcode: 0x%02X\n", cpu.PC - 1, opcode);

        switch (opcode) {
            case 0x00: // NOP (No operation)
                cpu_cycles += 4;
                break;

            case 0x06: // LD B, n (Load immediate value into B)
                cpu.B = read_memory(cpu.PC++);
                cpu_cycles += 8;
                break;

            // Add other opcodes here as needed...
            
            default:
                std::cout << "Unhandled opcode: 0x" << std::hex << (int)opcode << std::dec << std::endl;
                break;
        }
    }
}

// Main function to run the emulator
int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <romfile>" << std::endl;
        return EXIT_FAILURE;
    }

    if (load_rom(argv[1]) != 0) {
        return EXIT_FAILURE;
    }

    initialize_mbc1(); // Initialize MBC1 memory controller
    cpu_test_loop(); // Start the CPU test loop

    // Cleanup is handled automatically by std::vector
    return EXIT_SUCCESS;
}
