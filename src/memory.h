#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <cstdint>
#include <vector>
/*Paging*/
typedef struct core Core;
#define IBYTE 0xFF
#define IIBYTE 0xFF00
#define IIIBYTE 0xFF0000
#define IVBYTE 0xFF000000
#define PAGE_SIZE 0x1000 // maybe I can make page size dependant on the largest section
#define PAGE_MASK 0x0FFF // This depends on the value of PAGE_SIZE
#define PAGE 0xFFFFF000

typedef struct page_t{
	uint32_t     base;  // Address in the emulated memory represented by this page
						// This address will be the search parameter
	std::vector<char>      flags; //Whatever flags pages have WX
	uint8_t*    mem;   // Pointer to the actual allocated memory
	struct page_t* next;
	page_t():base(0), flags(0), mem(NULL), next(NULL){}
}page_t;
// manage the sorted(by base) linked list of page_t's :
/*mapping emulated memory to the virtual memory which is accessible for the emulator.*/

//base - lower address of the emulated memory represented by this page;
//flags - any attributes you would like your pages to have(e.g.is it writable or executable, etc.);
//mem - pointer to the memory area actually allocated by the emulator.


void free_memory(page_t* page_table);

page_t* memory_page_alloc();

void  memory_page_add(page_t** page_table, page_t* p);

page_t* get_last_page_entry(page_t* p);

/*Memory Access Emulation*/
void memory_read_byte_c(page_t* page_table, uint32_t address, uint8_t& byte);

void memory_write_byte_c(page_t* page_table, uint32_t address, uint8_t byte);

int memory_read_byte(page_t* page_table, uint32_t address, uint8_t& byte, Core* core);

int memory_write_byte(page_t* page_table, uint32_t address, uint8_t byte);

int memory_read_word(page_t* page_table, uint32_t address, uint16_t& word);

int memory_write_word(page_t* page_table, uint32_t address, uint16_t word);

int memory_read_dword_ex(page_t* page_table, uint32_t address, uint32_t& dword);

int memory_read_dword(page_t* page_table, uint32_t address, uint32_t& dword);

int memory_write_dword(page_t* page_table, uint32_t address, uint32_t dword);


#endif