#include "replacement_policies.h"
#include "memory.h"
#include "game.h"
#include <fstream>
#include <string>
#include <sstream>

static const bool COLLECT_DATA=true;

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

// a must be opt
// b must be access
std::pair<int,int> edit_distance_slow(const std::string& a, const std::string& b) {
  std::vector<std::vector<int>> dp;
  int B = b.size()+1;
  int A = a.size()+1;
  
  int swap_penalty = 2;
  int addrem_penalty = 1;
  
  // extend it by 1 on both sides in order
  // to include the empty string
  dp.resize(A);
  for (auto& e: dp)
    e.resize(B);
  
  // penalize these guys harder
  for(int i=0; i<B; ++i) dp.at(0).at(i) = i*5;
  for (int i=0; i<A; ++i) dp.at(i).at(0)=i;

  for (int i=1; i<A; i++) {
    for (int j=1; j<B; j++) {
      if (a.at(i-1) == b.at(j-1)) {
        dp.at(i).at(j) = dp.at(i-1).at(j-1); 
      } else {
        // add/remove to one or the other string
        int m = std::min(dp.at(i).at(j-1) +addrem_penalty,
                         dp.at(i-1).at(j) +addrem_penalty);
        // swap
        // add distance to swap penalty
        dp.at(i).at(j) = std::min(m, dp.at(i-1).at(j-1)+swap_penalty);
      }
    }
  }

  // now to figure out the path
  int i=A-1;
  int j=B-1;
  while (j == B-1) {
    int i_, j_;
    if(dp.at(i-1).at(j) < dp.at(i).at(j-1)) {
      i_ = i-1;
      j_ = j;
    } else {
      i_ = i;
      j_ = j-1;
    }
    if(dp.at(i_).at(j_) < dp.at(i-1).at(j-1)) {
      i = i_;
      j = j_;
    } else {
      i--;
      j--;
    }
  }

  /*
  std::cout << " i j " << i << " " << j << std::endl;
  for (auto& e: dp) {
    for (auto& f :e)
      std::cout << f << " ";
    std::cout << std::endl;
  }*/

  int replacement = i-1;
  std::pair<int,int> ret(dp.at(A-1).at(B-1), replacement);

  return ret;
}


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
    block b = block(accesses[i], game.get_block_size[accesses[i]]);
    int read_time = generalized_copy.mem.read(b);
    if (read_time != 0) {
        generalized_copy.update_recency(b, i);
        continue;
    }
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
      std::pair<int, int> info = edit_distance_fast(line, access_string.substr(0, i+1));
      //std::pair<int, int> info = edit_distance_slow(line, access_string.substr(0, i+1));
      //std::cout << "from library line: " << line << " we have: " << info.first << "," << info.second << std::endl;
      //std::cout << "exited edit_distance" << std::endl;
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

    //block b = block(accesses[i], game.get_block_size[accesses[i]]);

    bool not_feasible = false;
    for (int e:opt_replace[replacement_idx]) {
      int f = generalized_copy.mem.sim_erase(block(e, game.get_block_size[e]));
      if (f==0) not_feasible = true;
    }
    if (not_feasible || (replacement_idx <= 0)) {
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

      int replaced_time = 0;
      // Replace it.
      for (int e:opt_replace[replacement_idx]) {
        replaced_time += generalized_copy.mem.erase(block(e, game.get_block_size[e]));
      }


      int f = generalized_copy.mem.write(b);
      if(f==0) {
        for (int e:opt_replace[replacement_idx]) {
          generalized_copy.mem.write(block(e, game.get_block_size[e]));
        }
        std::cout << "[DRAM used LRU]" << std::endl;
        generalized_runtime += generalized_copy.access(b, i);
      } else {
        generalized_runtime += replaced_time;
        generalized_runtime += f;
        generalized_copy.update_recency(b, i);
      }
      
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

  if (COLLECT_DATA) {
    std::ofstream fh_out("analysis.txt", std::fstream::app);
    fh_out << generalized_runtime << "," << gcp_runtime << std::endl;
    fh_out.close();
  }
}
