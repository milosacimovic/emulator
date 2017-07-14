#ifndef _INTERPRET_H_
#define _INTERPRET_H_

#include <cstdint>
#include <condition_variable>


#define TYPE 0x000038
#define TYPESHIFT 3
#define ADDRMOD 0xE00000
#define SWORD 0x8000
#define SBYTE 0x80
#define SHIFTMOD 21
#define SHIFTR0 16
#define SHIFTR1 11
#define SHIFTR2 6
#define R0 0x1F0000
#define R1 0x00F800
#define R2 0x0007C0

#include "emulator.h"
#include "memory.h"

void int_inst(uint32_t instruction1,page_t* page_table, Core& core, bool is_inst, uint32_t int_num);

void jmp_inst(uint32_t instruction1,page_t* page_table,Core& core);

void call_inst(uint32_t instruction1,page_t* page_table,Core& core);

void ret_inst(uint32_t instruction1,page_t* page_table,Core& core);

void jz_inst(uint32_t instruction1,page_t* page_table,Core& core);

void jnz_inst(uint32_t instruction1,page_t* page_table,Core& core);

void jgz_inst(uint32_t instruction1,page_t* page_table,Core& core);

void jgez_inst(uint32_t instruction1,page_t* page_table,Core& core);

void jlz_inst(uint32_t instruction1,page_t* page_table,Core& core);

void jlez_inst(uint32_t instruction1,page_t* page_table,Core& core);

void load_inst(uint32_t instruction1,page_t* page_table,Core& core);

void store_inst(uint32_t instruction1,page_t* page_table,Core& core);

void push_inst(uint32_t instruction1,page_t* page_table,Core& core);

void pop_inst(uint32_t instruction1,page_t* page_table,Core& core);

void add_inst(uint32_t instruction1,page_t* page_table,Core& core);

void sub_inst(uint32_t instruction1,page_t* page_table,Core& core);

void mul_inst(uint32_t instruction1,page_t* page_table,Core& core);

void div_inst(uint32_t instruction1,page_t* page_table,Core& core);

void mod_inst(uint32_t instruction1,page_t* page_table,Core& core);

void and_inst(uint32_t instruction1,page_t* page_table,Core& core);

void or_inst(uint32_t instruction1,page_t* page_table,Core& core);

void xor_inst(uint32_t instruction1,page_t* page_table,Core& core);

void not_inst(uint32_t instruction1,page_t* page_table,Core& core);

void asl_inst(uint32_t instruction1,page_t* page_table,Core& core);

void asr_inst(uint32_t instruction1,page_t* page_table,Core& core);

#endif
