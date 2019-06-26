#include "GB.h"

GB::GB() {
    cpu = new CPU();
    ppu = new PPU();
}

void GB::load(string filename, EmulatorScreen* screen) {

    FILE* rom = fopen(filename.c_str(), "rb");
    if (rom == NULL) fputs("File error", stderr);
    else {
        fseek(rom, 0, SEEK_END);
        uint32_t romSize = ftell(rom);
        rewind(rom);

        uint8_t* buffer = (uint8_t*)malloc(sizeof(uint8_t) * romSize);
        if(buffer == NULL) fputs("memory error", stderr);
        else {
            if(fread(buffer, 1, romSize, rom) != romSize) fputs("Reading error", stderr);
            else {
                for(auto i = 0; i < 0x10000; i++)
                    cpu->memory.push_back(0);

                //for(auto i = 0; i < bios.size(); i++) cpu->memory[i] = bios[i]; //load bios
                for(auto i = bios.size(); i < 0x8000; i++) cpu->memory[i] = buffer[i]; //load rom
                //while(memory[0xFF50] != 1) cpu->emulateOpcode(); //run bios
                for(auto i = 0; i < bios.size(); i++) cpu->memory[i] = buffer[i]; //overwrite bios
            }
        }
        free(buffer);
    }
    fclose(rom);

    while(running) {
        cpu->emulateOpcode();

        //run every frame
        screen->update();
        QApplication::processEvents(QEventLoop::AllEvents);
    }
}
