#include <condition_variable>

#include "emulator.h"
#include "memory.h"
#include "utils.h"

using namespace std;
/*Page Management*/

// manage the sorted(by base) vecor of pages :

/*
This function would return a pointer to an allocated page_t structure.
Don't forget to allocate real memory area of PAGE_SIZE and store a pointer to it in page_t->mem.
*/
page_t* memory_page_alloc() {
    page_t* page_s = new (nothrow) page_t();
    if(!page_s){
        alloc_fail();
    }
    return page_s;
}
/* sorted insert in page linked list
*/
void  memory_page_add(page_t** page_table, page_t* p){
    page_t* cur = *page_table;
    page_t* prev = NULL;
    if(cur == NULL){
        *page_table = p;
    }else{
        while(cur != NULL && cur->base < p->base){
            prev = cur;
            cur = cur->next;
        }
        if(prev == NULL){
            p->next = *page_table;
            (*page_table)->next = p;
        }else{
            p->next = cur;
            prev->next = p;
        }
    }
}

/*
This function releases all the resources allocated for a page.
This includes the memory which actually represents the page and
is pointed to by page_t->mem and the page_t structure itself.
*/
void free_memory(page_t* page_table){
	if(page_table == NULL) return;
    page_t* cur = page_table;
    page_t* rem;
    while(cur !=  NULL){
        rem = cur;
        cur = cur->next;
        delete[] rem->mem;
        delete rem;
    }
}


page_t* get_last_page_entry(page_t* page_table){
    if(page_table == NULL) return NULL;
    page_t* cur = page_table;
    while(cur->next != NULL){
        cur = cur->next;
    }
    return cur;
}

/*Memory Access Emulation*/

/*
We have several separated memory areas(from the emulator's point of view)
access them with couple of functions that perform read/write
operations from/to the emulated memory.*/

void memory_read_byte_c(page_t* page_table, uint32_t address, uint8_t& byte) {
	page_t* cur = page_table;
	while (cur != NULL) {
		if (cur->base <= address && address < cur->base + PAGE_SIZE) {
			address &= PAGE_MASK;
			byte = cur->mem[address];
			return;
		}
		cur = cur->next;
	}
}

void memory_write_byte_c(page_t* page_table, uint32_t address, uint8_t byte) {
	page_t* cur = page_table;
	while (cur != NULL) {
		if (cur->base <= address && address < cur->base + PAGE_SIZE) {
			address &= PAGE_MASK;
			cur->mem[address] = byte;
			return;
		}
		cur = cur->next;
	}
}

/*
This function is responsible for reading a single byte from the emulated memory
pointed by address.The read byte is returned into location pointed by byte.
It walks the linked list of pages looking for a page where page_t->base <= address && (page_t->base + PAGE_SIZE) > address.
If there is no such page, then it either allocates and adds it to the list of pages, then performs the read operation
or simply returns error(this may be helpful in order to emulate memory access violations).It is up to you to define
the behavior of this function in such situation
*/
int memory_read_byte(page_t* page_table, uint32_t address, uint8_t& byte, Core* core){
	page_t* cur = page_table;
	if (core != NULL) {
		if (address == 0xA4) {
			//signal to thread waiting after getchar that it can generate an interrupt
			if (!core->in_reg_read) {
				core->in_reg_read = true;
				core->cv.notify_one();
			}
		}
	}
    while(cur != NULL){
        if(cur->base <= address && address < cur->base+PAGE_SIZE){
            address &= PAGE_MASK;
            byte = cur->mem[address];
            return 0;
        }
        cur = cur->next;
    }
    return -1;
}
/*
This function is almost identical to the one above, except that it writes a
single byte to the emulated memory.Its behavior should be the same as memory_read_byte.
*/
int memory_write_byte(page_t* page_table, uint32_t address, uint8_t byte){
    page_t* cur = page_table;
	if (address == 0xA0) {
		//print character
		printf("%c", (char)byte);
    fflush(stdout);
	}
    while(cur != NULL){
        if(cur->base <= address && address < cur->base+PAGE_SIZE){
            address &= PAGE_MASK;
            if(char_vector_contains(cur->flags, 'W')){
                cur->mem[address] = byte;
                return 0;
            }else{
                write_to_log("Error: writing to a section which isn't writable\n");
                return -2;
            }
        }
		cur = cur->next;
    }
    return -1;
}


int memory_read_word(page_t* page_table, uint32_t address, uint16_t& word){
	page_t* cur = page_table;
	uint32_t addr = address & PAGE_MASK;
    while(cur != NULL){
        if(cur->base <= address && address < cur->base+PAGE_SIZE){
            uint8_t first = cur->mem[addr++];
            if(address + 1 >= cur->base+PAGE_SIZE){
                cur = cur->next;
            }
            uint8_t second = cur->mem[addr++];
            word = (second<<8) | first;
            return 0;
        }
        cur = cur->next;
    }
    return -1;
}

int memory_write_word(page_t* page_table, uint32_t address, uint16_t word){
    page_t* cur = page_table;
	uint32_t addr = address & PAGE_MASK;
    while(cur != NULL){
        if(cur->base <= address && address < cur->base+PAGE_SIZE){

			if(char_vector_contains(cur->flags, 'W')){
                uint8_t first = (word & IBYTE);
                uint8_t second = (word & IIBYTE)>>8;
                cur->mem[addr++] = first;
                if(address + 1 >= cur->base+PAGE_SIZE){
                    cur = cur->next;
                }
                cur->mem[addr++] = second;
                return 0;
            }else{
                write_to_log("Error: writing to a section which isn't writable\n");
                return -2;
            }
        }
		cur = cur->next;
    }
    return -1;
}

int memory_read_dword_ex(page_t* page_table, uint32_t address, uint32_t& dword) {
	page_t* cur = page_table;
	uint32_t addr = address & PAGE_MASK;
	while (cur != NULL) {
		if (cur->base <= address && address < cur->base + PAGE_SIZE) {
			if (!char_vector_contains(cur->flags, 'X')) {
				return -1;
			}
			uint8_t first = cur->mem[addr++];
			if (address + 1 >= cur->base + PAGE_SIZE) {
				cur = cur->next;
				if (!char_vector_contains(cur->flags, 'X')) {
					return -1;
				}
			}
			uint8_t second = cur->mem[addr++];
			if (address + 2 >= cur->base + PAGE_SIZE) {
				cur = cur->next;
				if (!char_vector_contains(cur->flags, 'X')) {
					return -1;
				}
			}
			uint8_t third = cur->mem[addr++];
			if (address + 3 >= cur->base + PAGE_SIZE) {
				cur = cur->next;
				if (!char_vector_contains(cur->flags, 'X')) {
					return -1;
				}
			}
			uint8_t fourth = cur->mem[addr++];
			dword = (fourth << 24) | (third << 16) | (second << 8) | first;
			return 0;
		}
		cur = cur->next;
	}
	return -1;
}

int memory_read_dword(page_t* page_table, uint32_t address, uint32_t& dword){
	page_t* cur = page_table;
	uint32_t addr = address & PAGE_MASK;
    while(cur != NULL){
        if(cur->base <= address && address < cur->base+PAGE_SIZE){
            uint8_t first = cur->mem[addr++];
            if(address + 1 >= cur->base+PAGE_SIZE){
                cur = cur->next;
            }
            uint8_t second = cur->mem[addr++];
            if(address + 2 >= cur->base+PAGE_SIZE){
                cur = cur->next;
            }
            uint8_t third = cur->mem[addr++];
            if(address + 3 >= cur->base+PAGE_SIZE){
                cur = cur->next;
            }
            uint8_t fourth = cur->mem[addr++];
            dword = (fourth<<24) | (third<<16) | (second<<8) | first;
            return 0;
        }
        cur = cur->next;
    }
    return -1;
}

int memory_write_dword(page_t* page_table, uint32_t address, uint32_t dword){
    page_t* cur = page_table;
	uint32_t addr = address & PAGE_MASK;
    while(cur != NULL){
        if(cur->base <= address && address < cur->base+PAGE_SIZE){

			if(char_vector_contains(cur->flags, 'W')){
                uint8_t first = (dword & IBYTE);
                uint8_t second = (dword & IIBYTE)>>8;
                uint8_t third = (dword & IIIBYTE)>>16;
                uint8_t fourth = (dword & IVBYTE)>>24;
                cur->mem[addr++] = first;
                if(address + 1 >= cur->base+PAGE_SIZE){
                    cur = cur->next;
                }
                cur->mem[addr++] = second;
                if(address + 2 >= cur->base+PAGE_SIZE){
                    cur = cur->next;
                }
                cur->mem[addr++] = third;
                if(address + 3 >= cur->base+PAGE_SIZE){
                    cur = cur->next;
                }
                cur->mem[addr++] = fourth;
                return 0;
            }else{
                write_to_log("Error: writing to a section which isn't writable.\n");
                return -2;
            }
        }
		cur = cur->next;
    }
    return -1;
}
