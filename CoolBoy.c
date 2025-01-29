#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <math.h>
#include <stdint.h>
#include <memory.h>

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144
#define SCALE 4
#define MEMORY_SIZE 0x10000
#define ROM_SIZE 0x20000 // Increased to 128KB for larger ROMs (0x20000 bytes)
#define MAX_ROM_BANKS 128
#define MAX_RAM_BANKS 4

// CPU structure
struct CPU {
    uint8_t memory[MEMORY_SIZE]; // 64KB memory space (Game Boy)
    uint8_t rom[ROM_SIZE];       // Increased ROM space
    uint16_t pc;                 // Program Counter
    uint16_t sp;                 // Stack Pointer
    uint8_t a, b, c, d, e, h, l; // Registers
    uint8_t zf, nf, hf, cf;      // Flags
    uint8_t ie;                  // Interrupt Enable Register
    uint8_t iflags;              // Interrupt Flags Register
    uint8_t selected_bank;       // Currently selected ROM bank
    uint8_t selected_ram_bank;   // Currently selected RAM bank
};

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

// Function to load the ROM into memory
int loadROM(struct CPU *cpu, const char *filename) {
    printf("Loading ROM: %s\n", filename);
    
    FILE *rom = fopen(filename, "rb");
    if (!rom) {
        printf("Failed to open ROM file: %s\n", filename);
        return 0; // Failed to open ROM
    }

    // Check the file size
    fseek(rom, 0, SEEK_END);
    long fileSize = ftell(rom);
    fseek(rom, 0, SEEK_SET);

    printf("ROM size: %ld bytes\n", fileSize);

    if (fileSize > ROM_SIZE) {
        printf("ROM size exceeds allocated memory\n");
        fclose(rom);
        return 0; // ROM size too big
    }

    // Read ROM into memory
    size_t bytesRead = fread(cpu->rom, 1, fileSize, rom);
    if (bytesRead != fileSize) {
        printf("Failed to read ROM file\n");
        fclose(rom);
        return 0; // Failed to read ROM
    }

    fclose(rom);
    printf("ROM loaded successfully\n");
    return 1; // Successfully loaded ROM
}

// Function to read and print the ROM header
void readROMHeader(struct CPU *cpu) {
    printf("Reading ROM header\n");
    
    struct ROMHeader header;
    
    // Copy ROM header data from memory
    memcpy(header.title, &cpu->rom[0x0134], 16);
    header.cartridge_type = cpu->rom[0x0147];
    header.rom_size = cpu->rom[0x0148];
    header.ram_size = cpu->rom[0x0149];
    header.destination_code = cpu->rom[0x014A];
    header.old_licensee_code = cpu->rom[0x014B];
    header.mask_rom_version = cpu->rom[0x014C];
    header.header_checksum = cpu->rom[0x014D];
    header.global_checksum[0] = cpu->rom[0x014E];
    header.global_checksum[1] = cpu->rom[0x014F];

    // Print ROM header information
    printf("Title: %.16s\n", header.title);
    printf("Cartridge Type: 0x%02X\n", header.cartridge_type);
    printf("ROM Size: 0x%02X\n", header.rom_size);
    printf("RAM Size: 0x%02X\n", header.ram_size);
    printf("Destination Code: 0x%02X\n", header.destination_code);
    printf("Old Licensee Code: 0x%02X\n", header.old_licensee_code);
    printf("Mask ROM Version: 0x%02X\n", header.mask_rom_version);
    printf("Header Checksum: 0x%02X\n", header.header_checksum);
    printf("Global Checksum: 0x%02X 0x%02X\n", header.global_checksum[0], header.global_checksum[1]);
}

// Initialize the CPU
void initializeCPU(struct CPU *cpu) {
    printf("Initializing CPU\n");

    cpu->pc = 0x0100; // Start address after boot ROM
    cpu->sp = 0xFFFE; // Initialize stack pointer
    
    // Initialize registers
    cpu->a = 0x01; // Accumulator
    cpu->b = 0x00; // Register B
    cpu->c = 0x13; // Register C
    cpu->d = 0x00; // Register D
    cpu->e = 0xD8; // Register E
    cpu->h = 0x01; // Register H
    cpu->l = 0x4D; // Register L

    // Initialize flags
    cpu->zf = 1;  // Zero Flag
    cpu->nf = 0;  // Subtract Flag
    cpu->hf = 1;  // Half Carry Flag
    cpu->cf = 1;  // Carry Flag

    // Initialize other special registers
    cpu->ie = 0x00;         // Interrupt Enable Register
    cpu->iflags = 0x00;     // Interrupt Flags Register
    cpu->selected_bank = 0; // Selected ROM bank
    cpu->selected_ram_bank = 0; // Selected RAM bank

    // Zero out memory (optional, but good practice)
    for (int i = 0; i < MEMORY_SIZE; i++) {
        cpu->memory[i] = 0x00;
    }

    printf("CPU initialized\n");
}

// Fetch, decode, and execute loop
// Fetch, decode, and execute loop
// Fetch, decode, and execute loop
void emulateCycle(struct CPU *cpu) {
    uint8_t opcode = cpu->memory[cpu->pc];
    printf("PC: 0x%04X, Opcode: 0x%02X\n", cpu->pc, opcode);

    switch (opcode) {
        // 8-Bit Loads
        case 0x06: // LD B,n
            printf("LD B, 0x%02X\n", cpu->memory[cpu->pc + 1]);
            cpu->b = cpu->memory[cpu->pc + 1];
            cpu->pc += 2;
            break;

        case 0x0E: // LD C,n
            printf("LD C, 0x%02X\n", cpu->memory[cpu->pc + 1]);
            cpu->c = cpu->memory[cpu->pc + 1];
            cpu->pc += 2;
            break;

        case 0x16: // LD D,n
            printf("LD D, 0x%02X\n", cpu->memory[cpu->pc + 1]);
            cpu->d = cpu->memory[cpu->pc + 1];
            cpu->pc += 2;
            break;

        case 0x1E: // LD E,n
            printf("LD E, 0x%02X\n", cpu->memory[cpu->pc + 1]);
            cpu->e = cpu->memory[cpu->pc + 1];
            cpu->pc += 2;
            break;

        case 0x26: // LD H,n
            printf("LD H, 0x%02X\n", cpu->memory[cpu->pc + 1]);
            cpu->h = cpu->memory[cpu->pc + 1];
            cpu->pc += 2;
            break;

        case 0x2E: // LD L,n
            printf("LD L, 0x%02X\n", cpu->memory[cpu->pc + 1]);
            cpu->l = cpu->memory[cpu->pc + 1];
            cpu->pc += 2;
            break;

        case 0x7F: // LD A,A
            printf("LD A, A\n");
            cpu->a = cpu->a;
            cpu->pc++;
            break;

        case 0x78: // LD A,B
            printf("LD A, B\n");
            cpu->a = cpu->b;
            cpu->pc++;
            break;

        case 0x79: // LD A,C
            printf("LD A, C\n");
            cpu->a = cpu->c;
            cpu->pc++;
            break;

        case 0x7A: // LD A,D
            printf("LD A, D\n");
            cpu->a = cpu->d;
            cpu->pc++;
            break;

        case 0x7B: // LD A,E
            printf("LD A, E\n");
            cpu->a = cpu->e;
            cpu->pc++;
            break;

        case 0x7C: // LD A,H
            printf("LD A, H\n");
            cpu->a = cpu->h;
            cpu->pc++;
            break;

        case 0x7D: // LD A,L
            printf("LD A, L\n");
            cpu->a = cpu->l;
            cpu->pc++;
            break;

        case 0x7E: // LD A,(HL)
            printf("LD A, (HL)\n");
            cpu->a = cpu->memory[(cpu->h << 8) | cpu->l];
            cpu->pc++;
            break;

        case 0x40: // LD B,B
            printf("LD B, B\n");
            cpu->b = cpu->b;
            cpu->pc++;
            break;

        case 0x41: // LD B,C
            printf("LD B, C\n");
            cpu->b = cpu->c;
            cpu->pc++;
            break;

        case 0x42: // LD B,D
            printf("LD B, D\n");
            cpu->b = cpu->d;
            cpu->pc++;
            break;

        case 0x43: // LD B,E
            printf("LD B, E\n");
            cpu->b = cpu->e;
            cpu->pc++;
            break;

        case 0x44: // LD B,H
            printf("LD B, H\n");
            cpu->b = cpu->h;
            cpu->pc++;
            break;

        case 0x45: // LD B,L
            printf("LD B, L\n");
            cpu->b = cpu->l;
            cpu->pc++;
            break;

        case 0x46: // LD B,(HL)
            printf("LD B, (HL)\n");
            cpu->b = cpu->memory[(cpu->h << 8) | cpu->l];
            cpu->pc++;
            break;

        case 0x48: // LD C,B
            printf("LD C, B\n");
            cpu->c = cpu->b;
            cpu->pc++;
            break;

        case 0x49: // LD C,C
            printf("LD C, C\n");
            cpu->c = cpu->c;
            cpu->pc++;
            break;

        case 0x4A: // LD C,D
            printf("LD C, D\n");
            cpu->c = cpu->d;
            cpu->pc++;
            break;

        case 0x4B: // LD C,E
            printf("LD C, E\n");
            cpu->c = cpu->e;
            cpu->pc++;
            break;

        case 0x4C: // LD C,H
            printf("LD C, H\n");
            cpu->c = cpu->h;
            cpu->pc++;
            break;

        case 0x4D: // LD C,L
            printf("LD C, L\n");
            cpu->c = cpu->l;
            cpu->pc++;
            break;

        case 0x4E: // LD C,(HL)
            printf("LD C, (HL)\n");
            cpu->c = cpu->memory[(cpu->h << 8) | cpu->l];
            cpu->pc++;
            break;

        case 0x50: // LD D,B
            printf("LD D, B\n");
            cpu->d = cpu->b;
            cpu->pc++;
            break;

        case 0x51: // LD D,C
            printf("LD D, C\n");
            cpu->d = cpu->c;
            cpu->pc++;
            break;

        case 0x52: // LD D,D
            printf("LD D, D\n");
            cpu->d = cpu->d;
            cpu->pc++;
            break;

        case 0x53: // LD D,E
            printf("LD D, E\n");
            cpu->d = cpu->e;
            cpu->pc++;
            break;

        case 0x54: // LD D,H
            printf("LD D, H\n");
            cpu->d = cpu->h;
            cpu->pc++;
            break;

        case 0x55: // LD D,L
            printf("LD D, L\n");
            cpu->d = cpu->l;
            cpu->pc++;
            break;

        case 0x56: // LD D,(HL)
            printf("LD D, (HL)\n");
            cpu->d = cpu->memory[(cpu->h << 8) | cpu->l];
            cpu->pc++;
            break;

        case 0x58: // LD E,B
            printf("LD E, B\n");
            cpu->e = cpu->b;
            cpu->pc++;
            break;

        case 0x59: // LD E,C
            printf("LD E, C\n");
            cpu->e = cpu->c;
            cpu->pc++;
            break;

        case 0x5A: // LD E,D
            printf("LD E, D\n");
            cpu->e = cpu->d;
            cpu->pc++;
            break;

        case 0x5B: // LD E,E
            printf("LD E, E\n");
            cpu->e = cpu->e;
            cpu->pc++;
            break;

        case 0x5C: // LD E,H
            printf("LD E, H\n");
            cpu->e = cpu->h;
            cpu->pc++;
            break;

        case 0x5D: // LD E,L
            printf("LD E, L\n");
            cpu->e = cpu->l;
            cpu->pc++;
            break;

        case 0x5E: // LD E,(HL)
            printf("LD E, (HL)\n");
            cpu->e = cpu->memory[(cpu->h << 8) | cpu->l];
            cpu->pc++;
            break;

                case 0x60: // LD H,B
            printf("LD H, B\n");
            cpu->h = cpu->b;
            cpu->pc++;
            break;

        case 0x61: // LD H,C
            printf("LD H, C\n");
            cpu->h = cpu->c;
            cpu->pc++;
            break;

        case 0x62: // LD H,D
            printf("LD H, D\n");
            cpu->h = cpu->d;
            cpu->pc++;
            break;

        case 0x63: // LD H,E
            printf("LD H, E\n");
            cpu->h = cpu->e;
            cpu->pc++;
            break;

        case 0x64: // LD H,H
            printf("LD H, H\n");
            cpu->h = cpu->h;
            cpu->pc++;
            break;

        case 0x65: // LD H,L
            printf("LD H, L\n");
            cpu->h = cpu->l;
            cpu->pc++;
            break;

        case 0x66: // LD H,(HL)
            printf("LD H, (HL)\n");
            cpu->h = cpu->memory[(cpu->h << 8) | cpu->l];
            cpu->pc++;
            break;

        case 0x68: // LD L,B
            printf("LD L, B\n");
            cpu->l = cpu->b;
            cpu->pc++;
            break;

        case 0x69: // LD L,C
            printf("LD L, C\n");
            cpu->l = cpu->c;
            cpu->pc++;
            break;

        case 0x6A: // LD L,D
            printf("LD L, D\n");
            cpu->l = cpu->d;
            cpu->pc++;
            break;

        case 0x6B: // LD L,E
            printf("LD L, E\n");
            cpu->l = cpu->e;
            cpu->pc++;
            break;

        case 0x6C: // LD L,H
            printf("LD L, H\n");
            cpu->l = cpu->h;
            cpu->pc++;
            break;

        case 0x6D: // LD L,L
            printf("LD L, L\n");
            cpu->l = cpu->l;
            cpu->pc++;
            break;

        case 0x6E: // LD L,(HL)
            printf("LD L, (HL)\n");
            cpu->l = cpu->memory[(cpu->h << 8) | cpu->l];
            cpu->pc++;
            break;

        case 0x70: // LD (HL),B
            printf("LD (HL), B\n");
            cpu->memory[(cpu->h << 8) | cpu->l] = cpu->b;
            cpu->pc++;
            break;

        case 0x71: // LD (HL),C
            printf("LD (HL), C\n");
            cpu->memory[(cpu->h << 8) | cpu->l] = cpu->c;
            cpu->pc++;
            break;

        case 0x72: // LD (HL),D
            printf("LD (HL), D\n");
            cpu->memory[(cpu->h << 8) | cpu->l] = cpu->d;
            cpu->pc++;
            break;

        case 0x73: // LD (HL),E
            printf("LD (HL), E\n");
            cpu->memory[(cpu->h << 8) | cpu->l] = cpu->e;
            cpu->pc++;
            break;

        case 0x74: // LD (HL),H
            printf("LD (HL), H\n");
            cpu->memory[(cpu->h << 8) | cpu->l] = cpu->h;
            cpu->pc++;
            break;

        case 0x75: // LD (HL),L
            printf("LD (HL), L\n");
            cpu->memory[(cpu->h << 8) | cpu->l] = cpu->l;
            cpu->pc++;
            break;

        case 0x36: // LD (HL),n
            printf("LD (HL), 0x%02X\n", cpu->memory[cpu->pc + 1]);
            cpu->memory[(cpu->h << 8) | cpu->l] = cpu->memory[cpu->pc + 1];
            cpu->pc += 2;
            break;

        case 0x0A: // LD A,(BC)
            printf("LD A, (BC)\n");
            cpu->a = cpu->memory[(cpu->b << 8) | cpu->c];
            cpu->pc++;
            break;

        case 0x1A: // LD A,(DE)
            printf("LD A, (DE)\n");
            cpu->a = cpu->memory[(cpu->d << 8) | cpu->e];
            cpu->pc++;
            break;

        case 0xFA: // LD A,(nn)
            printf("LD A, (0x%04X)\n", (cpu->memory[cpu->pc + 2] << 8) | cpu->memory[cpu->pc + 1]);
            cpu->a = cpu->memory[(cpu->memory[cpu->pc + 2] << 8) | cpu->memory[cpu->pc + 1]];
            cpu->pc += 3;
            break;

        case 0x3E: // LD A,n
            printf("LD A, 0x%02X\n", cpu->memory[cpu->pc + 1]);
            cpu->a = cpu->memory[cpu->pc + 1];
            cpu->pc += 2;
            break;

        case 0x47: // LD B,A
            printf("LD B, A\n");
            cpu->b = cpu->a;
            cpu->pc++;
            break;

        case 0x4F: // LD C,A
            printf("LD C, A\n");
            cpu->c = cpu->a;
            cpu->pc++;
            break;

        case 0x57: // LD D,A
            printf("LD D, A\n");
            cpu->d = cpu->a;
            cpu->pc++;
            break;

        case 0x5F: // LD E,A
            printf("LD E, A\n");
            cpu->e = cpu->a;
            cpu->pc++;
            break;

        case 0x67: // LD H,A
            printf("LD H, A\n");
            cpu->h = cpu->a;
            cpu->pc++;
            break;

        case 0x6F: // LD L,A
            printf("LD L, A\n");
            cpu->l = cpu->a;
            cpu->pc++;
            break;

        case 0x02: // LD (BC),A
            printf("LD (BC), A\n");
            cpu->memory[(cpu->b << 8) | cpu->c] = cpu->a;
            cpu->pc++;
            break;

        case 0x12: // LD (DE),A
            printf("LD (DE), A\n");
            cpu->memory[(cpu->d << 8) | cpu->e] = cpu->a;
            cpu->pc++;
            break;

        case 0x77: // LD (HL),A
            printf("LD (HL), A\n");
            cpu->memory[(cpu->h << 8) | cpu->l] = cpu->a;
            cpu->pc++;
            break;

        case 0xEA: // LD (nn),A
            printf("LD (0x%04X), A\n", (cpu->memory[cpu->pc + 2] << 8) | cpu->memory[cpu->pc + 1]);
            cpu->memory[(cpu->memory[cpu->pc + 2] << 8) | cpu->memory[cpu->pc + 1]] = cpu->a;
            cpu->pc += 3;
            break;

        case 0xF2: // LD A,(C)
            printf("LD A, ($FF00+C)\n");
            cpu->a = cpu->memory[0xFF00 + cpu->c];
            cpu->pc++;
            break;

        case 0xE2: // LD (C),A
            printf("LD ($FF00+C), A\n");
            cpu->memory[0xFF00 + cpu->c] = cpu->a;
            cpu->pc++;
            break;

        case 0x3A: // LDD A,(HL)
            printf("LDD A, (HL)\n");
            cpu->a = cpu->memory[(cpu->h << 8) | cpu->l];
            uint16_t hl = ((cpu->h << 8) | cpu->l) - 1;
            cpu->h = hl >> 8;
            cpu->l = hl & 0xFF;
            cpu->pc++;
            break;

        case 0x32: // LDD (HL),A
            printf("LDD (HL), A\n");
            cpu->memory[(cpu->h << 8) | cpu->l] = cpu->a;
            hl = ((cpu->h << 8) | cpu->l) - 1;
            cpu->h = hl >> 8;
            cpu->l = hl & 0xFF;
            cpu->pc++;
            break;

        case 0x2A: // LDI A,(HL)
            printf("LDI A, (HL)\n");
            cpu->a = cpu->memory[(cpu->h << 8) | cpu->l];
            hl = ((cpu->h << 8) | cpu->l) + 1;
            cpu->h = hl >> 8;
            cpu->l = hl & 0xFF;
            cpu->pc++;
            break;

        case 0x22: // LDI (HL),A
            printf("LDI (HL), A\n");
            cpu->memory[(cpu->h << 8) | cpu->l] = cpu->a;
            hl = ((cpu->h << 8) | cpu->l) + 1;
            cpu->h = hl >> 8;
            cpu->l = hl & 0xFF;
            cpu->pc++;
            break;

        case 0xE0: // LDH (n),A
            printf("LDH (0x%02X), A\n", cpu->memory[cpu->pc + 1]);
            cpu->memory[0xFF00 + cpu->memory[cpu->pc + 1]] = cpu->a;
            cpu->pc += 2;
            break;

        case 0xF0: // LDH A,(n)
            printf("LDH A, (0x%02X)\n", cpu->memory[cpu->pc + 1]);
            cpu->a = cpu->memory[0xFF00 + cpu->memory[cpu->pc + 1]];
            cpu->pc += 2;
            break;


       // 16-Bit Loads
        case 0x01: // LD BC,nn
            printf("LD BC, 0x%04X\n", (cpu->memory[cpu->pc + 2] << 8) | cpu->memory[cpu->pc + 1]);
            cpu->c = cpu->memory[cpu->pc + 1];
            cpu->b = cpu->memory[cpu->pc + 2];
            cpu->pc += 3;
            break;

        case 0x11: // LD DE,nn
            printf("LD DE, 0x%04X\n", (cpu->memory[cpu->pc + 2] << 8) | cpu->memory[cpu->pc + 1]);
            cpu->e = cpu->memory[cpu->pc + 1];
            cpu->d = cpu->memory[cpu->pc + 2];
            cpu->pc += 3;
            break;

        case 0x21: // LD HL,nn
            printf("LD HL, 0x%04X\n", (cpu->memory[cpu->pc + 2] << 8) | cpu->memory[cpu->pc + 1]);
            cpu->l = cpu->memory[cpu->pc + 1];
            cpu->h = cpu->memory[cpu->pc + 2];
            cpu->pc += 3;
            break;

        case 0x31: // LD SP,nn
            printf("LD SP, 0x%04X\n", (cpu->memory[cpu->pc + 2] << 8) | cpu->memory[cpu->pc + 1]);
            cpu->sp = (cpu->memory[cpu->pc + 2] << 8) | cpu->memory[cpu->pc + 1];
            cpu->pc += 3;
            break;

        case 0xF9: // LD SP,HL
            printf("LD SP, HL\n");
            cpu->sp = (cpu->h << 8) | cpu->l;
            cpu->pc++;
            break;

        case 0xF8: // LD HL,SP+e
            printf("LD HL, SP+0x%02X\n", (int8_t)cpu->memory[cpu->pc + 1]);
            {
                int8_t offset = (int8_t)cpu->memory[cpu->pc + 1];
                uint16_t result = cpu->sp + offset;
                cpu->h = result >> 8;
                cpu->l = result & 0xFF;
                cpu->zf = 0;
                cpu->nf = 0;
                cpu->hf = ((cpu->sp & 0x0F) + (offset & 0x0F)) > 0x0F;
                cpu->cf = ((cpu->sp & 0xFF) + (uint8_t)offset) > 0xFF;
                cpu->pc += 2;
            }
            break;

        case 0x08: // LD (nn),SP
            printf("LD (0x%04X), SP\n", (cpu->memory[cpu->pc + 2] << 8) | cpu->memory[cpu->pc + 1]);
            {
                uint16_t address = (cpu->memory[cpu->pc + 2] << 8) | cpu->memory[cpu->pc + 1];
                cpu->memory[address] = cpu->sp & 0xFF;
                cpu->memory[address + 1] = cpu->sp >> 8;
                cpu->pc += 3;
            }
            break;

        case 0xC5: // PUSH BC
            printf("PUSH BC\n");
            cpu->memory[--cpu->sp] = cpu->b;
            cpu->memory[--cpu->sp] = cpu->c;
            cpu->pc++;
            break;

        case 0xD5: // PUSH DE
            printf("PUSH DE\n");
            cpu->memory[--cpu->sp] = cpu->d;
            cpu->memory[--cpu->sp] = cpu->e;
            cpu->pc++;
            break;

        case 0xE5: // PUSH HL
            printf("PUSH HL\n");
            cpu->memory[--cpu->sp] = cpu->h;
            cpu->memory[--cpu->sp] = cpu->l;
            cpu->pc++;
            break;

        case 0xF5: // PUSH AF
            printf("PUSH AF\n");
            cpu->memory[--cpu->sp] = cpu->a;
            cpu->memory[--cpu->sp] = cpu->zf << 7 | cpu->nf << 6 | cpu->hf << 5 | cpu->cf << 4;
            cpu->pc++;
            break;

        case 0xC1: // POP BC
            printf("POP BC\n");
            cpu->c = cpu->memory[cpu->sp++];
            cpu->b = cpu->memory[cpu->sp++];
            cpu->pc++;
            break;

        case 0xD1: // POP DE
            printf("POP DE\n");
            cpu->e = cpu->memory[cpu->sp++];
            cpu->d = cpu->memory[cpu->sp++];
            cpu->pc++;
            break;

        case 0xE1: // POP HL
            printf("POP HL\n");
            cpu->l = cpu->memory[cpu->sp++];
            cpu->h = cpu->memory[cpu->sp++];
            cpu->pc++;
            break;

        case 0xF1: // POP AF
            printf("POP AF\n");
            {
                uint8_t flags = cpu->memory[cpu->sp++];
                cpu->a = cpu->memory[cpu->sp++];
                cpu->zf = (flags >> 7) & 1;
                cpu->nf = (flags >> 6) & 1;
                cpu->hf = (flags >> 5) & 1;
                cpu->cf = (flags >> 4) & 1;
                cpu->pc++;
            }
            break;

                // 8-Bit ALU Operations
        case 0x87: // ADD A, A
            printf("ADD A, A\n");
            cpu->a = cpu->a + cpu->a;
            cpu->pc++;
            break;

        case 0x80: // ADD A, B
            printf("ADD A, B\n");
            cpu->a = cpu->a + cpu->b;
            cpu->pc++;
            break;

        case 0x81: // ADD A, C
            printf("ADD A, C\n");
            cpu->a = cpu->a + cpu->c;
            cpu->pc++;
            break;

        case 0x82: // ADD A, D
            printf("ADD A, D\n");
            cpu->a = cpu->a + cpu->d;
            cpu->pc++;
            break;

        case 0x83: // ADD A, E
            printf("ADD A, E\n");
            cpu->a = cpu->a + cpu->e;
            cpu->pc++;
            break;

        case 0x84: // ADD A, H
            printf("ADD A, H\n");
            cpu->a = cpu->a + cpu->h;
            cpu->pc++;
            break;

        case 0x85: // ADD A, L
            printf("ADD A, L\n");
            cpu->a = cpu->a + cpu->l;
            cpu->pc++;
            break;

        case 0x86: // ADD A, (HL)
            printf("ADD A, (HL)\n");
            cpu->a = cpu->a + cpu->memory[(cpu->h << 8) | cpu->l];
            cpu->pc++;
            break;

        case 0xC6: // ADD A, n
            printf("ADD A, 0x%02X\n", cpu->memory[cpu->pc + 1]);
            cpu->a = cpu->a + cpu->memory[cpu->pc + 1];
            cpu->pc += 2;
            break;

        case 0x8F: // ADC A, A
            printf("ADC A, A\n");
            cpu->a = cpu->a + cpu->a + cpu->cf;
            cpu->pc++;
            break;

        case 0x88: // ADC A, B
            printf("ADC A, B\n");
            cpu->a = cpu->a + cpu->b + cpu->cf;
            cpu->pc++;
            break;

        case 0x89: // ADC A, C
            printf("ADC A, C\n");
            cpu->a = cpu->a + cpu->c + cpu->cf;
            cpu->pc++;
            break;

        case 0x8A: // ADC A, D
            printf("ADC A, D\n");
            cpu->a = cpu->a + cpu->d + cpu->cf;
            cpu->pc++;
            break;

        case 0x8B: // ADC A, E
            printf("ADC A, E\n");
            cpu->a = cpu->a + cpu->e + cpu->cf;
            cpu->pc++;
            break;

        case 0x8C: // ADC A, H
            printf("ADC A, H\n");
            cpu->a = cpu->a + cpu->h + cpu->cf;
            cpu->pc++;
            break;

        case 0x8D: // ADC A, L
            printf("ADC A, L\n");
            cpu->a = cpu->a + cpu->l + cpu->cf;
            cpu->pc++;
            break;

        case 0x8E: // ADC A, (HL)
            printf("ADC A, (HL)\n");
            cpu->a = cpu->a + cpu->memory[(cpu->h << 8) | cpu->l] + cpu->cf;
            cpu->pc++;
            break;

        case 0xCE: // ADC A, n
            printf("ADC A, 0x%02X\n", cpu->memory[cpu->pc + 1]);
            cpu->a = cpu->a + cpu->memory[cpu->pc + 1] + cpu->cf;
            cpu->pc += 2;
            break;

        case 0x97: // SUB A
            printf("SUB A, A\n");
            cpu->a = cpu->a - cpu->a;
            cpu->pc++;
            break;

        case 0x90: // SUB B
            printf("SUB A, B\n");
            cpu->a = cpu->a - cpu->b;
            cpu->pc++;
            break;

        case 0x91: // SUB C
            printf("SUB A, C\n");
            cpu->a = cpu->a - cpu->c;
            cpu->pc++;
            break;

        case 0x92: // SUB D
            printf("SUB A, D\n");
            cpu->a = cpu->a - cpu->d;
            cpu->pc++;
            break;

        case 0x93: // SUB E
            printf("SUB A, E\n");
            cpu->a = cpu->a - cpu->e;
            cpu->pc++;
            break;

        case 0x94: // SUB H
            printf("SUB A, H\n");
            cpu->a = cpu->a - cpu->h;
            cpu->pc++;
            break;

        case 0x95: // SUB L
            printf("SUB A, L\n");
            cpu->a = cpu->a - cpu->l;
            cpu->pc++;
            break;

        case 0x96: // SUB (HL)
            printf("SUB A, (HL)\n");
            cpu->a = cpu->a - cpu->memory[(cpu->h << 8) | cpu->l];
            cpu->pc++;
            break;

        case 0xD6: // SUB n
            printf("SUB A, 0x%02X\n", cpu->memory[cpu->pc + 1]);
            cpu->a = cpu->a - cpu->memory[cpu->pc + 1];
            cpu->pc += 2;
            break;

        case 0x9F: // SBC A, A
            printf("SBC A, A\n");
            cpu->a = cpu->a - cpu->a - cpu->cf;
            cpu->pc++;
            break;

        case 0x98: // SBC A, B
            printf("SBC A, B\n");
            cpu->a = cpu->a - cpu->b - cpu->cf;
            cpu->pc++;
            break;

        case 0x99: // SBC A, C
            printf("SBC A, C\n");
            cpu->a = cpu->a - cpu->c - cpu->cf;
            cpu->pc++;
            break;

        case 0x9A: // SBC A, D
            printf("SBC A, D\n");
            cpu->a = cpu->a - cpu->d - cpu->cf;
            cpu->pc++;
            break;

        case 0x9B: // SBC A, E
            printf("SBC A, E\n");
            cpu->a = cpu->a - cpu->e - cpu->cf;
            cpu->pc++;
            break;

        case 0x9C: // SBC A, H
            printf("SBC A, H\n");
            cpu->a = cpu->a - cpu->h - cpu->cf;
            cpu->pc++;
            break;

        case 0x9D: // SBC A, L
            printf("SBC A, L\n");
            cpu->a = cpu->a - cpu->l - cpu->cf;
            cpu->pc++;
            break;

        case 0x9E: // SBC A, (HL)
            printf("SBC A, (HL)\n");
            cpu->a = cpu->a - cpu->memory[(cpu->h << 8) | cpu->l] - cpu->cf;
            cpu->pc++;
            break;

        case 0xDE: // SBC A, n
            printf("SBC A, 0x%02X\n", cpu->memory[cpu->pc + 1]);
            cpu->a = cpu->a - cpu->memory[cpu->pc + 1] - cpu->cf;
            cpu->pc += 2;
            break;

        case 0xA7: // AND A
            printf("AND A\n");
            cpu->a = cpu->a & cpu->a;
            cpu->pc++;
            break;

                case 0xA0: // AND B
            printf("AND A, B\n");
            cpu->a = cpu->a & cpu->b;
            cpu->pc++;
            break;

        case 0xA1: // AND C
            printf("AND A, C\n");
            cpu->a = cpu->a & cpu->c;
            cpu->pc++;
            break;

        case 0xA2: // AND D
            printf("AND A, D\n");
            cpu->a = cpu->a & cpu->d;
            cpu->pc++;
            break;

        case 0xA3: // AND E
            printf("AND A, E\n");
            cpu->a = cpu->a & cpu->e;
            cpu->pc++;
            break;

        case 0xA4: // AND H
            printf("AND A, H\n");
            cpu->a = cpu->a & cpu->h;
            cpu->pc++;
            break;

        case 0xA5: // AND L
            printf("AND A, L\n");
            cpu->a = cpu->a & cpu->l;
            cpu->pc++;
            break;

        case 0xA6: // AND (HL)
            printf("AND A, (HL)\n");
            cpu->a = cpu->a & cpu->memory[(cpu->h << 8) | cpu->l];
            cpu->pc++;
            break;

        case 0xE6: // AND n
            printf("AND A, 0x%02X\n", cpu->memory[cpu->pc + 1]);
            cpu->a = cpu->a & cpu->memory[cpu->pc + 1];
            cpu->pc += 2;
            break;

        case 0xB7: // OR A
            printf("OR A, A\n");
            cpu->a = cpu->a | cpu->a;
            cpu->pc++;
            break;

        case 0xB0: // OR B
            printf("OR A, B\n");
            cpu->a = cpu->a | cpu->b;
            cpu->pc++;
            break;

        case 0xB1: // OR C
            printf("OR A, C\n");
            cpu->a = cpu->a | cpu->c;
            cpu->pc++;
            break;

        case 0xB2: // OR D
            printf("OR A, D\n");
            cpu->a = cpu->a | cpu->d;
            cpu->pc++;
            break;

        case 0xB3: // OR E
            printf("OR A, E\n");
            cpu->a = cpu->a | cpu->e;
            cpu->pc++;
            break;

        case 0xB4: // OR H
            printf("OR A, H\n");
            cpu->a = cpu->a | cpu->h;
            cpu->pc++;
            break;

        case 0xB5: // OR L
            printf("OR A, L\n");
            cpu->a = cpu->a | cpu->l;
            cpu->pc++;
            break;

        case 0xB6: // OR (HL)
            printf("OR A, (HL)\n");
            cpu->a = cpu->a | cpu->memory[(cpu->h << 8) | cpu->l];
            cpu->pc++;
            break;

        case 0xF6: // OR n
            printf("OR A, 0x%02X\n", cpu->memory[cpu->pc + 1]);
            cpu->a = cpu->a | cpu->memory[cpu->pc + 1];
            cpu->pc += 2;
            break;

        case 0xAF: // XOR A
            printf("XOR A\n");
            cpu->a = cpu->a ^ cpu->a;
            cpu->pc++;
            break;

        case 0xA8: // XOR B
            printf("XOR A, B\n");
            cpu->a = cpu->a ^ cpu->b;
            cpu->pc++;
            break;

        case 0xA9: // XOR C
            printf("XOR A, C\n");
            cpu->a = cpu->a ^ cpu->c;
            cpu->pc++;
            break;

        case 0xAA: // XOR D
            printf("XOR A, D\n");
            cpu->a = cpu->a ^ cpu->d;
            cpu->pc++;
            break;

        case 0xAB: // XOR E
            printf("XOR A, E\n");
            cpu->a = cpu->a ^ cpu->e;
            cpu->pc++;
            break;

        case 0xAC: // XOR H
            printf("XOR A, H\n");
            cpu->a = cpu->a ^ cpu->h;
            cpu->pc++;
            break;

        case 0xAD: // XOR L
            printf("XOR A, L\n");
            cpu->a = cpu->a ^ cpu->l;
            cpu->pc++;
            break;

        case 0xAE: // XOR (HL)
            printf("XOR A, (HL)\n");
            cpu->a = cpu->a ^ cpu->memory[(cpu->h << 8) | cpu->l];
            cpu->pc++;
            break;

        case 0xEE: // XOR n
            printf("XOR A, 0x%02X\n", cpu->memory[cpu->pc + 1]);
            cpu->a = cpu->a ^ cpu->memory[cpu->pc + 1];
            cpu->pc += 2;
            break;

        case 0xBF: // CP A
            printf("CP A, A\n");
            {
                uint8_t result = cpu->a - cpu->a;
                cpu->zf = (result == 0);
                cpu->nf = 1;
                cpu->hf = ((cpu->a & 0x0F) - (cpu->a & 0x0F)) < 0;
                cpu->cf = (cpu->a < cpu->a);
                cpu->pc++;
            }
            break;

        case 0xB8: // CP B
            printf("CP A, B\n");
            {
                uint8_t result = cpu->a - cpu->b;
                cpu->zf = (result == 0);
                cpu->nf = 1;
                cpu->hf = ((cpu->a & 0x0F) - (cpu->b & 0x0F)) < 0;
                cpu->cf = (cpu->a < cpu->b);
                cpu->pc++;
            }
            break;

        case 0xB9: // CP C
            printf("CP A, C\n");
            {
                uint8_t result = cpu->a - cpu->c;
                cpu->zf = (result == 0);
                cpu->nf = 1;
                cpu->hf = ((cpu->a & 0x0F) - (cpu->c & 0x0F)) < 0;
                cpu->cf = (cpu->a < cpu->c);
                cpu->pc++;
            }
            break;

        case 0xBA: // CP D
            printf("CP A, D\n");
            {
                uint8_t result = cpu->a - cpu->d;
                cpu->zf = (result == 0);
                cpu->nf = 1;
                cpu->hf = ((cpu->a & 0x0F) - (cpu->d & 0x0F)) < 0;
                cpu->cf = (cpu->a < cpu->d);
                cpu->pc++;
            }
            break;

        case 0xBB: // CP E
            printf("CP A, E\n");
            {
                uint8_t result = cpu->a - cpu->e;
                cpu->zf = (result == 0);
                cpu->nf = 1;
                cpu->hf = ((cpu->a & 0x0F) - (cpu->e & 0x0F)) < 0;
                cpu->cf = (cpu->a < cpu->e);
                cpu->pc++;
            }
            break;

        case 0xBC: // CP H
            printf("CP A, H\n");
            {
                uint8_t result = cpu->a - cpu->h;
                cpu->zf = (result == 0);
                cpu->nf = 1;
                cpu->hf = ((cpu->a & 0x0F) - (cpu->h & 0x0F)) < 0;
                cpu->cf = (cpu->a < cpu->h);
                cpu->pc++;
            }
            break;

        case 0xBD: // CP L
            printf("CP A, L\n");
            {
                uint8_t result = cpu->a - cpu->l;
                cpu->zf = (result == 0);
                cpu->nf = 1;
                cpu->hf = ((cpu->a & 0x0F) - (cpu->l & 0x0F)) < 0;
                cpu->cf = (cpu->a < cpu->l);
                cpu->pc++;
            }
            break;


        // ... (additional cases)

        default:
            printf("Unknown opcode: 0x%02X\n", opcode);
            cpu->pc++;
            break;
    }
}


int main() {
    printf("Starting emulator\n");

    struct CPU cpu;
    initializeCPU(&cpu);

    if (!loadROM(&cpu, "game.gb")) {
        printf("Error loading ROM\n");
        return 1;
    }

    readROMHeader(&cpu);
    
    InitWindow(SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, "Gameboy Emulator");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        emulateCycle(&cpu);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw Gameboy screen here

        EndDrawing();
    }

    CloseWindow();
    printf("Emulator closed\n");
    system("pause"); // Keep the console open
    return 0;
}
