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

uint8_t super_fontset[160]={
    0xff, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xff, // 0
    0x18, 0x78, 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0xff, 0xff, // 1
    0xff, 0xff, 0x03, 0x03, 0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff, // 2
    0xff, 0xff, 0x03, 0x03, 0xff, 0xff, 0x03, 0x03, 0xff, 0xff, // 3
    0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xff, 0x03, 0x03, 0x03, 0x03, // 4
    0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff, 0x03, 0x03, 0xff, 0xff, // 5
    0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff, 0xc3, 0xc3, 0xff, 0xff, // 6
    0xff, 0xff, 0x03, 0x03, 0x06, 0x0c, 0x18, 0x18, 0x18, 0x18, // 7
    0xff, 0xff, 0xc3, 0xc3, 0xff, 0xff, 0xc3, 0xc3, 0xff, 0xff, // 8
    0xff, 0xff, 0xc3, 0xc3, 0xff, 0xff, 0x03, 0x03, 0xff, 0xff, // 9
    0x7e, 0xff, 0xc3, 0xc3, 0xc3, 0xff, 0xff, 0xc3, 0xc3, 0xc3, // A
    0xfc, 0xfc, 0xc3, 0xc3, 0xfc, 0xfc, 0xc3, 0xc3, 0xfc, 0xfc, // B
    0x3c, 0xff, 0xc3, 0xc0, 0xc0, 0xc0, 0xc0, 0xc3, 0xff, 0x3c, // C
    0xfc, 0xfe, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xfe, 0xfc, // D
    0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff, // E
    0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff, 0xc0, 0xc0, 0xc0, 0xc0
};

Chip8::Chip8(){
    sound_timer = 0;
    delay_timer = 0;
    instruction_count = 0;
    I = 0;
    pc = 512;
    sp = 0;
    opcode = 0;
    extended_mode = false;
    for(int i=0;i<16;++i){
        stack.at(i) = 0;
        V.at(i) = 0;
        key.at(i) = 0;
    }
    for(auto& pixel: gfx) pixel=0;
    for(auto& b: memory) b=0;
    for(int i=0;i<80;++i) memory.at(i) = fontset[i];
    for(int i=0;i<160;++i) memory.at(i+80) = super_fontset[i];
    for(auto& r:Rpl) r=0;
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
    uint8_t Vx = (V[x(opcode)]+256)%(64*(1+extended_mode));
    uint8_t Vy = (V[y(opcode)]+256)%(32*(1+extended_mode));
    uint8_t height = n(opcode);
    if((!height) && extended_mode) height=16;
    uint8_t spriteRowData;
    V[0xF] = 0;
    if(!extended_mode){
    for(int row=0;row<height;++row){
        spriteRowData=memory[I+row];
        for(int col=0;col<8;++col)
            if(spriteRowData&(128>>col))
                if(!(gfx.at(64*((Vy+row)%32)+((Vx+col)%64))^=1)) V[0xF]=1;//collision detection
    }
    }else{
    if(height!=16){
    for(int row=0;row<height;++row){
        spriteRowData=memory[I+row];
        for(int col=0;col<8;++col)
            if(spriteRowData&(128>>col))
                if(!(gfx.at(128*((Vy+row)%64)+((Vx+col)%128))^=1)) V[0xF]=1;//collision detection
    }
    }else{
    for(int row=0;row<height;++row){
        spriteRowData=memory[I+row*2];
        for(int col=0;col<8;++col)
            if(spriteRowData&(128>>col))
                if(!(gfx.at(128*((Vy+row)%64)+((Vx+col)%128))^=1))
                V[0xF]=1;//collisio
    }
    Vx+=8;
    for(int row=0;row<height;++row){
        spriteRowData=memory[1+I+row*2];
        for(int col=0;col<8;++col)
            if(spriteRowData&(128>>col))
               if(!(gfx.at(128*((Vy+row)%64)+((Vx+col)%128))^=1))
               V[0xF]=1;
    }
    }}
    pc+=2;
}

void Chip8::down(){
    uint8_t N = n(opcode);
    for(int i=8191;i>=128*N;--i)
        gfx[i]=gfx[i-128*N];
    for(int i=0;i<128*N;++i)
        gfx[i]=0;
}

void Chip8::right(){
    for(int row=0;row<64;++row)
    for(int col=127;col>4;col--)
        gfx[row*128+col]=gfx[row*128+col-4];
    for(int row=0;row<64;++row)
    for(int col=0;col<4;col++)
        gfx[row*128+col]=0;
}

void Chip8::left(){
    for(int row=0;row<64;++row)
    for(int col=0;col<128-4;++col)
        gfx[row*128+col]=gfx[row*128+col+4];
    for(int row=0;row<64;++row)
    for(int col=124;col<128;++col)
        gfx[row*128+col]=0;
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
                         case 0xFB: right();
                                    pc += 2;
                                    break;
                         case 0xFC: left();
                                    pc += 2;
                                    break;
                         case 0xFD: exit(0); pc += 2; break;
                         case 0xFE: extended_mode = 0;
                                    pc += 2;
                                    break;
                         case 0xFF: extended_mode = 1;
                                    pc += 2;
                                    break;
                         default: if(y(opcode)==0xC){
                                      down();
                                      pc += 2;
                                  }else exit(1);
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
                                      else V[0xF] = 0;
                                      pc += 2;
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
                                      pc += 2;
                                      break;
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
        case 0xC000: V[x(opcode)] = (rand() % (256)) & nn(opcode);
                     pc += 2;
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
                         //point I to 10byte font sprite for Vx
                         case 0x0030: I = 80 + V[x(opcode)]*10;
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
                         //Store V0..VX in RPL user flags (X <= 7)
                         case 0x0075: for(int i=0;i<x(opcode);++i)
                                          Rpl.at(i) = V[i];
                                      pc += 2;
                                      break;
                         //Read V0..VX from RPL user flags (X <= 7) 
                         case 0x0085: for(int i=0;i<x(opcode);++i)
                                          V[i] = Rpl.at(i);
                                      pc += 2;
                                      break;
                         default: exit(1);
                     }
                     break;
        default: exit(1);
    }

    if(instruction_count == 9*(1+extended_mode)){
        if (delay_timer > 0) --delay_timer;
        if (sound_timer > 0) --sound_timer;
        instruction_count =0;
    }
    ++instruction_count;
}

	
