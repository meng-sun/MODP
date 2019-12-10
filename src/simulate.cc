#include "memory.h"
#include "replacement_policies.h"
#include <iostream>
#include <queue>

#include <string>
#include <iostream>
//#include <filesystem>

size_t dram_sz = 300;

struct Game {
  static const int access_sz = 25;
  static const int alphabet_sz = 10;
  std::vector<block> alphabet;
  std::unordered_map<int, size_t> get_block_size;
  int accesses[access_sz] = {9, 8, 7, 6, 5, 4, 3, 2, 1,
                             8, 9, 8, 7, 6, 5, 4, 3, 2,
                             1, 8, 9, 8, 7, 6, 5};

  // repeated patterns
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
  // can adapt over time, much cheaper to maintain
  // Let's have the dataset be the same
  // Graph input should be BLOCK_ID weight
  // Access input should be a BLOCK_ID in order of access times
  //std::string path = "/path/to/directory";
  //for (const auto & entry : std::filesystem::directory_iterator(path))
  //    // have stiff boxes.
  //    std::cout << entry.path() << std::endl;
  //    fh_graph = std::ifstream

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
  BlockMemory bbDRAM(DRAM);
  bbDRAM.change_name("bbDRAM");
  BlockMemory bsDRAM(DRAM);
  bsDRAM.change_name("bsDRAM");

  int total_run_time = 0;
  for(int i=0; i<game.access_sz; ++i) {
    total_run_time += bb_search(i, bbDRAM, game);
  }
  std::cout << "bbsearch: " << total_run_time << std::endl;
  std::cout << bbDRAM << std::endl;

  total_run_time = 0;
  for(int i=0; i<game.access_sz; ++i) {
    total_run_time += bs_search(i, bsDRAM, game);
  }
  std::cout << "bssearch: " << total_run_time << std::endl;
  std::cout << bsDRAM << std::endl;

  BlockMemory gcpDRAM(DRAM);
  gcpDRAM.change_name("gcpDRAM");
  GeneralCachePolicy gcp(game.alphabet, gcpDRAM);
  total_run_time = 0;
  for(int i=0; i<game.access_sz; ++i) {
    total_run_time += gcp.access(block(game.accesses[i], game.get_block_size[game.accesses[i]]), i);
  }
  std::cout << "lrucache: " << total_run_time << std::endl;
  gcp.print_memory();

  // Know whether it is temporal or bfs.
  // min reuse distance along sizes and ids.
  // default back to LRU.

  // large chunks of data...
  // more along the times of file rewrite
  // in between hardware specific and general...

  
}
