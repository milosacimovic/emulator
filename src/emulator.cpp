#include <cstdio>
#include <cstring>
#include <chrono>
#include <thread>

#include "utils.h"
#include "memory.h"
#include "emulator.h"
#include "tables.h"
#include "interpret.h"

using namespace std;

const uint8_t MAX_ARGS = 3;
const uint16_t BUF_SIZE = 1024;
const char* IGNORE_CHARS = " \f\n\r\t\v";
const int NUM_BUCKETS = 54;
/*A char flag will be set depending on where
	the buffer is currently at
*/
int read_object_file(FILE* input, vector<vector<Symbol*>>& symtbl,vector<RelTable*>& reltbls){
	char buf[BUF_SIZE];
	char inside;
	char* sec_name;
	int i = 0;
	Symbol* seg;
	while(fgets(buf, BUF_SIZE, input) != NULL){
		char* cur = strtok(buf, IGNORE_CHARS);
		if (cur == NULL) {
			continue;
		}
		if(cur[0] == '#'){
			if(strcmp(buf, "#TabelaSimbola") == 0){inside ='t';}
			else if(strcmp(buf, "#end") == 0){break;}
			else{
				inside = 'r';
				char* pt = strchr(buf, '.');
				sec_name = strdup(pt);
				seg = get_symbol(symtbl, pt);
				reltbls.push_back(new RelTable());
				reltbls[i]->sec = seg->num;
				reltbls[i++]->section_content = new uint8_t[seg->sec_size];
			}
		}else{
			Symbol* in;
			uint32_t num;
			char* name;
			uint32_t addr;
			int32_t sec_num;
			uint32_t sec_size;
			switch(inside){
				case 't':// Inside symbol table
					if(strcmp(cur, "SEG") == 0){
						cur = strtok(NULL, IGNORE_CHARS);
						num = strtol(cur, NULL, 10);
						cur = strtok(NULL, IGNORE_CHARS);
						name = strdup(cur);
						cur = strtok(NULL, IGNORE_CHARS);
						cur = strtok(NULL, IGNORE_CHARS);
						addr = strtol(cur, NULL, 16);
						add_to_table(symtbl, name, addr, &in);
						in->num = num;
						in->sec_num = num;
						cur = strtok(NULL, IGNORE_CHARS);
						sec_size = strtol(cur, NULL, 16);
						in->sec_size = sec_size;
						cur = strtok(NULL, IGNORE_CHARS);
						for(int i = 0; cur[i] != NULL; i++){
							in->flags.push_back(cur[i]);
						}
					}else{
						cur = strtok(NULL, IGNORE_CHARS);
						num = strtol(cur, NULL, 10);
						cur = strtok(NULL, IGNORE_CHARS);
						name = strdup(cur);
						cur = strtok(NULL, IGNORE_CHARS);
						sec_num = strtol(cur, NULL, 10);
						if (sec_num == 0) {
							return -1;
						}
						cur = strtok(NULL, IGNORE_CHARS);
						addr = strtol(cur, NULL, 16);
						add_to_table(symtbl, name, addr, &in);
						in->sec_num = sec_num;
						in->num = num;
						cur = strtok(NULL, IGNORE_CHARS);
						char flag = cur[0];
						in->flag = flag;
					}
				break;
				case 'r':// Inside rel table
					if(strcmp(sec_name,cur) == 0){
						inside = 's';
					}else{//build relocation table
						uint32_t addr = strtol(cur, NULL, 16);
						cur = strtok(NULL, IGNORE_CHARS);
						char type = cur[0];
						cur = strtok(NULL, IGNORE_CHARS);
						uint32_t sym_num = strtol(cur, NULL, 10);
						add_rel_symbol(*(reltbls[i-1]), addr, type, sym_num);
					}
				break;
				case 's':// Inside section content
					while(cur != NULL){
						uint8_t byte = strtol(cur, NULL, 16);
						reltbls[i-1]->section_content[reltbls[i-1]->ind++] = byte;
						cur = strtok(NULL, IGNORE_CHARS);
					}
				break;
			}
		}
	}
	return 0;
}

int prepare_emulation(vector<vector<Symbol*>>& symtbl,vector<RelTable*>& reltbls, page_t** page_table){
	size_t sz = symtbl[NUM_BUCKETS - 1].size();
	for(size_t i =0; i < NUM_BUCKETS - 1; i++){
		for(size_t j = 0; j < symtbl[i].size(); j++){
			if(symtbl[i][j]->sec_num == 0){
				write_to_log("Error: there are undefined symbols, namely: %s\n", symtbl[i][j]->name);
				return -1;
			}
		}
	}
	page_t* page_entry;
	Symbol* seg;
	page_t* first_page;
	uint32_t base;
	size_t num_pages;
	int rel_ind = 0;
	for(size_t i = 0; i < sz; i++){
		seg = symtbl[NUM_BUCKETS - 1][i];
		if(char_vector_contains(seg->flags, 'A')){
			//assign (a) page(s) to sections and with it the new address of the section in symbol table

			int seg_page = seg->addr & PAGE;
			if(char_vector_contains(seg->flags, 'F')){
				//check if there is another section that is conflicting
				for(size_t j = 0; j < sz; j++){
					if(j != i){
						Symbol* sym2 = symtbl[NUM_BUCKETS- 1][j];//reconsider this checking with pages
						int sym2_page = sym2->addr & PAGE;
						if(char_vector_contains(sym2->flags, 'F') && seg_page == sym2_page){
							write_to_log("Error: two sections: %s and %s collide with their respective ORG directives\n", seg->name, symtbl[NUM_BUCKETS - 1][j]->name);
							return -2;
						}
					}
				}
				//if there aren't any conflicting sections
				base = seg->addr & PAGE;
				int page_taken = (seg->addr & PAGE_MASK) + seg->sec_size;
				num_pages = page_taken / PAGE_SIZE;
				num_pages++;
				for (size_t j = 0; j < num_pages; j++) {
					page_entry = memory_page_alloc();
					if (char_vector_contains(seg->flags, 'W')) {
						page_entry->flags.push_back('W');
					}
					if (char_vector_contains(seg->flags, 'X')) {
						page_entry->flags.push_back('X');
					}
					page_entry->base = base + j*PAGE_SIZE;
					memory_page_add(page_table, page_entry);
					page_entry->mem = new uint8_t[PAGE_SIZE];
					if (j == 0)
						first_page = page_entry;

				}
				if (char_vector_contains(seg->flags, 'P')) {
					//load the section contents
					RelTable* tbl;
					for (size_t j = 0; j < reltbls.size(); j++) {
						if (reltbls[j]->sec == seg->num) {
							tbl = reltbls[j];
						}
					}
					uint32_t k = seg->addr;
					for (size_t l = 0; l < seg->sec_size; l++) {
						first_page->mem[k] = tbl->section_content[l];
						k++;
						if (k >= PAGE_SIZE) {
							k = 0;
							first_page = first_page->next;
						}
					}
					delete[] tbl->section_content;

				}
			}
			//and correct all the global symbols addresses in symbol table
			for (size_t m = 0; m < NUM_BUCKETS - 1; m++) {
				for (size_t k = 0; k < symtbl[m].size(); k++) {
					Symbol* sym = symtbl[m][k];
						if (sym->sec_num == seg->num && sym->flag == 'G') {
							sym->addr += seg->addr;
						}
				}
			}
		}
	}
	for (size_t i = 0; i < sz; i++) {
		seg = symtbl[NUM_BUCKETS - 1][i];
		if (char_vector_contains(seg->flags, 'A')) {
			//assign (a) page(s) to sections and with it the new address of the section in symbol table


			if (!char_vector_contains(seg->flags, 'F')) {
				//assign the any base to this "free" section
				page_t* last = get_last_page_entry(*page_table);
				base = (last->base & PAGE) + PAGE_SIZE; //since org-ed sections probably won't be aligned to pages
				seg->addr = base;
				RelTable* tbl;

				//add the base to all relocation table entries so that
				//now they point to emulated memory
				if (char_vector_contains(seg->flags, 'P')) {
					for (size_t j = 0; j < reltbls.size(); j++) {
						if (reltbls[j]->sec == seg->num) {
							tbl = reltbls[j];
						}
					}
					for (size_t j = 0; j < tbl->tbl.size(); j++) {
						tbl->tbl[j]->addr += seg->addr;
					}
				}
				num_pages = seg->sec_size / PAGE_SIZE;
				num_pages++;
				for (size_t j = 0; j < num_pages; j++) {
					page_entry = memory_page_alloc();
					if (char_vector_contains(seg->flags, 'W')) {
						page_entry->flags.push_back('W');
					}
					if (char_vector_contains(seg->flags, 'X')) {
						page_entry->flags.push_back('X');
					}
					page_entry->base = base + j*PAGE_SIZE;
					memory_page_add(page_table, page_entry);
					page_entry->mem = new uint8_t[PAGE_SIZE];
					if (j == 0)
						first_page = page_entry;

				}

				if (char_vector_contains(seg->flags, 'P')) {
					//load the section content
					uint32_t k = 0;
					for (size_t l = 0; l < seg->sec_size; l++) {
						first_page->mem[k] = tbl->section_content[l];
						k++;
						if (k >= PAGE_SIZE) {
							k = 0;
							first_page = first_page->next;
						}
					}
					delete[] tbl->section_content;
				}
				//and correct all the global symbols addresses in symbol table
				for (size_t m = 0; m < NUM_BUCKETS - 1; m++) {
					for (size_t k = 0; k < symtbl[m].size(); k++) {
						Symbol* sym = symtbl[m][k];
							if (sym->sec_num == seg->num && sym->flag == 'G') {
								sym->addr += seg->addr;
							}
					}
				}
			}

		}
	}

	rel_ind = 0;
	// go through each section that is Present and go
	// through its respective relocation table and do the fixing
	for(size_t i = 0; i < sz; i++){
		page_t* page_entry;
		Symbol* seg = symtbl[NUM_BUCKETS - 1][i];

		if(char_vector_contains(seg->flags, 'P')){
			RelTable* tbl;
			for (size_t j = 0; j < reltbls.size(); j++) {
				if (reltbls[j]->sec == seg->num) {
					tbl = reltbls[j];
					break;
				}
			}
			size_t len = tbl->tbl.size();
			for(size_t j = 0; j < len; j++){
				uint32_t sym_num = tbl->tbl[j]->sym_num; //
				char type = tbl->tbl[j]->type; //type of relocation have to subtract the address and add symvalue
				uint32_t addr = tbl->tbl[j]->addr; //address on which four bytes need to be fixed
				Symbol* sym = get_symbol(symtbl, sym_num);

				uint8_t first;
				memory_read_byte_c(*page_table, addr,first);
				uint8_t second;
				memory_read_byte_c(*page_table, addr+1,second);
				uint8_t third;
				memory_read_byte_c(*page_table, addr+2,third);
				uint8_t fourth;
				memory_read_byte_c(*page_table, addr+3,fourth);

				int32_t ch = (fourth<<24) | (third<<16) | (second<<8) | first;
				ch += (int32_t)sym->addr;
				if(type == 'R'){
					ch-=(int32_t)addr;
				}
				memory_write_byte_c(*page_table,addr,ch&IBYTE);
				memory_write_byte_c(*page_table,addr+1,(ch&IIBYTE)>>8);
				memory_write_byte_c(*page_table,addr+2, (ch&IIIBYTE) >> 16);
				memory_write_byte_c(*page_table,addr+3, (ch&IVBYTE) >> 24);
			}
			rel_ind++;
		}
	}
	return 0;
}

void emulate(Emulate emul) {
	Core* core = emul.core;
	chrono::high_resolution_clock::time_point last_interrupt = chrono::high_resolution_clock::now();
	uint32_t instruction1;
	char c;
	uint8_t opcode;
	while (core->running) {
		unique_lock<mutex> lk(core->mutex);
		int ret = memory_read_dword_ex(core->page_table, core->pc, instruction1);
		core->pc += 4;
		if (ret == -1) {
			core->running = false;
			continue;
		}
		opcode = (instruction1 & IVBYTE) >> 24;
		int r0 = (instruction1 & R0) >> SHIFTR0;
		switch (opcode) {
			case 0x00:
				if (core->regs.regs[r0].dword[DW_IND] == 0) {

					core->running = false;
					continue;
				}
				int_inst(instruction1, core->page_table, *core, true, 0);
				continue;
			break;
			case 0x02:
				jmp_inst(instruction1, core->page_table, *core);
			break;
			case 0x03:
				call_inst(instruction1, core->page_table, *core);
			break;
			case 0x01:
				ret_inst(instruction1, core->page_table, *core);
			break;
			case 0x04:
				jz_inst(instruction1, core->page_table, *core);
			break;
			case 0x05:
				jnz_inst(instruction1, core->page_table, *core);
			break;
			case 0x06:
				jgz_inst(instruction1, core->page_table, *core);
			break;
			case 0x07:
				jgez_inst(instruction1, core->page_table, *core);
			break;
			case 0x08:
				jlz_inst(instruction1, core->page_table, *core);
			break;
			case 0x09:
				jlez_inst(instruction1, core->page_table, *core);
			break;
			case 0x10:
				load_inst(instruction1, core->page_table, *core);
			break;
			case 0x11:
				store_inst(instruction1, core->page_table, *core);
			break;
			case 0x20:
				push_inst(instruction1, core->page_table, *core);
			break;
			case 0x21:
				pop_inst(instruction1, core->page_table, *core);
			break;
			case 0x30:
				add_inst(instruction1, core->page_table, *core);
			break;
			case 0x31:
				sub_inst(instruction1, core->page_table, *core);
			break;
			case 0x32:
				mul_inst(instruction1, core->page_table, *core);
			break;
			case 0x33:
				div_inst(instruction1, core->page_table, *core);
			break;
			case 0x34:
				mod_inst(instruction1, core->page_table, *core);
			break;
			case 0x35:
				and_inst(instruction1, core->page_table, *core);
			break;
			case 0x36:
				or_inst(instruction1, core->page_table, *core);
			break;
			case 0x37:
				xor_inst(instruction1, core->page_table, *core);
			break;
			case 0x38:
				not_inst(instruction1, core->page_table, *core);
			break;
			case 0x39:
				asl_inst(instruction1, core->page_table, *core);
			break;
			case 0x3A:
				asr_inst(instruction1, core->page_table, *core);
			break;
			default:
				int_inst(0, core->page_table, *core, false, 3);
				break;
		}
		if (!core->waiting_input) {
			core->waiting_input = true;
			if (!core->in_reg_read) {
				core->cv.wait(lk);
			}
			lk.unlock();
			c = getchar();
			lk.lock();
			core->in_reg_read = false;
			memory_write_byte(core->page_table, 0xA4, c);
			core->waiting_input = false;
			if (c == '\n') {
				core->in_reg_read = true;
			}
			else {
				core->key_pressed = true; //generate interrupt
			}

		}

		//Handle interrupts
		if(!core->interrupted && opcode != 0x01){
			if (core->key_pressed) { // key press has higher priority since human typing is much slower
				//and thus there will be a balance between the occurence of these two interrupts
				core->key_pressed = false;
				int_inst(0, core->page_table, *core, false, 5);
			}else if (chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - last_interrupt).count() > 100) {
				last_interrupt = chrono::high_resolution_clock::now();
				int_inst(0, core->page_table, *core, false, 4);
			}

		}
	}
}

void print_usage() {
	printf("Usage:\n\temulator input_file\n");
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		print_usage();
		return -1;
	}
	vector<vector<Symbol*>> symtbl(NUM_BUCKETS);
	vector<RelTable*> reltbls;
	Core* core = new Core(0, 0);
	int ret;
	Emulate emul;
	emul.core = core;
	FILE* input = fopen(argv[1], "r");
	if(!input){
		write_to_log("Error: upon opening file %s\n", argv[1]);
		return 1;
	}
	/*read object file*/
	if (read_object_file(input, symtbl, reltbls) != 0) {
		write_to_log("Error: there are undefined symbols.\n");
		return -3;
	}
	Symbol* ssym = get_symbol(symtbl, "START");
	if(!ssym){
		write_to_log("Error: symbol START needs to be defined.\n");
		return -2;
	}
	if (ssym->addr == 'L') {
		write_to_log("Error: symbol START needs to be global.\n");
		return -3;
	}
	/*prepare memory*/
	ret = prepare_emulation(symtbl, reltbls, &core->page_table);
	if(ret){
		free_rel_tables(reltbls);
		free_table(symtbl);
		free_memory(core->page_table);
		return ret;
	}
	uint32_t sp;
	ret = memory_read_dword(core->page_table, 0x0, sp);
	if(ret != 0) {
		write_to_log("Error: stack pointer is not defined\n");
		free_rel_tables(reltbls);
		free_table(symtbl);
		free_memory(core->page_table);
		return ret;
	}
	uint32_t start = ssym->addr;

	core->pc = start;
	core->sp = sp;
	free_rel_tables(reltbls);
	free_table(symtbl);
	vector<thread> threads;
	for (int i = 0; i < 2; i++) {
		threads.emplace_back(emulate, emul);
	}
	//emulate(page_table, *core);
	for (int i = 0; i < 2; i++) {
		threads[i].join();
	}

	free_memory(core->page_table);
	free(core);
	return ret;
}
