

#include "interpret.h"
#include "memory.h"


void int_inst(uint32_t instruction1, page_t* page_table,Core& core, bool is_inst, uint32_t int_num){
	if (is_inst) {
		uint8_t r0;
		r0 = (instruction1 & R0) >> SHIFTR0;
		int_num = core.regs.regs[r0].dword[DW_IND];
		
	}
	
	core.interrupted = true;
	core.sp += 1;
	memory_write_dword(page_table, core.sp, core.pc);
	core.sp += 3;
	memory_read_dword(page_table, int_num * 4, core.pc);
}

void jmp_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0;
    uint32_t address;
    r0 = (instruction1 & R0)>>SHIFTR0;
    uint8_t addrmod = (instruction1 & ADDRMOD) >>SHIFTMOD;
    switch(addrmod){
        case 6://memdir
            memory_read_dword(page_table, core.pc, address);
			core.pc += 4;
        break;
        case 2://regind
            address = core.regs.regs[r0].dword[DW_IND];
            memory_read_dword(page_table, address, address);
        break;
        case 7://regind off
            memory_read_dword(page_table, core.pc, address);
			core.pc += 4;
            if(r0 == 16){
                address += core.sp;
            }else if(r0 == 17){
                address += core.pc;
				break;
            }else{
                address += core.regs.regs[r0].dword[DW_IND];
            }
            memory_read_dword(page_table, address, address);
        break;
    }
    core.pc = address;
}

void call_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0;
    uint32_t address;
    r0 = (instruction1 & R0)>>SHIFTR0;
    uint8_t addrmod = (instruction1 & ADDRMOD) >>SHIFTMOD;
    switch(addrmod){
        case 6://memdir
            memory_read_dword(page_table, core.pc, address);
			core.pc += 4;
        break;
        case 2://regind
            address = core.regs.regs[r0].dword[DW_IND];
            memory_read_dword(page_table, address, address);
        break;
        case 7://regind off
            memory_read_dword(page_table, core.pc, address);
			core.pc += 4;
            if(r0 == 16){
                address += core.sp;
            }else if(r0 == 17){
                address += core.pc;
				break;
            }else{
                address += core.regs.regs[r0].dword[DW_IND];
            }
            memory_read_dword(page_table, address, address);
        break;
    }
	core.sp += 1;
    memory_write_dword(page_table, core.sp, core.pc);
	core.sp += 3;
    core.pc = address;
}

void ret_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint32_t ret_addr;
	core.sp -= 3;
    memory_read_dword(page_table, core.sp, ret_addr);
	core.sp -= 1;
    core.pc = ret_addr;
	if (core.interrupted) {
		core.interrupted = false;
	}
}

void jz_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0, r1;
    uint32_t address;
    r0 = (instruction1 & R0)>>SHIFTR0;
    r1 = (instruction1 & R1)>>SHIFTR1;
    uint8_t addrmod = (instruction1 & ADDRMOD) >>SHIFTMOD;
    
    switch(addrmod){
        case 6://memdir
            memory_read_dword(page_table, core.pc, address);
			core.pc += 4;
        break;
        case 2://regind
            address = core.regs.regs[r0].dword[DW_IND];
            memory_read_dword(page_table, address, address);
        break;
        case 7://regind off
            memory_read_dword(page_table, core.pc, address);
			core.pc += 4;
            if(r0 == 16){
                address += core.sp;
            }else if(r0 == 17){
                address += core.pc;
				break;
            }else{
                address += core.regs.regs[r0].dword[DW_IND];
            }
            memory_read_dword(page_table, address, address);
        break;
    }
	if (core.regs.regs[r1].dword[DW_IND] == 0) {
		core.pc = address;
	}

}

void jnz_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0, r1;
    uint32_t address;
    r0 = (instruction1 & R0)>>SHIFTR0;
    r1 = (instruction1 & R1)>>SHIFTR1;
    uint8_t addrmod = (instruction1 & ADDRMOD) >>SHIFTMOD;
   
	switch(addrmod){
    case 6://memdir
        memory_read_dword(page_table, core.pc, address);
		core.pc += 4;
    break;
    case 2://regind
        address = core.regs.regs[r0].dword[DW_IND];
        memory_read_dword(page_table, address, address);
    break;
    case 7://regind off
        memory_read_dword(page_table, core.pc, address);
		core.pc += 4;

        if(r0 == 16){
            address += core.sp;
        }else if(r0 == 17){
            address += core.pc;
			break;
        }else{
            address += core.regs.regs[r0].dword[DW_IND];
        }
        memory_read_dword(page_table, address, address);
    break;
	}
	if (core.regs.regs[r1].dword[DW_IND] != 0) {
	core.pc = address;
	}
}

void jgz_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0, r1;
    uint32_t address;
    r0 = (instruction1 & R0)>>SHIFTR0;
    r1 = (instruction1 & R1)>>SHIFTR1;
    uint8_t addrmod = (instruction1 & ADDRMOD) >>SHIFTMOD;
    
    switch(addrmod){
        case 6://memdir
			memory_read_dword(page_table, core.pc, address);
			core.pc += 4;
        break;
        case 2://regind
            address = core.regs.regs[r0].dword[DW_IND];
            memory_read_dword(page_table, address, address);
        break;
        case 7://regind off
                
            memory_read_dword(page_table, core.pc, address);
			core.pc += 4;
            if(r0 == 16){
                address += core.sp;
            }else if(r0 == 17){
                address += core.pc;
				break;
            }else{
                address += core.regs.regs[r0].dword[DW_IND];
            }
            memory_read_dword(page_table, address, address);
        break;
    }
	if ((int32_t)core.regs.regs[r1].dword[DW_IND] > 0) {
		core.pc = address;
	}
}

void jgez_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0, r1;
    uint32_t address;
    r0 = (instruction1 & R0)>>SHIFTR0;
    r1 = (instruction1 & R1)>>SHIFTR1;
    uint8_t addrmod = (instruction1 & ADDRMOD) >>SHIFTMOD;
   
    switch(addrmod){
        case 6://memdir
                
            memory_read_dword(page_table, core.pc, address);
			core.pc += 4;
        break;
        case 2://regind
            address = core.regs.regs[r0].dword[DW_IND];
            memory_read_dword(page_table, address, address);
        break;
        case 7://regind off
               
            memory_read_dword(page_table, core.pc, address);
			core.pc += 4;
            if(r0 == 16){
                address += core.sp;
            }else if(r0 == 17){
                address += core.pc;
				break;
            }else{
                address += core.regs.regs[r0].dword[DW_IND];
            }
            memory_read_dword(page_table, address, address);
        break;
    }
	if ((int32_t)core.regs.regs[r1].dword[DW_IND] >= 0) {
		core.pc = address;
	}
}

void jlz_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0, r1;
    uint32_t address;
    r0 = (instruction1 & R0)>>SHIFTR0;
    uint8_t addrmod = (instruction1 & ADDRMOD) >>SHIFTMOD;
    
    switch(addrmod){
        case 6://memdir
                
            memory_read_dword(page_table, core.pc, address);
			core.pc += 4;
        break;
        case 2://regind
            address = core.regs.regs[r0].dword[DW_IND];
            memory_read_dword(page_table, address, address);
        break;
        case 7://regind off
                
            memory_read_dword(page_table, core.pc, address);
			core.pc += 4;
            if(r0 == 16){
                address += core.sp;
            }else if(r0 == 17){
                address += core.pc;
				break;
            }else{
                address += core.regs.regs[r0].dword[DW_IND];
            }
            memory_read_dword(page_table, address, address);
        break;
    }
	if ((int32_t)core.regs.regs[r1].dword[DW_IND] < 0) {
		core.pc = address;
	}
}

void jlez_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0, r1;
    uint32_t address;
    r0 = (instruction1 & R0)>>SHIFTR0;
    r1 = (instruction1 & R1)>>SHIFTR1;
    uint8_t addrmod = (instruction1 & ADDRMOD) >>SHIFTMOD;
    
    switch(addrmod){
        case 6://memdir
                
            memory_read_dword(page_table, core.pc, address);
			core.pc += 4;
        break;
        case 2://regind
            address = core.regs.regs[r0].dword[DW_IND];
            memory_read_dword(page_table, address, address);
        break;
        case 7://regind off
                
            memory_read_dword(page_table, core.pc, address);
			core.pc += 4;
            if(r0 == 16){
                address += core.sp;
            }else if(r0 == 17){
                address += core.pc;
				break;
            }else{
                address += core.regs.regs[r0].dword[DW_IND];
            }
            memory_read_dword(page_table, address, address);
        break;
    }
	if ((int32_t)core.regs.regs[r1].dword[DW_IND] <= 0) {
		core.pc = address;
	}
}

void load_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0, r1;
    r0 = (instruction1 & R0)>>SHIFTR0;
    r1 = (instruction1 & R1)>>SHIFTR1;
    uint8_t type = (instruction1 & TYPE) >> TYPESHIFT;
    uint8_t addrmod = (instruction1 & ADDRMOD) >>SHIFTMOD;
	uint32_t dword;
    switch(addrmod){
        case 4://imeddiate
			dword = core.pc;
			switch (type) {
			case 0://dword
				memory_read_dword(page_table, dword, core.regs.regs[r1].dword[DW_IND]);

				break;
			case 1://word unsigned
				memory_read_word(page_table, dword, core.regs.regs[r1].word[W_IND]);
				core.regs.regs[r1].word[W_IND + 1] = 0;
				break;
			case 5://word signed
				memory_read_word(page_table, dword, core.regs.regs[r1].word[W_IND]);
				if (core.regs.regs[r1].word[W_IND] & SWORD) {
					core.regs.regs[r1].word[W_IND + 1] |= (IBYTE) | (IIBYTE);
				}
				else {
					core.regs.regs[r1].word[W_IND + 1] = 0;
				}
				break;
			case 3://byte unsigned
				memory_read_byte(page_table, dword, core.regs.regs[r1].byte[B_IND], &core);
				core.regs.regs[r1].byte[B_IND + 1] = 0;
				core.regs.regs[r1].byte[B_IND + 2] = 0;
				core.regs.regs[r1].byte[B_IND + 3] = 0;
				break;
			case 7://byte signed
				memory_read_byte(page_table, dword, core.regs.regs[r1].byte[B_IND], &core);
				if (core.regs.regs[r1].byte[B_IND] & SBYTE) {
					core.regs.regs[r1].byte[B_IND + 1] |= IBYTE;
					core.regs.regs[r1].byte[B_IND + 2] |= IBYTE;
					core.regs.regs[r1].byte[B_IND + 3] |= IBYTE;
				}
				else {
					core.regs.regs[r1].byte[B_IND + 1] = 0;
					core.regs.regs[r1].byte[B_IND + 2] = 0;
					core.regs.regs[r1].byte[B_IND + 3] = 0;
				}
				break;
			}
            core.pc+=4;
        break;
        case 0://regdir
			if (r0 == 16) {
				core.regs.regs[r1].dword[DW_IND] = core.sp;
			}
			else {
				core.regs.regs[r1].dword[DW_IND] = core.regs.regs[r0].dword[DW_IND];
			}
        break;
        case 6://memdir
            memory_read_dword(page_table, core.pc, dword);//dword = address
            core.pc+=4;
            switch(type){
                case 0://dword
                    memory_read_dword(page_table,dword, core.regs.regs[r1].dword[DW_IND]);
                    
                break;
                case 1://word unsigned
                    memory_read_word(page_table, dword, core.regs.regs[r1].word[W_IND]);
					core.regs.regs[r1].word[W_IND + 1] = 0;
                break;
                case 5://word signed
                    memory_read_word(page_table, dword, core.regs.regs[r1].word[W_IND]);
                    if(core.regs.regs[r1].word[W_IND] & SWORD){
                        core.regs.regs[r1].word[W_IND+1] |= (IBYTE)|(IIBYTE);
                    }else{
                        core.regs.regs[r1].word[W_IND+1] = 0;
                    }
                break;
                case 3://byte unsigned
                    memory_read_byte(page_table, dword, core.regs.regs[r1].byte[B_IND], &core);
                    core.regs.regs[r1].byte[B_IND+1] = 0;
                    core.regs.regs[r1].byte[B_IND+2] = 0;
                    core.regs.regs[r1].byte[B_IND+3] = 0;
                break;
                case 7://byte signed
                    memory_read_byte(page_table, dword, core.regs.regs[r1].byte[B_IND], &core);
                    if(core.regs.regs[r1].byte[B_IND] & SBYTE){
						core.regs.regs[r1].byte[B_IND + 1] |= IBYTE;
						core.regs.regs[r1].byte[B_IND + 2] |= IBYTE;
						core.regs.regs[r1].byte[B_IND + 3] |= IBYTE;
                    }else{
                        core.regs.regs[r1].byte[B_IND+1] = 0;
                        core.regs.regs[r1].byte[B_IND+2] = 0;
                        core.regs.regs[r1].byte[B_IND+3] = 0;
                    }
                break;
            }
        break;
        case 2://regind
            switch(type){
                case 0://dword
                    memory_read_dword(page_table,core.regs.regs[r0].dword[DW_IND], core.regs.regs[r1].dword[DW_IND]);
                    
                break;
                case 1://word unsigned
                    memory_read_word(page_table, core.regs.regs[r0].dword[DW_IND], core.regs.regs[r1].word[W_IND]);
					core.regs.regs[r1].word[W_IND + 1] = 0;
                break;
                case 5://word signed
                    memory_read_word(page_table, core.regs.regs[r0].dword[DW_IND], core.regs.regs[r1].word[W_IND]);
                    if(core.regs.regs[r1].word[W_IND] & SWORD){
                        core.regs.regs[r1].word[W_IND+1] |= (IBYTE)|(IIBYTE);
                    }else{
                        core.regs.regs[r1].word[W_IND+1] = 0;
                    }
                break;
				case 3://byte unsigned
					memory_read_byte(page_table, dword, core.regs.regs[r1].byte[B_IND], &core);
					core.regs.regs[r1].byte[B_IND + 1] = 0;
					core.regs.regs[r1].byte[B_IND + 2] = 0;
					core.regs.regs[r1].byte[B_IND + 3] = 0;
					break;
				case 7://byte signed
					memory_read_byte(page_table, dword, core.regs.regs[r1].byte[B_IND], &core);
					if (core.regs.regs[r1].byte[B_IND] & SBYTE) {
						core.regs.regs[r1].byte[B_IND + 1] |= IBYTE;
						core.regs.regs[r1].byte[B_IND + 2] |= IBYTE;
						core.regs.regs[r1].byte[B_IND + 3] |= IBYTE;
					}
					else {
						core.regs.regs[r1].byte[B_IND + 1] = 0;
						core.regs.regs[r1].byte[B_IND + 2] = 0;
						core.regs.regs[r1].byte[B_IND + 3] = 0;
					}
					break;
            }
        break;
        case 7://regind + off
            
            memory_read_dword(page_table, core.pc, dword);//dword = offset
			core.pc += 4;
            if(r0 == 16){
                dword += core.sp;
            }else if(r0 == 17){
                dword += core.pc;
            }else{
                dword += core.regs.regs[r0].dword[DW_IND];
            }
            switch(type){
                case 0://dword
                    memory_read_dword(page_table,dword, core.regs.regs[r1].dword[DW_IND]);
                    
                break;
                case 1://word unsigned
                    memory_read_word(page_table, dword, core.regs.regs[r1].word[W_IND]);
					core.regs.regs[r1].word[W_IND + 1] = 0;
                break;
                case 5://word signed
                    memory_read_word(page_table, dword, core.regs.regs[r1].word[W_IND]);
                    if(core.regs.regs[r1].word[W_IND] & SWORD){
                        core.regs.regs[r1].word[W_IND+1] |= (IBYTE)|(IIBYTE);
                    }else{
                        core.regs.regs[r1].word[W_IND+1] = 0;
                    }
                break;
				case 3://byte unsigned
					memory_read_byte(page_table, dword, core.regs.regs[r1].byte[B_IND], &core);
					core.regs.regs[r1].byte[B_IND + 1] = 0;
					core.regs.regs[r1].byte[B_IND + 2] = 0;
					core.regs.regs[r1].byte[B_IND + 3] = 0;
					break;
				case 7://byte signed
					memory_read_byte(page_table, dword, core.regs.regs[r1].byte[B_IND], &core);
					if (core.regs.regs[r1].byte[B_IND] & SBYTE) {
						core.regs.regs[r1].byte[B_IND + 1] |= IBYTE;
						core.regs.regs[r1].byte[B_IND + 2] |= IBYTE;
						core.regs.regs[r1].byte[B_IND + 3] |= IBYTE;
					}
					else {
						core.regs.regs[r1].byte[B_IND + 1] = 0;
						core.regs.regs[r1].byte[B_IND + 2] = 0;
						core.regs.regs[r1].byte[B_IND + 3] = 0;
					}
					break;
            }
        break;
    }
}   

void store_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0, r1;
    r0 = (instruction1 & R0)>>SHIFTR0;
    r1 = (instruction1 & R1)>>SHIFTR1;
    uint8_t type = (instruction1 & TYPE) >> TYPESHIFT;
    uint8_t addrmod = (instruction1 & ADDRMOD) >>SHIFTMOD;
	uint32_t dword;
    switch(addrmod){
        case 0://regdir
            core.regs.regs[r0].dword[DW_IND] = core.regs.regs[r1].dword[DW_IND];
        break;
        case 6://memdir
            memory_read_dword(page_table, core.pc, dword);//dword = address
            core.pc+=4;
            switch(type){
                case 0://dword
                    memory_write_dword(page_table,dword, core.regs.regs[r1].dword[DW_IND]);
                break;
                case 1://word unsigned
                    memory_write_word(page_table, dword, core.regs.regs[r1].word[W_IND]);
                break;
                case 3://byte unsigned
                    memory_write_byte(page_table, dword, core.regs.regs[r1].byte[B_IND]);
            }
        break;
        case 2://regind
            switch(type){
                case 0://dword
                    memory_write_dword(page_table,core.regs.regs[r0].dword[DW_IND], core.regs.regs[r1].dword[DW_IND]);
                break;
                case 1://word unsigned
                    memory_write_word(page_table, core.regs.regs[r0].dword[DW_IND], core.regs.regs[r1].word[W_IND]);
                break;
                case 3://byte unsigned
                    memory_write_byte(page_table, core.regs.regs[r0].dword[DW_IND], core.regs.regs[r1].byte[B_IND]);

            }
        break;
        case 7://regind + off
            
            memory_read_dword(page_table, core.pc, dword);//dword = offset
			core.pc += 4;
            if(r0 == 16){
                dword += core.sp;
            }else if(r0 == 17){
                dword += core.pc;
            }else{
                dword += core.regs.regs[r0].dword[DW_IND];
            }
            switch(type){
                case 0://dword
                    memory_write_dword(page_table,dword, core.regs.regs[r1].dword[DW_IND]);
                break;
                case 1://word unsigned
                    memory_write_word(page_table, dword, core.regs.regs[r1].word[W_IND]);
                break;
                case 3://byte unsigned
                    memory_write_byte(page_table, dword, core.regs.regs[r1].byte[B_IND]);
                break;
            }
        break;
    }
}

void push_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0;
    r0 = (instruction1 & R0)>>SHIFTR0;
    core.sp += 1;
    memory_write_dword(page_table, core.sp, core.regs.regs[r0].dword[DW_IND]);
	core.sp += 3;
}

void pop_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0;
    r0 = (instruction1 & R0)>>SHIFTR0;
	core.sp -= 3;
    memory_read_dword(page_table, core.sp, core.regs.regs[r0].dword[DW_IND]);
	core.sp -= 1;
    
}

void add_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0, r1, r2;
    r0 = (instruction1 & R0)>>SHIFTR0;
    r1 = (instruction1 & R1)>>SHIFTR1;
    r2 = (instruction1 & R2)>>SHIFTR2;
    core.regs.regs[r0].dword[DW_IND] = core.regs.regs[r1].dword[DW_IND] + core.regs.regs[r2].dword[DW_IND];
}

void sub_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0, r1, r2;
    r0 = (instruction1 & R0)>>SHIFTR0;
    r1 = (instruction1 & R1)>>SHIFTR1;
    r2 = (instruction1 & R2)>>SHIFTR2;
    core.regs.regs[r0].dword[DW_IND] = core.regs.regs[r1].dword[DW_IND] - core.regs.regs[r2].dword[DW_IND];
}

void mul_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0, r1, r2;
    r0 = (instruction1 & R0)>>SHIFTR0;
    r1 = (instruction1 & R1)>>SHIFTR1;
    r2 = (instruction1 & R2)>>SHIFTR2;
    core.regs.regs[r0].dword[DW_IND] = core.regs.regs[r1].dword[DW_IND]*core.regs.regs[r2].dword[DW_IND];
}

void div_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0, r1, r2;
    r0 = (instruction1 & R0)>>SHIFTR0;
    r1 = (instruction1 & R1)>>SHIFTR1;
    r2 = (instruction1 & R2)>>SHIFTR2;
    core.regs.regs[r0].dword[DW_IND] = core.regs.regs[r1].dword[DW_IND]/core.regs.regs[r2].dword[DW_IND];
}

void mod_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0, r1, r2;
    r0 = (instruction1 & R0)>>SHIFTR0;
    r1 = (instruction1 & R1)>>SHIFTR1;
    r2 = (instruction1 & R2)>>SHIFTR2;
    core.regs.regs[r0].dword[DW_IND] = core.regs.regs[r1].dword[DW_IND] % core.regs.regs[r2].dword[DW_IND];
}

void and_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0, r1, r2;
    r0 = (instruction1 & R0)>>SHIFTR0;
    r1 = (instruction1 & R1)>>SHIFTR1;
    r2 = (instruction1 & R2)>>SHIFTR2;
    core.regs.regs[r0].dword[DW_IND] = core.regs.regs[r1].dword[DW_IND] & core.regs.regs[r2].dword[DW_IND];
}

void or_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0, r1, r2;
    r0 = (instruction1 & R0)>>SHIFTR0;
    r1 = (instruction1 & R1)>>SHIFTR1;
    r2 = (instruction1 & R2)>>SHIFTR2;
    core.regs.regs[r0].dword[DW_IND] = core.regs.regs[r1].dword[DW_IND] | core.regs.regs[r2].dword[DW_IND];
}

void xor_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0, r1, r2;
    r0 = (instruction1 & R0)>>SHIFTR0;
    r1 = (instruction1 & R1)>>SHIFTR1;
    r2 = (instruction1 & R2)>>SHIFTR2;
    core.regs.regs[r0].dword[DW_IND] = core.regs.regs[r1].dword[DW_IND] ^ core.regs.regs[r2].dword[DW_IND];
}

void not_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0, r1;
    r0 = (instruction1 & R0)>>SHIFTR0;
    r1 = (instruction1 & R1)>>SHIFTR1;
    core.regs.regs[r0].dword[DW_IND] = ~core.regs.regs[r1].dword[DW_IND];
}

void asl_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0, r1, r2;
    r0 = (instruction1 & R0)>>SHIFTR0;
    r1 = (instruction1 & R1)>>SHIFTR1;
    r2 = (instruction1 & R2)>>SHIFTR2;
    core.regs.regs[r0].dword[DW_IND] = core.regs.regs[r1].dword[DW_IND] << core.regs.regs[r2].dword[DW_IND];
}

void asr_inst(uint32_t instruction1, page_t* page_table,Core& core){

    uint8_t r0, r1, r2;
    r0 = (instruction1 & R0)>>SHIFTR0;
    r1 = (instruction1 & R1)>>SHIFTR1;
    r2 = (instruction1 & R2)>>SHIFTR2;
    core.regs.regs[r0].dword[DW_IND] = (int32_t)core.regs.regs[r1].dword[DW_IND] >> core.regs.regs[r2].dword[DW_IND];
}

