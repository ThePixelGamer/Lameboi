#include "GB.h"

GB::GB() {
    cpu = new CPU();
    ppu = new PPU();
}

void GB::load(string filename) {
    FILE* rom = fopen(filename.c_str(), "rb");
    if (rom == NULL) {
        fputs("File error", stderr);
    } else {
        fseek(rom, 0, SEEK_END);
        uint32_t romSize = ftell(rom);
        rewind(rom);

        uint8_t* buffer = (uint8_t*)malloc(sizeof(uint8_t) * romSize);
        if (buffer == NULL) {
            fputs("Memory error", stderr);
        } else {
            uint16_t result = fread(buffer, 1, romSize, rom);
            if (result != romSize) {
                fputs("Reading error", stderr);
            } else {
                cpu->memory.reserve(0xFFFF);

                for(uint32_t i = 0; i < sizeof(cpu->bios); i++)
                    cpu->memory[i] = cpu->bios[i];

                while(cpu->memory[0xFF50] != 1)
                    cpu->emulateOpcode();

                for(uint32_t i = 0; i < romSize; i++)
                    cpu->memory[i] = buffer[i];

                fclose(rom);
                free(buffer);
            }
        }
    }
}
