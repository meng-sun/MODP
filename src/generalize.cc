#include <fstream>
#include <string>
#include "replacement_policies.h"
#include "memory.h"
#include "game.h"

// Calculate minimum distance.

// Open library of sequences. Choose either BFS
// or BMF. Ideally we just use unlabelled versions
// but I don't have time to build a probabilistic
// model.

std::pair<int,int> edit_distance_fast(std::string& opt_access, std::string& access) {
  int unmatched_idx = 0;
  while (opt_access[unmatched_idx] == access[unmatched_idx]) {
    unmatched_idx++;
  }

  // Return the remaining bit for b or the remaining bit for a.
  return std::pair<int, int>(opt_access.size() - unmatched_idx, unmatched_idx);
}

// This is the actual algorithm, but it might run
// worse than exact matching because I will have access
// gaps.
/*
int edit_distance(std::string& a, std::string& b) {

}*/


int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Missing arguments." << std::endl;
  }

  std::string access_string = argv[2];

  // Chomp.
  std::vector<int> accesses;
  accesses.resize(access_string.size()-1);
  for (int i=0; i<accesses.size(); ++i) {
    accesses[i] = std::stoi(access_string[i]);
  }

  Game game(accesses);
  std::unordered_map<block, bool, hash_pair> dram_mem;
  BlockMemory DRAM(10, dram_mem, dram_sz);

  BlockMemory gcpDRAM(DRAM);
  gcpDRAM.change_name("gcpDRAM");
  GeneralCachePolicy gcp(game.alphabet, gcpDRAM);
  int gcp_runtime =0;
  int generalized_runtime =0;

  std::string library_file;
  if (argv[1] == "bfs")
    library_file = "library_bfs.txt";
  else
    library_file = "library_bmf.txt";
  

  // generalizing is hard, easier to have exact sequences.

  for (int i=0; i< access_string.size(); ++i) {
  
    std::ofstream fh(library_file, std::ofstream::in);
    std::string line;

    int min_edit_distance;
    //std::unordered_map<int, int> replacement;
    int replacement_idx;
    std::vector<std::string> min_edit_sequence;
  
    int threshold_factor = 10;

    std::vector<std::string> opt_replace;
    //???

    while (std::getline(fh, line)) {
      std::pair<int, int> info = edit_distance_fast(line, access_string.substr(0, i));
      if (distance > threshold_factor) {
        if (info.first < min_edit_distance) {
          min_edit_distance = info.first;
          /*replacement.clear();
          for (int e:opt_replace[info.second])
            replacement[e] = 1;*/
          replacement_idx = info.second;
          min_edit_sequence.clear();
          min_edit_sequence.push_back(line);
        } /*else if (info.first == min_edit_distance) {
          for (int e:opt_replace[info.second]) {
            if (replacement.count(info.second) == 1) {
              replacement[info.second]++;
            } else {
              replacement[info.second] = 1;
            }
          }*/
          min_edit_distance.push_back(line);
        }
      }
    }

    if (replacement.size() == 0) {
      //LRU
    } else {
      int max = 0;
      for (auto it=replacement.begin(); it != replacement.end(); ++it) {
        if (it->second > max)
          max = it->second;
      }
      // Replace it.
      for (int e:opt_replace[replacement_idx])
        DRAM.erase(block(e, game.get_block_size[e]), i);
      DRAM.write()
      
    }

    // LRU baseline
    gcp_runtime += gcp.access(block(game.accesses[i], game.get_block_size[game.accesses[i]]), i);
  
    fh.close()
  }

  std::cout << "Final runtimes for access: " << access_string << std::endl;
  std::cout << "Generalized_runtime: " << generalized_runtime << std::endl;
  std::cout << "GCP_runtime: " << gcp_runtime << std::endl;
}
