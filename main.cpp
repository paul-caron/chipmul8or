#include "chip8.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <map>
#include <csignal>
#include <ncurses.h>
using namespace std;

map<char,int>keymap={
{'x',0},
{'1',1},
{'2',2}, 
{'3',3}, 
{'q',4},
{'w',5}, 
{'e',6}, 
{'a',7}, 
{'s',8},
{'d',9},
{'z',10},
{'c',11},
{'4',12}, 
{'r',13}, 
{'f',14}, 
{'v',15}
};

void stop(int signum){
    endwin();
    exit(0);
}

void chip8Status(Chip8 c8){
    attroff(A_REVERSE);
    int x=c8.extended_mode*64+65;
    for(int i=0;i<16;++i)
        mvprintw(i,x,"V%2x: %2x",i,c8.V.at(i));
    mvprintw(16,x,"I: %x",c8.I);
    mvprintw(17,x,"sp: %x",c8.sp);
    mvprintw(18,x,"delay: %x",c8.delay_timer);
    mvprintw(19,x,"sound: %x",c8.sound_timer);
    mvprintw(20,x,"opcode: %x",c8.opcode);
    move(31,x-1);
    refresh();
}

int main(int argc, char ** argv){
    signal(SIGINT, stop);
    if(argc <=1) return 0;
    Chip8 c8=Chip8();
    cout << "Loading ROM: " << argv[1] << endl;
    if(!c8.load(argv[1])){
        cerr << "ROM could not be loaded" << endl;
        return 0;
    }
    cout << "ROM loaded successfully" <<endl;
    initscr();
    noecho();
    cbreak();
    nodelay(stdscr,true);
    while(true){
        try{
            c8.cycle();
        }catch(std::exception e){
            chip8Status(c8);
//            nocbreak();
            nodelay(stdscr,false);
            getch();
            stop(0);
        }
        this_thread::sleep_for(chrono::milliseconds(2-c8.extended_mode));
        int i=0;
        for(int y=0;y<(1+c8.extended_mode)*32;++y)
        for(int x=0;x<(1+c8.extended_mode)*64;++x){
            if(c8.gfx[i++]) attron(A_REVERSE);
            else attroff(A_REVERSE);
            mvaddch(y,x,' ');
        }
        refresh();
        chip8Status(c8);
        int key = getch();
        if(key!=ERR){
            for(auto & k: c8.key) k=false;
            if(keymap.find(key)!=keymap.end())
                c8.key[keymap[key]] = true;
            if(key == 27 && getch() == ERR) break;
        }
        if(c8.sound_timer) beep();
    }
    refresh();
    endwin();
    return 0;
}
