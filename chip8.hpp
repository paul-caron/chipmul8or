#include <cstdint>
#include <string>

class Chip8{
public:
    uint8_t gfx[64*32];
    uint8_t V[16];
    uint8_t key[16];
    uint16_t stack[16];
    uint16_t sp;
    uint16_t opcode;
    uint16_t pc;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint16_t I;
    bool load(char * path);
    void cycle();
    Chip8();
private:
    int instruction_count=0; //slows timers
    void drawSprite();
    uint8_t memory[4096];
};


