#include "memory.h"
#include "replacement_policies.h"
#include <iostream>

size_t dram_sz = 100;

size_t alphabet_sz=10;
std::vector<block> alphabet;
std::unordered_map<int, size_t> get_block_size;

static const size_t access_sz = 5;
int accesses[access_sz] = {10, 20, 30, 40, 50};

// OPT.

// Brute force OPT.

// Branch and bound OPT.
int beam_search(int idx, BlockMemory& mem) {
  block b = block(accesses[idx], get_block_size[accesses[idx]]);

  int access_time = mem.read(b);
  if (access_time !=0) return access_time;

  std::unordered_map<block, bool, hash_pair> data = mem.get();

  std::vector<std::pair<block, int>> cache;

  for (auto it = data.begin(); it != data.end(); ++it) {
    int i = idx;
    while(i<access_sz && accesses[i]!=it->first.first) {
      i++;
    }
    cache.push_back(std::pair<block, int>(it->first, i));
  }

  std::sort(cache.begin(), cache.end(),
            [](const std::pair<block, size_t>& a,
               const std::pair<block, size_t>& b) {
    return a.second < b.second;
  });

  while (mem.get_free_mem() < b.second) {
    access_time += mem.erase(cache.back().first);
    cache.pop_back();
  }

  access_time += mem.write(b);
  return access_time;
}

// Best-search OPT.

// MCMC OPT.


int main(){
  //Generate alphabet.
  for (int i=0; i<alphabet_sz; ++i) {
    block new_b(i,9*(i+1));
    get_block_size[i] = 9*(i+1);
    alphabet.push_back(new_b);
  }

  // Generate accesses.

  std::unordered_map<block, bool, hash_pair> dram_mem;
  //Memory nvram()
  BlockMemory DRAM(10, dram_mem, dram_sz);
  std::cout << DRAM.write(alphabet.front()) << std::endl; 


}
