#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include <cstdint>
#include <vector>
#include <cstdio>
#include <mutex>
#include <condition_variable>

#include "memory.h"
#include "tables.h"
/*Emulator data structures*/

#define DW_IND 0
#define W_IND 0
#define B_IND 0

//enum {R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15};

typedef union reg {
	uint32_t dword[1];
	uint16_t word[2];
	uint8_t byte[4];
} reg;

//are flags really necessary : no

typedef struct CPU_regs {
	reg regs[16];
}cpu_regs;

typedef struct core{
	uint32_t pc;
	uint32_t sp;
	page_t* page_table;
	std::mutex mutex;
	std::condition_variable cv;
	cpu_regs regs;
	bool interrupted;
	bool in_reg_read;
	bool waiting_input;
	bool key_pressed;
	bool running;
	core(uint32_t start, uint32_t s) : pc(start), sp(s), page_table(NULL), interrupted(false), in_reg_read(true), waiting_input(false), mutex(), cv(), running(true), key_pressed(false) {}
}Core;

typedef struct emulate {
	Core* core;
}Emulate;

int read_object_file(FILE* input, std::vector<std::vector<Symbol*>>& symtbl,std::vector<RelTable*>& reltbls);

#endif
