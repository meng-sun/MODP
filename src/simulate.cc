#include "memory.h"
#include "replacement_policies.h"
#include <iostream>

// OPT.

int main(){
  size_t dram_sz = 100;

  //Generate alphabet.
  size_t alphabet_sz=10;
  std::vector<block> alphabet;
  for (int i=0; i<alphabet_sz; ++i) {
    block new_b(i,9*(i+1));
    alphabet.push_back(new_b);
  }

  std::unordered_map<block, bool, hash_pair> dram_mem;
  //Memory nvram()
  BlockMemory DRAM(10, dram_mem, dram_sz);
  std::cout << DRAM.write(alphabet.front()) << std::endl; 
  
  //access geb
  //timer
}
