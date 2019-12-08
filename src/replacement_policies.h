#pragma once
#include "memory.h"
#include <algorithm>

// Cache policies.

// Implements least recently used, least frequently used,
// as well as mix between recency and frequency.
template <typename MemType>
class GeneralCachePolicy{
  std::unordered_map<block, size_t, hash_pair> recency; 
  std::unordered_map<block, size_t, hash_pair> frequency; 
  MemType mem;

  
  // Generally for LRU we want our algorithm to do something
  // along the lines of:
  //   First pass to check if we can remove one thing
  //   of size >= our block then iterate on removing
  //   two things, three, etc.

  // However this is ugly
  void LRU_penalize(std::vector<std::pair<block, size_t>>& cache, block b) {
    std::unordered_map<block, bool, hash_pair>& data = mem.get();
    for (std::unordered_map<block, bool, hash_pair>::iterator it = data.begin();
         it != data.end(); ++it)
      cache.push_back(std::pair<block, size_t>(it->first, recency[it->first]));

    for (auto& e: cache) {
      e.second = e.second*std::pow(b.second,2)/std::pow(e.first.second,2);
    }
  }

  void LFU_penalize(std::vector<std::pair<block, size_t>>& cache, block b) {
    std::unordered_map<block, bool, hash_pair>& data = mem.get();
    for (std::unordered_map<block, bool, hash_pair>::iterator it = data.begin();
         it != data.end(); ++it)
      cache.push_back(std::pair<block, size_t>(it->first, frequency[it->first]));

    for (std::pair<block, size_t>& e: cache) {
      e.second = e.second*std::pow(b.second,2)/std::pow(e.first.second,2);
    }
  }

  // TODO
  void LRFU_penalize(std::vector<std::pair<block, size_t>>& cache, block b) {}

  public:
    // Initialize every data to 0 except the data that is
    // currently in the memory, which is set to 1
    GeneralCachePolicy(std::vector<block> dataset, MemType m) :
                     mem(m) {
      for (block& b: dataset) {
        recency[b]=0;
        frequency[b]=0;
      }
  
      std::unordered_map<block, bool, hash_pair>& data = mem.get();
      for (std::unordered_map<block, bool, hash_pair>::iterator it =
           data.begin(); it != data.end(); ++it) {
        recency[it->first]=1;
        frequency[it->first]=1;
      }
    }
  int access(block b, size_t time) {
    int access_time = mem.read(b);

    if (access_time != 0) {
      recency[b] = time; 
      frequency[b] += 1;
      return access_time;

    } else {
      assert(b.second <= mem.get_size());

      std::vector<std::pair<block, size_t>> cache;

      LRU_penalize(cache, b);

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
      frequency += 1;
      recency[b] = time;
      return access_time;
    }
  }
};

