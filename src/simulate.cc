#include "memory.h"
#include "replacement_policies.h"
#include <iostream>
#include <queue>

size_t dram_sz = 100;

struct Game {
  static const int access_sz = 5;
  static const int alphabet_sz = 10;
  std::vector<block> alphabet;
  std::unordered_map<int, size_t> get_block_size;
  int accesses[access_sz] = {1, 2, 3, 4, 5};
};

// OPT algorithms.

/*
def OPT_DP(const block b, const std::vector<block>& cache) {
  std::vector<std::vector<block>> dp_flat;
  std::vector<block>* dp [b.second];

  for(size_t i=0; i<b.second; ++i) {
    dp[]
  }

}*/

// Slightly better than brute force OPT.
int bf_search(int idx, BlockMemory& mem, Game g) {
  if (idx < g.access_sz) {
    block b = block(g.accesses[idx], g.get_block_size[g.accesses[idx]]);
  
    int access_time = mem.read(b);
    if (access_time !=0) return access_time;

    std::unordered_map<block, bool, hash_pair> data = mem.get();

    // Using BFS to explore minimum frontier.
    // This is a bad "DP."

    std::unordered_map<int, int> explored;
    int total_elements = data.size();
    std::queue<block> bfs_frontier;
    int num_removable = 1;
    std::vector<block> removed_group;
    size_t required_mem = b.second;

    while (explored.size() < total_elements) {

      for (auto it = data.begin(); it != data.end(); ++it) {
        if (explored.find(it->first.first)==explored.end()) {
          if (it->first.second >= required_mem) {
            explored[it->first.first] = 1;
            // remove the elements and call this function again
          } else {
            bfs_frontier.push(it->first);
          }
        }
      }
      while (explored.find(bfs_frontier.front().first)!=explored.end()) {
        bfs_frontier.pop();
      }
      removed_group.push_back(bfs_frontier.front());
      required_mem -= bfs_frontier.front().second;
      explored[bfs_frontier.front().first] = 1;
      bfs_frontier.pop();
      num_removable += 1;
    }
  }
  return 0;
}

// Branch and bound OPT.
int bb_search(int idx, BlockMemory& mem, Game g) {
  block b = block(g.accesses[idx], g.get_block_size[g.accesses[idx]]);
  //block b = block(accesses[idx], get_block_size[accesses[idx]]);
  std::cout << "b is " << b.first << " , " << b.second << std::endl;

  int access_time = mem.read(b);
  if (access_time !=0) return access_time;

  std::unordered_map<block, bool, hash_pair> data = mem.get();
  std::vector<std::pair<block, int>> cache;

  for (auto it = data.begin(); it != data.end(); ++it) {
    int i = idx;
    while(i<g.access_sz && g.accesses[i]!=it->first.first) {
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
    std::cout << "removed " << cache.back().first.first << std::endl;
    access_time += mem.erase(cache.back().first);
    cache.pop_back();
  }

  access_time += mem.write(b);
  return access_time;
}

// Best-search OPT.
// Refresh approximation.
int bs_search(int idx, BlockMemory& mem, Game g) {
  block b = block(g.accesses[idx], g.get_block_size[g.accesses[idx]]);
  //block b = block(accesses[idx], get_block_size[accesses[idx]]);
  int access_time = mem.read(b);
  if (access_time !=0) return access_time;

  std::unordered_map<block, bool, hash_pair> data = mem.get();
  std::vector<std::pair<block, int>> cache;

  for (auto it = data.begin(); it != data.end(); ++it) {
    int i = idx;
    size_t weight = 0;
    while(i<g.access_sz && g.accesses[i]!=it->first.first) {
      weight += g.get_block_size[i];
      i++;
    }
    cache.push_back(std::pair<block, int>(it->first, weight));
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

// Brute force up to k steps away.
int bfk_search(int idx, BlockMemory& mem, Game g) {
  if (idx < g.access_sz) {

    block b = block(g.accesses[idx], g.get_block_size[g.accesses[idx]]);
    //block b = block(accesses[idx], get_block_size[accesses[idx]]);
  
    int access_time = mem.read(b);
    if (access_time !=0) return access_time;

    std::unordered_map<block, bool, hash_pair> data = mem.get();

    // Using BFS to explore minimum frontier.
    // This is a bad "DP."

    std::unordered_map<int, int> explored;
    int total_elements = data.size();
    std::queue<block> bfs_frontier;
    int num_removable = 1;
    std::vector<block> removed_group;
    size_t required_mem = b.second;

    while (explored.size() < total_elements) {

      for (auto it = data.begin(); it != data.end(); ++it) {
        if (explored.find(it->first.first)==explored.end()) {
          if (it->first.second >= required_mem) {
            explored[it->first.first] = 1;
            // remove the elements and call this function again
          } else {
            bfs_frontier.push(it->first);
          }
        }
      }
      while (explored.find(bfs_frontier.front().first)!=explored.end()) {
        bfs_frontier.pop();
      }
      removed_group.push_back(bfs_frontier.front());
      required_mem -= bfs_frontier.front().second;
      explored[bfs_frontier.front().first] = 1;
      bfs_frontier.pop();
      num_removable += 1;
    }
  }
  return 0;
}


// Complex OPT.


// Random OPT.
// MCMC OPT.


int main(){

  Game game;

  //Generate alphabet.
  for (int i=0; i<game.alphabet_sz; ++i) {
    block new_b(i,9*(i+1));
    std::cout << "created new block: " << new_b.first << "," << new_b.second << std::endl;
    game.get_block_size[i] = 9*(i+1);
    game.alphabet.push_back(new_b);
  }


  std::unordered_map<block, bool, hash_pair> dram_mem;
  //Memory nvram()
  BlockMemory DRAM(10, dram_mem, dram_sz);
  std::cout << DRAM.write(game.alphabet.front()) << std::endl; 

  int total_run_time = 0;
  for(int i=0; i<game.access_sz; ++i) {
    total_run_time += bb_search(i, DRAM, game);
  }
  std::cout << total_run_time << std::endl;
}
