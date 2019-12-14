#pragma once
#include "memory.h"
#include <string>
#include <fstream>
static const size_t dram_sz = 30;

/*
struct Game {
  static const int access_sz = 25;
  static const int alphabet_sz = 10;
  std::vector<block> alphabet;
  std::unordered_map<int, size_t> get_block_size;
  int accesses[access_sz] = {9, 8, 7, 6, 5, 4, 3, 2, 1,
                             8, 9, 8, 7, 6, 5, 4, 3, 2,
                             1, 8, 9, 8, 7, 6, 5};

  // repeated patterns
};*/

struct Game {
  int access_sz;
  int alphabet_sz;
  std::vector<block> alphabet;
  std::unordered_map<int, size_t> get_block_size;
  std::vector<int> accesses;

  Game() {
    // Generated.
    alphabet_sz = 23;
    get_block_size[0]=26;
    get_block_size[1]=1;
    get_block_size[2]=1;
    get_block_size[3]=1;
    get_block_size[4]=10;
    get_block_size[5]=2;
    get_block_size[6]=3;
    get_block_size[7]=0;
    get_block_size[8]=0;
    get_block_size[9]=0;
    get_block_size[10]=1;
    get_block_size[11]=0;
    get_block_size[12]=1;
    get_block_size[13]=1;
    get_block_size[14]=1;
    get_block_size[15]=2;
    get_block_size[16]=3;
    get_block_size[17]=1;
    get_block_size[18]=0;
    get_block_size[19]=1;
    get_block_size[20]=2;
    get_block_size[21]=1;
    get_block_size[22]=1;

    //Generate alphabet.
    for (int i=0; i<alphabet_sz; ++i) {
      if (get_block_size[i] != 0) {
        block new_b(i, get_block_size[i]);
        std::cout << "created new block: " << new_b.first << "," << new_b.second << std::endl;
        alphabet.push_back(new_b);
      }

      if (get_block_size[i] > dram_sz) {
        std::cerr << "Not enough memory." << std::endl;
        exit(1);
      }
      //block new_b(i,9*(i+1));
      //game.get_block_size[i] = 9*(i+1);
      //game.alphabet.push_back(new_b);
    }
  }

  Game(std::vector<int> a) : Game() {
    access_sz = a.size();
    accesses.resize(access_sz);
    std::copy(a.begin(), a.end(), accesses.begin());
  }
};

inline void parse_Game(std::string txt_file, Game& game) {
  std::ifstream fh_access(txt_file, std::ifstream::in);
  std::string line;

  while (std::getline(fh_access, line)) {
    game.accesses.push_back(std::stoi(line));
  }
  fh_access.close();

  // 9, 8, 7, 6, 5, 4, 3, 2, 1,
  // 8, 9, 8, 7, 6, 5, 4, 3, 2,
  // 1, 8, 9, 8, 7, 6, 5

  game.access_sz = game.accesses.size();

}

inline void parse_Game(std::vector<int> accesses, Game& game) {
  game.access_sz = accesses.size();
  game.accesses.resize(accesses.size());
  std::copy(accesses.begin(), accesses.end(), game.accesses.begin());
}
