#include "memory.h"
#include "replacement_policies.h"
#include <iostream>
#include <queue>
#include "game.h"

#include <string>
#include <fstream>
#include <sstream>

static const bool OUTPUT_TO_FILE = true;
static const bool COLLECT_DATA = true;
//#include <filesystem>

/*
size_t dram_sz = 30;


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

struct Game {
  int access_sz;
  int alphabet_sz;
  std::vector<block> alphabet;
  std::unordered_map<int, size_t> get_block_size;
  std::vector<int> accesses;
};*/

// OPT algorithms.

/*
def OPT_DP(const block b, const std::vector<block>& cache) {
  std::vector<std::vector<block>> dp_flat;
  std::vector<block>* dp [b.second];

  for(size_t i=0; i<b.second; ++i) {
    dp[]
  }

}*/


// Brute force
// do not run this parallelized
void brute_force_search(int idx,
                        BlockMemory mem,
                        Game& g,
                        std::string s,
                        int total_time,
                        std::vector<std::pair<int,std::string>>& brute_force_solns,
                        bool same_level=false,
                        int lookahead_k=-1,
                        int lookahead_counter=-1) {
  if ((g.access_sz > 20) && lookahead_counter == -1) return;
    /*std::cout << "doing stuff on idx " << idx << " where s " << s;
    std::cout << " and time " << total_time << " from " << same_level;
    std::cout <<  " and lookaheads " << lookahead_k << " " << lookahead_counter << std::endl;*/
  if ((idx < g.access_sz) &&
       (((lookahead_k == -1) && (lookahead_counter == -1))
         || (lookahead_counter < lookahead_k))) {
    block b = block(g.accesses[idx], g.get_block_size[g.accesses[idx]]);
    int access_time = mem.read(b);
    if (access_time !=0) {
      if (lookahead_counter != -1) lookahead_counter++;
      // no need to do any more
      if (same_level){
        std::string new_str=s;
        new_str = new_str.substr(0, new_str.size()-1);
        new_str += " _ ";
        brute_force_search(idx+1, mem, g,
                           new_str,
                           total_time,
                           brute_force_solns, false,
                           lookahead_k, lookahead_counter);
      }else{
        brute_force_search(idx+1, mem, g, s+"_ ", total_time,
                           brute_force_solns, false, lookahead_k,
                           lookahead_counter);
      }
    }

    // try to write
    if (mem.get_free_mem() >= b.second) {
      if (lookahead_counter != -1) lookahead_counter++;
      int write_time = mem.write(b);
      if (same_level){
        std::string new_str=s;
        new_str = new_str.substr(0, new_str.size()-1);
        new_str += " ";
        brute_force_search(idx+1, mem, g,
                           new_str,
                           total_time+write_time,
                           brute_force_solns, false, lookahead_k,
                           lookahead_counter);
      }else{
        brute_force_search(idx+1, mem, g, s+"_ ",
                           total_time+write_time,
                           brute_force_solns, false, lookahead_k,
                           lookahead_counter);
      }
    } else {
      // try every combination
      std::unordered_map<block, bool, hash_pair> data = mem.get();
      for (auto it=data.begin(); it != data.end(); it++) {
        // not sure why I had that not true bug
        // maybe due to the copy
        int erase_time = mem.erase(it->first);
        brute_force_search(idx, mem, g,
                           s+std::to_string(it->first.first)+",",
                           total_time + erase_time,
                           brute_force_solns,
                           true, lookahead_k, lookahead_counter);
      }
    }
  } else {
    brute_force_solns.push_back(std::pair<int,std::string>(total_time, s));
  }
}

std::pair<int,std::string> brute_force(BlockMemory& mem, Game& g) {
  std::vector<std::pair<int, std::string>> brute_force_solutions;
  int min_time = 0;
  std::string min_solution = "";
  brute_force_search(0, mem, g, min_solution, min_time,
                     brute_force_solutions); 
  std::sort(brute_force_solutions.begin(), brute_force_solutions.end(),
            [](const std::pair<int,std::string>& a,
               const std::pair<int,std::string>& b) {
              return a.first < b.first;
            });
  if (brute_force_solutions.size() == 0){
    std::cout << "Access sequence too long for brute force." << std::endl;
    return std::pair<int, std::string> (1e6,"");
  }
  return brute_force_solutions.at(0);
}

// lookahead k brute force
std::pair<int,std::string> lookahead_k_brute_force(int idx, BlockMemory& mem, Game& g) {
  std::vector<std::pair<int, std::string>> brute_force_solutions;
  int min_time = 0;
  std::string min_solution = "";
  brute_force_search(idx, mem, g, min_solution, min_time,
                     brute_force_solutions, false, 3, 0); 
  std::sort(brute_force_solutions.begin(), brute_force_solutions.end(),
            [](const std::pair<int,std::string>& a,
               const std::pair<int,std::string>& b) {
              return a.first < b.first;
            });
  if (brute_force_solutions.size() == 0){
    std::cout << "Access sequence too long for brute force." << std::endl;
    return std::pair<int, std::string> (1e6,"");
  }
  return brute_force_solutions.at(0);
}
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
std::pair<int,std::string> bb_search(int idx, BlockMemory& mem, Game g) {
  block b = block(g.accesses[idx], g.get_block_size[g.accesses[idx]]);
  //block b = block(accesses[idx], get_block_size[accesses[idx]]);

  int access_time = mem.read(b);
  if (access_time !=0) return std::pair<int,std::string>(access_time,"_");

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

  std::string removed_blocks;
  while (mem.get_free_mem() < b.second) {
    removed_blocks += std::to_string(cache.back().first.first) + ",";
    access_time += mem.erase(cache.back().first);
    cache.pop_back();
  }

  if (removed_blocks.size() == 0)
    removed_blocks = "_,";
  access_time += mem.write(b);
  return std::pair<int,std::string>(access_time, removed_blocks.substr(0, removed_blocks.size()-1));
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
    size_t weight = (g.access_sz*(g.access_sz+1)*0.5)+1;
    while(i<g.access_sz) {
      if (g.accesses[i]!=it->first.first)
        weight -= (g.access_sz - i);
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


int main(int argc, char* argv[]){
  // can adapt over time, much cheaper to maintain
  // Let's have the dataset be the same
  // Graph input should be BLOCK_ID weight
  // Access input should be a BLOCK_ID in order of access times
  //std::string path = "/path/to/directory";
  //for (const auto & entry : std::filesystem::directory_iterator(path))
  //    // have stiff boxes.
  //    std::cout << entry.path() << std::endl;
  //    fh_graph = std::ifstream
  if (argc < 1) {
    std::cerr << "Missing access file." << std::endl;
    exit(1);
  }

  Game game;

  std::string txt_file = argv[1];
  // hack
  std::cout << txt_file << std::endl;
  if (txt_file.at(0) == '.')
    parse_Game(txt_file, game);
  else {
    std::istringstream iss(txt_file);
    std::vector<int> accesses;
    std::string result;
    while (std::getline(iss, result, ' ')) {
      accesses.push_back(std::stoi(result));
    }
    parse_Game(accesses, game);
  }
  /*std::ifstream fh_access(txt_file, std::ifstream::in);
  std::string line;

  while (std::getline(fh_access, line)) {
    game.accesses.push_back(std::stoi(line));
  }
  fh_access.close();

  // 9, 8, 7, 6, 5, 4, 3, 2, 1,
  // 8, 9, 8, 7, 6, 5, 4, 3, 2,
  // 1, 8, 9, 8, 7, 6, 5

  game.access_sz = game.accesses.size();

  // Generated.
  game.alphabet_sz = 23;
  game.get_block_size[0]=26;
  game.get_block_size[1]=1;
  game.get_block_size[2]=1;
  game.get_block_size[3]=1;
  game.get_block_size[4]=10;
  game.get_block_size[5]=2;
  game.get_block_size[6]=3;
  game.get_block_size[7]=0;
  game.get_block_size[8]=0;
  game.get_block_size[9]=0;
  game.get_block_size[10]=1;
  game.get_block_size[11]=0;
  game.get_block_size[12]=1;
  game.get_block_size[13]=1;
  game.get_block_size[14]=1;
  game.get_block_size[15]=2;
  game.get_block_size[16]=3;
  game.get_block_size[17]=1;
  game.get_block_size[18]=0;
  game.get_block_size[19]=1;
  game.get_block_size[20]=2;
  game.get_block_size[21]=1;
  game.get_block_size[22]=1;

  //Generate alphabet.
  for (int i=0; i<game.alphabet_sz; ++i) {
    if (game.get_block_size[i] != 0) {
      block new_b(i, game.get_block_size[i]);
      std::cout << "created new block: " << new_b.first << "," << new_b.second << std::endl;
      game.alphabet.push_back(new_b);
    }

    if (game.get_block_size[i] > dram_sz) {
      std::cerr << "Not enough memory." << std::endl;
      exit(1);
    }
    //block new_b(i,9*(i+1));
    //game.get_block_size[i] = 9*(i+1);
    //game.alphabet.push_back(new_b);
  }*/

  std::unordered_map<block, bool, hash_pair> dram_mem;
  //Memory nvram()
  BlockMemory DRAM(10, dram_mem, dram_sz);
  BlockMemory bbDRAM(DRAM);
  bbDRAM.change_name("bbDRAM");
  BlockMemory bsDRAM(DRAM);
  bsDRAM.change_name("bsDRAM");
  BlockMemory bfDRAM(DRAM);
  bfDRAM.change_name("bfDRAM");
  BlockMemory bflkDRAM(DRAM);
  bflkDRAM.change_name("bflkDRAM");

  std::string opt;
  int total_run_time = 0;
  std::pair<int, std::string> info;
  for(int i=0; i<game.access_sz; ++i) {
    info = bb_search(i, bbDRAM, game);
    total_run_time += info.first;
    opt += info.second + " ";
    std::cout << "info.second |" << info.second << std::endl; 
  }
  if (COLLECT_DATA) {
    std::ofstream fh_out("opt_analysis.txt", std::fstream::app);
    fh_out << total_run_time << ",";
  }
  std::cout << "bbsearch: " << total_run_time << std::endl;
  std::cout << bbDRAM << std::endl;

  total_run_time = 0;
  for(int i=0; i<game.access_sz; ++i) {
    total_run_time += bs_search(i, bsDRAM, game);
  }
  if (COLLECT_DATA) {
    std::ofstream fh_out("opt_analysis.txt", std::fstream::app);
    fh_out << total_run_time << ",";
  }
  std::cout << "bssearch: " << total_run_time << std::endl;
  std::cout << bsDRAM << std::endl;

  std::pair<int, std::string> bf = brute_force(bfDRAM, game);
  if (COLLECT_DATA) {
    std::ofstream fh_out("opt_analysis.txt", std::fstream::app);
    fh_out << bf.first << ",";
  }
  std::cout << "bfsearch: " << bf.first << std::endl;
  //std::cout << bfDRAM << std::endl;

  total_run_time = 0;
  for(int i=0; i<game.access_sz; ++i) {
    block b = block(game.accesses[i], game.get_block_size[game.accesses[i]]);
    int read_time = bflkDRAM.read(b);
    if (read_time != 0) continue;
    BlockMemory bflkDRAM_copy(bflkDRAM);
    bflkDRAM_copy.change_name("bflkDRAM_copy");
    std::pair<int, std::string> bflk = lookahead_k_brute_force(i, bflkDRAM_copy, game);
    std::cout << "bflk: " << bflk.second << std::endl;

    std::istringstream iss(bflk.second);
    std::string result;
    std::getline(iss, result, ' ');
    if (result != "_") {
      std::cout << "result: " << result << " " << (result != "_") << std::endl;
      std::istringstream iss_id(result);
      std::string result_id;
      while (std::getline(iss_id, result_id, ',')) {
        int block_id = std::stoi(result_id);
        total_run_time += bflkDRAM.erase(block(block_id, game.get_block_size[block_id]));
      }
    }
    total_run_time += bflkDRAM.write(block(game.accesses[i], game.get_block_size[game.accesses[i]]));
  }
  if (COLLECT_DATA) {
    std::ofstream fh_out("opt_analysis.txt", std::fstream::app);
    fh_out << total_run_time << ",";
  }
  std::cout << "bflksearch: " << total_run_time << std::endl;

  BlockMemory gcpDRAM(DRAM);
  gcpDRAM.change_name("gcpDRAM");
  GeneralCachePolicy gcp(game.alphabet, gcpDRAM);
  total_run_time = 0;
  for(int i=0; i<game.access_sz; ++i) {
    total_run_time += gcp.access(block(game.accesses[i], game.get_block_size[game.accesses[i]]), i);
  }
  if (COLLECT_DATA) {
    std::ofstream fh_out("opt_analysis.txt", std::fstream::app);
    fh_out << total_run_time << std::endl;
  }
  std::cout << "lrucache: " << total_run_time << std::endl;
  gcp.print_memory();

  // Know whether it is temporal or bfs.
  // min reuse distance along sizes and ids.
  // default back to LRU.

  // large chunks of data...
  // more along the times of file rewrite
  // in between hardware specific and general...

  if (OUTPUT_TO_FILE) {
    std::string access_string;
    for (int e:game.accesses)
      access_string += std::to_string(e) + " ";
    access_string = access_string.substr(0, access_string.size()-1);
  
    std::cout << "\n" << access_string << std::endl;
    //std::cout << opt << std::endl;
    std::cout << bf.second << std::endl;
  
    //std::cout << "Output to file: " << txt_file << "_opt" <<  std::endl;
    std::ofstream fh_out("library_temp.txt", std::fstream::app);
    fh_out << access_string << std::endl;
    fh_out << opt << std::endl;
    fh_out.close();
  }
}
