#include "replacement_policies.h"
#include "memory.h"
#include "game.h"
#include <fstream>
#include <string>
#include <sstream>

// Calculate minimum distance.

// Open library of sequences. Choose either BFS
// or BMF. Ideally we just use unlabelled versions
// but I don't have time to build a probabilistic
// model.

std::pair<int,int> edit_distance_fast(const std::string& opt_access, const std::string& access) {
  int unmatched_idx = 0;
  while (opt_access[unmatched_idx] == access[unmatched_idx]) {
    unmatched_idx++;
  }

  // Return the remaining bit for b or the remaining bit for a.
  //unmatched_idx--;
  return std::pair<int, int>(access.size() - unmatched_idx, unmatched_idx);
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

  std::cout << "Parsing accesses:" << std::endl;
  // Chomp.
  std::vector<int> accesses;
  std::istringstream iss(access_string);
  std::string result;
  while (std::getline(iss, result, ' ')) {
    if (result != "_") {
      accesses.push_back(std::stoi(result));
    }
  }
  for (int i:accesses) std::cout << i << " ";
  std::cout << std::endl;

  std::unordered_map<block, bool, hash_pair> dram_mem;
  BlockMemory DRAM(10, dram_mem, dram_sz);
  std::cout << "Creating game." << std::endl;

  Game game(accesses);

  std::cout << "Creating Generalized with Backup LRU." << std::endl;
  GeneralCachePolicy generalized_copy(game.alphabet, DRAM);

  std::cout << "Creating LRU." << std::endl;
  BlockMemory gcpDRAM(DRAM);
  gcpDRAM.change_name("gcpDRAM");
  GeneralCachePolicy gcp(game.alphabet, gcpDRAM);
  int gcp_runtime =0;
  int generalized_runtime =0;

  std::cout << "Reading library files." << std::endl;
  std::string library_file;
  if (argv[1] == std::string("bfs")) {
    library_file = "library_bfs.txt";
  } else if (argv[1] == std::string("bmf")) {
    library_file = "library_bmf.txt";
  } else {
    std::cerr << "Temporal workload type: " << argv[1] << " not recognized." << std::endl;
    exit(1);
  }

  // generalizing is hard, easier to have exact sequences.

  std::cout << "Start computation.\n" << std::endl;
  for (int i=0; i<game.access_sz; ++i) {
    std::cout << "Accessing: " << game.accesses[i] << std::endl;
    std::ifstream fh(library_file, std::ifstream::in);
    std::string line;

    // Edit distance is based on the entire string, rather
    // than individual ids. This is because it doesn't make
    // a difference in this algorithm.
    int min_edit_distance = access_string.size();

    //std::unordered_map<int, int> replacement;
    int replacement_idx = -1;
    std::vector<std::string> min_edit_sequence;
  
    int threshold_factor = access_string.size(); //thresholding needs to be based on max

    std::unordered_map<int,std::vector<int>> opt_replace;

    while (std::getline(fh, line)) {
      std::pair<int, int> info = edit_distance_fast(line, access_string.substr(0, i));
      //std::cout << "from library line: " << line << " we have: " << info.first << "," << info.second << std::endl;
      if (info.first < threshold_factor) {
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
          }
          min_edit_distance.push_back(line);
        }*/
      }

      std::cout << "For line: " << line << "| edit distance is " << info.first << "," << info.second << std::endl;

      // Would be faster to separate this from main loop.
      std::getline(fh, line);
      std::istringstream iss_idx(line);
      std::string result_idx;
      int idx_counter = 0;
      while (std::getline(iss_idx, result_idx, ' ')) {
        std::istringstream iss_blockid(result_idx);
        std::string result_blockid;
        std::vector<int> removed_blocks;
        while (std::getline(iss_blockid, result_blockid, ',')) {
          if (result_blockid != "_") {
            removed_blocks.push_back(std::stoi(result_blockid));
          }
        }
        if (removed_blocks.size() > 0)
          opt_replace[idx_counter] = removed_blocks;
        idx_counter++;
      }

      std::cout << "opt replace: " << line << std::endl;
      /*for (auto it = opt_replace.begin(); it != opt_replace.end(); it++) {
        for (auto n: it->second)
          std::cout << it->first <<"," << n;
        std::cout << " ";
      }
      std::cout << std::endl;*/
    }

    block b = block(accesses[i], game.get_block_size[accesses[i]]);
    if (replacement_idx <= 0) {
      // This is just a second thresholding factor.
      //LRU
      std::cout << "[DRAM used LRU]" << std::endl;
      generalized_runtime += generalized_copy.access(b, i);
    } else {
      /*int max = 0;
      for (auto it=replacement.begin(); it != replacement.end(); ++it) {
        if (it->second > max)
          max = it->second;
      }*/

      // Replace it.
      for (int e:opt_replace[replacement_idx]) {
        generalized_runtime += generalized_copy.mem.erase(block(e, game.get_block_size[e]));
      }

      generalized_runtime += generalized_copy.mem.write(b);
      generalized_copy.update_recency(b, i);
      
    }

    std::cout << generalized_copy.mem << std::endl;
    // LRU baseline
    gcp_runtime += gcp.access(b, i);

    std::cout << gcp.mem << std::endl;

    // Check that I can read from both.
    if (generalized_copy.mem.read(b) == 0) std::cout << "Error, did not write block." << std::endl;
    if (gcp.mem.read(b) == 0) std::cout << "Error, did not write block." << std::endl;
  
    fh.close();
  }

  std::cout << "\nFinal runtimes for access----------- " << std::endl;
  std::cout << "Generalized_runtime: " << generalized_runtime << std::endl;
  std::cout << "GCP_runtime: " << gcp_runtime << std::endl;
}
