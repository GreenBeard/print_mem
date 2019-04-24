#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

/* Used for validation of program */
char test[] = "hello world";

long long pmod(long long num, long long mod) {
  return (num % mod + mod) % mod;
}

/* Calculates the physical address of a given virtual address */
uintptr_t find_phys_address(uintptr_t addr, long page_size) {
  pid_t pid = getpid();

  /* bad practice */
  char pagemap_str[128];
  sprintf(pagemap_str, "/proc/%d/pagemap", pid);

  FILE* pagemap_file = fopen(pagemap_str, "r");
  long page = addr / page_size;
  uintptr_t phys_addr;
  long long page_info;

  fseek(pagemap_file, page * sizeof(phys_addr), SEEK_SET);
  fread(&page_info, sizeof(page_info), 1, pagemap_file);
  fclose(pagemap_file);

  bool valid_page = 0x70000000 != 0;
  printf("Page present %d\n", valid_page);

  long long page_frame_number = page_info & 0x6FFFFFF;
  phys_addr = page_frame_number * page_size + addr % page_size;

  return phys_addr;
}

/* Reads only one byte at a time */
/* For serious readings this function should be modified to be more efficient */
char read_phys_mem(uintptr_t mem_addr, long page_size) {
  FILE* file = fopen("/dev/mem", "r");

  off_t page = mem_addr - pmod(mem_addr, page_size);

  char* mem = mmap(0, page_size, PROT_READ, MAP_PRIVATE, fileno(file), page);
  if (mem == (char*) -1) {
    perror("Error on mmap (try using a kernel module to disable /dev/mem restrictions)");
    exit(1);
    return 0;
  }
  char* mem_actual = mem + (off_t) mem_addr % page_size;
  char ret_val = *mem_actual;
  munmap(mem, page_size);
  fclose(file);

  return ret_val;
}

int main(int argc, char** argv) {
  long page_size = sysconf(_SC_PAGE_SIZE);

  void* test_str_addr = (void*) find_phys_address((uintptr_t) test, page_size);
  printf("Physical address of %p is %p\n", test, test_str_addr);

  printf("Simple print of memory: %s\n", test);
  printf("Read from physical memory print: ");

  char* c_addr = test_str_addr;
  while (true) {
    char c = read_phys_mem((uintptr_t) c_addr, page_size);
    if (c == '\0') {
      break;
    } else {
      printf("%c", c);
      ++c_addr;
    }
  }
  printf("\n");

  while (true) {
    char* mem_addr;
    scanf("%p", &mem_addr);
    printf("%x\n", (unsigned char) read_phys_mem((uintptr_t) mem_addr, page_size));
  }

  return 0;
}
