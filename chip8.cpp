#include "chip8.hpp"
#include <ctime>
#include <random>
#include <fstream>

using namespace std;

uint8_t fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

Chip8::Chip8(){
    sound_timer = 0;
    delay_timer = 0;
    instruction_count = 0;
    I = 0;
    pc = 512;
    sp = 0;
    opcode = 0;
    for(int i=0;i<16;++i){
        stack.at(i) = 0;
        V.at(i) = 0;
        key.at(i) = 0;
    }
    for(auto& pixel: gfx) pixel=0;
    for(auto& b: memory) b=0;
    for(int i=0;i<80;++i) memory.at(i) = fontset[i];
    srand(time(0));
}

bool Chip8::load(char * path){
    ifstream file(path, ios::in | ios::binary);
    if(!file.is_open()) return false;
    file.read((char*)(&memory.at(512)), 4096 - 512);
    return true;
}

uint8_t n(uint16_t opcode){
    return opcode & 0x000F;
}

uint8_t nn(uint16_t opcode){
    return opcode & 0x00FF;
}

uint16_t nnn(uint16_t opcode){
    return opcode & 0x0FFF;
}

uint8_t x(uint16_t opcode){
    return (opcode & 0x0F00)>>8;
}

uint8_t y(uint16_t opcode){
    return (opcode & 0x00F0)>>4;
}

void Chip8::drawSprite(){
    uint8_t Vx = (V[x(opcode)]+256)%64;
    uint8_t Vy = (V[y(opcode)]+256)%32;
    uint8_t height = n(opcode);
    uint8_t spriteRowData;
    V[0xF] = 0;
    for(int row=0;row<height;++row){
        spriteRowData=memory[I+row];
        for(int col=0;col<8;++col)
            if(spriteRowData&(128>>col))
                if(!(gfx.at(64*((Vy+row)%32)+((Vx+col)%64))^=1)) V[0xF]=1;//collision detection
    }
    pc+=2;
}

void Chip8::cycle() {
    opcode = memory[pc] << 8 | memory[pc + 1];
    switch(opcode & 0xF000){
        case 0x0000: switch(nn(opcode)){
                         case 0xE0: for(auto & pixel:gfx) pixel = 0;
                                    pc+=2;
                                    break;
                         case 0xEE: pc = stack.at(--sp)+ 2;
                                    break;
                         default: exit(1);
                     }
                     break;
        case 0x1000: pc = nnn(opcode);  break;
        case 0x2000: stack.at(sp++) = pc;
                     pc = nnn(opcode);
                     break;
        case 0x3000: if (V[x(opcode)] == nn(opcode)) pc += 4;
                     else pc += 2;
                     break;
        case 0x4000: if (V[x(opcode)] != nn(opcode)) pc += 4;
                     else pc += 2;
                     break;
        case 0x5000: if (V[x(opcode)] == V[y(opcode)]) pc += 4;
                     else pc += 2;
                     break;
        case 0x6000: V[x(opcode)] = nn(opcode);
                     pc += 2;
                     break;
        case 0x7000: V[x(opcode)] += nn(opcode);
                     pc += 2;
                     break;
        case 0x8000: switch(n(opcode)){
                         case 0x0000: V[x(opcode)] = V[y(opcode)];
                                      pc += 2;
                                      break;
                         case 0x0001: V[x(opcode)] |= V[y(opcode)];
                                      pc += 2;
                                      break;
                         case 0x0002: V[x(opcode)] &= V[y(opcode)];
                                      pc += 2;
                                      break;
                         case 0x0003: V[x(opcode)] ^= V[y(opcode)];
                                      pc += 2;
                                      break;
                         case 0x0004: V[x(opcode)] += V[y(opcode)];
                                      if(V[y(opcode)] > (0xFF - V[x(opcode)])) V[0xF] = 1;
                                      else V[0xF] = 0;                                                                                                                                                                 pc += 2;
                                      break;
                         case 0x0005: if(V[y(opcode)] > V[x(opcode)]) V[0xF] = 0;
                                      else V[0xF] = 1;
                                      V[x(opcode)] -= V[y(opcode)];
                                      pc += 2;
                                      break;
                         case 0x0006: V[0xF] = V[x(opcode)] & 0x1;
                                      V[x(opcode)] >>= 1;
                                      pc += 2;
                                      break;
                         case 0x0007: if(V[x(opcode)] > V[y(opcode)])
                                          V[0xF] = 0;
                                      else
                                          V[0xF] = 1;
                                      V[x(opcode)] = V[y(opcode)] - V[x(opcode)];
                                      pc += 2;
                                      break;
                         case 0x000E: V[0xF] = V[x(opcode)] >> 7;
                                      V[x(opcode)] <<= 1;
                                      pc += 2;                                                                                                                                                                         break;
                         default: exit(1);
                     }
                     break;
        case 0x9000: if (V[x(opcode)] != V[y(opcode)]) pc += 4;
                     else pc += 2;
                     break;
        case 0xA000: I = nnn(opcode);
                     pc += 2;
                     break;
        case 0xB000: pc = nnn(opcode) + V[0];
                     break;
        case 0xC000: V[x(opcode)] = (rand() % (256)) & nn(opcode);                                                                                                                                    pc += 2;
                     break;
        case 0xD000: drawSprite();
                     break;
        case 0xE000: switch(nn(opcode)){
                         case 0x009E: if (key.at(V[x(opcode)]))
                                          pc +=  4;
                                      else pc += 2;
                                      break;
                         case 0x00A1: if (!key.at(V[x(opcode)]))
                                          pc +=  4;
                                      else pc += 2;
                                      break;
                         default: exit(1);
                     }
                     break;
        case 0xF000: switch(opcode & 0x00FF) {
                         case 0x0007: V[x(opcode)] = delay_timer;
                                      pc += 2;
                                      break;
                         case 0x000A: for(int i = 0; i < 16; ++i){
                                          if(key[i]){
                                              V[x(opcode)] = i;
                                              pc += 2 ;
                                              break;
                                          }
                                      }
                                      break;
                         case 0x0015: delay_timer = V[x(opcode)];
                                      pc += 2;
                                      break;
                         case 0x0018: sound_timer = V[x(opcode)];
                                      pc += 2;
                                      break;
                         case 0x001E: if(I + V[x(opcode)] > 0xFFF) V[0xF] = 1;
                                      else V[0xF] = 0;
                                      I += V[x(opcode)];
                                      pc += 2;
                                      break;
                         case 0x0029: I = V[x(opcode)] * 5;
                                      pc += 2;
                                      break;
                         case 0x0033: memory.at(I) = V[x(opcode)] / 100;
                                      memory.at(I + 1) = (V[x(opcode)] / 10) % 10;
                                      memory.at(I + 2) = V[x(opcode)] % 10;
                                      pc += 2;
                                      break;
                         case 0x0055: for(int i=0;i<=x(opcode);++i)
                                          memory.at(I + i) = V[i];
                                      I += x(opcode) + 1;
                                      pc += 2;
                                      break;

                         case 0x0065: for(int i=0;i<=x(opcode);++i)
                                          V[i] = memory.at(I + i);
                                      I += x(opcode) + 1;
                                      pc += 2;
                                      break;
                         default: exit(1);
                     }
                     break;
        default: exit(1);
    }

    if(instruction_count == 9){
        if (delay_timer > 0) --delay_timer;
        if (sound_timer > 0) --sound_timer;
        instruction_count =0;
    }
    ++instruction_count;
}

	
