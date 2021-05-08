//
//  memmgr.c
//  memmgr
//
//  Created by William McCarthy on 17/11/20.
//  Copyright © 2020 William McCarthy. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define ARGC_ERROR 1
#define FILE_ERROR 2
#define BUFLEN 256
#define FRAME_SIZE 256
#define PAGE_SIZE 256
#define TLB_SIZE 16


//-------------------------------------------------------------------
unsigned getpage(unsigned x) { return (0xff00 & x) >> 8; }

unsigned getoffset(unsigned x) { return (0xff & x); }

void getpage_offset(unsigned x) {
  unsigned  page   = getpage(x);
  unsigned  offset = getoffset(x);
  printf("x is: %u, page: %u, offset: %u, address: %u, paddress: %u\n", x, page, offset,
         (page << 8) | getoffset(x), page * 256 + offset);
}
struct tlbTable { 
  unsigned pageNum;
  unsigned frameNum;
};

int main(int argc, const char* argv[]) {
  FILE* fadd = fopen("addresses.txt", "r");    // open file addresses.txt  (contains the logical addresses)
  if (fadd == NULL) { fprintf(stderr, "Could not open file: 'addresses.txt'\n");  exit(FILE_ERROR);  }

  FILE* fcorr = fopen("correct.txt", "r");     // contains the logical and physical address, and its value
  if (fcorr == NULL) { fprintf(stderr, "Could not open file: 'correct.txt'\n");  exit(FILE_ERROR);  }
  
  FILE *BACKING = fopen("BACKING_STORE.bin", "rb");
  if (BACKING == NULL) { fprintf(stderr, "Could not open file: 'BACKING_STORE.bin'\n");  exit(FILE_ERROR);  }

  char buf[BUFLEN];
  unsigned   page, offset, physical_add, frame = 0;
  unsigned   logic_add, value;                  // read from file address.txt
  unsigned   virt_add, phys_add, value2;  // read from file correct.txt
  int flag, tlbcount, addresscount, pageFault, tlbSize = 0;
  size_t tlbIndex = 0;

  int physical_mem[FRAME_SIZE * PAGE_SIZE];
  int pageTable[PAGE_SIZE];
  
  memset(pageTable, -1, PAGE_SIZE*sizeof(int));
  struct tlbTable tlb[TLB_SIZE];
  while(fscanf(fadd, "%d", &logic_add) == 1){
	  ++addresscount;
	  page = getpage(logic_add);
	  offset = getoffset(logic_add);
	  flag = -1;
	  
	  for(int i = 0; i < tlbSize; ++i){
			if(tlb[i].pageNum == page)
			{
				flag = tlb[i].frameNum;
				physical_add = flag*256 + offset;
			}
	  }
	  if(flag != -1)
	  {
			++tlbcount;
	  }
	  else if(pageTable[page] == -1)
	  {
			fseek(BACKING, page*256, SEEK_SET);
			fread(buf, sizeof(char), 256, BACKING);
			pageTable[page] = frame;

			for(int i = 0; i < 256; ++i)
			{
				physical_mem[frame*256 + i] = buf[i];
			}
			++pageFault;
			++frame;

			if(tlbSize == 16) tlbSize--; //FIFO

			for(tlbIndex = tlbSize; tlbIndex > 0; tlbIndex--)
			{
				tlb[tlbIndex].pageNum = tlb[tlbIndex-1].pageNum;
				tlb[tlbIndex].frameNum = tlb[tlbIndex-1].frameNum;
			}

			if (tlbSize <= 15) ++tlbSize;

			tlb[0].pageNum = page;
			tlb[0].frameNum = pageTable[page];
			physical_add = pageTable[page]*256 + offset;
	  }
	  else{
		    physical_add = pageTable[page]*256 + offset;
	  }
	  value = physical_mem[physical_add];
	  printf("Virtual Address: %d Physical Address: %d Value: %d \n", logic_add, physical_add, value);
	  fscanf(fcorr, "%s %s %d %s %s %d %s %d", buf, buf, &virt_add, buf, buf, &phys_add, buf, &value2);
	  assert(physical_add == phys_add);
	  assert(value == value2);
	  
  }
  --tlbcount;
  
  
  //printf("ONLY READ FIRST 20 entries -- TODO: change to read all entries\n\n");

  // not quite correct -- should search page table before creating a new entry
      //   e.g., address # 25 from addresses.txt will fail the assertion
      // TODO:  add page table code
      // TODO:  add TLB code
  /*while (frame < 20) {

    fscanf(fcorr, "%s %s %d %s %s %d %s %d", buf, buf, &virt_add,
           buf, buf, &phys_add, buf, &value);  // read from file correct.txt

    fscanf(fadd, "%d", &logic_add);  // read from file address.txt
    page   = getpage(  logic_add);
    offset = getoffset(logic_add);
    
    physical_add = frame++ * FRAME_SIZE + offset;
    
    assert(physical_add == phys_add);*/
    
    // todo: read BINARY_STORE and confirm value matches read value from correct.txt
    
    //printf("logical: %5u (page: %3u, offset: %3u) ---> physical: %5u -- passed\n", logic_add, page, offset, physical_add);
    //if (frame % 5 == 0) { printf("\n"); }
  
  fclose(fcorr);
  fclose(fadd);
  //printf("Num of addresses: %d\n", addresscount);
  //printf("Num of page faults: %d\n", pageFault);
  //printf("ONLY READ FIRST 20 entries -- TODO: change to read all entries\n\n");
  
  printf("ALL logical ---> physical assertions PASSED!\n");
  //printf("!!! This doesn't work passed entry 24 in correct.txt, because of a duplicate page table entry\n");
  //printf("--- you have to implement the PTE and TLB part of this code\n");

//  printf("NOT CORRECT -- ONLY READ FIRST 20 ENTRIES... TODO: MAKE IT READ ALL ENTRIES\n");

  printf("\n\t\t...done.\n");
  return 0;
}
