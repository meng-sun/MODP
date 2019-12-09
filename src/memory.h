#pragma once
#include <utility>
#include <cstring>
#include <unordered_map>
#include <map>
#include <vector>
#include <functional>

// Describes how memory functions.

// A block represents a unit of readable or writeable data.
// These units of data are at least 64B for DRAM and
// 256B for NVRAM. These units of data are static.

// the first element of block is its ID. The second is
// its size.
typedef std::pair<int, size_t> block;


// The Memory class is a simulation of DRAM or NVRAM.
//
//   Size refers to the maximum size of the memory.
//   free_mem refers to the amount of free memory in
//   system.
//   The access_time refers to the time cost of one unit
//   of action on the memory.
//   Data holds the data of the memory formatted by some data
//   structure called data_st.

struct hash_pair { 
    template <class T1, class T2> 
    size_t operator()(const std::pair<T1, T2>& p) const
    { 
        auto hash1 = std::hash<T1>{}(p.first); 
        auto hash2 = std::hash<T2>{}(p.second); 
        return hash1 ^ hash2; 
    } 
}; 

template <typename data_st>
class Memory{
  protected:
    size_t size;
    size_t free_mem;
    int access_time;
    std::unordered_map<block, data_st, hash_pair> data;

  public:
    Memory(int a, std::unordered_map<block, data_st, hash_pair> d, size_t s) : access_time(a),
              data(d), size(s) {
      free_mem = size;
    }

    
    // get data
    const std::unordered_map<block, data_st, hash_pair>& get() const {
      return data;
    }
    const size_t get_free_mem() const {
      return free_mem;
    }
  
    const size_t get_size() const {
      return size;
    }
    // read a block. Returns the number of access units it took
    // to read this block.
    virtual size_t read(block b) {};
  
    // write a block. This must ensure that the free_mem >=0. 
    virtual size_t write(block b) {};
  
    // erase a block.
    virtual size_t erase(block b) {};

    // returns the access_time of each of the following actions
    // but does not take the action.
    virtual size_t sim_write(block b) {};
    virtual size_t sim_erase(block b) {};
  
};

// implementation of Memory where the data structures
// are contiguous blocks of memory.
class BlockMemory : public Memory<bool> {
  public:
    BlockMemory(int a, std::unordered_map<block, bool, hash_pair> d, size_t s) :
      Memory<bool>(a,d,s){}

    BlockMemory(const BlockMemory &m) : Memory(m) {}

    // Read time is the size of the block times the access
    // time if block is present, otherwise return 0
    size_t read(block b) {
      if(Memory::data[b]) { return b.second*access_time; }
      else { return 0; }
    }

    // Write time is the size of the block times the access
    // time if enough space is present, otherwise return 0
    size_t write(block b) {
      if (free_mem >= b.second) {
        Memory::data[b] = true;
        free_mem -= b.second;
        return b.second*access_time;
      } else return 0;
    }

    // Write time is the size of the block times the access
    // time if enough space is present, otherwise return 0
    size_t erase(block b) {
      if (Memory::data[b]) {
        Memory::data.erase(b);
        free_mem += b.second;
        return access_time;
      }
      return 0;
    } 

    size_t sim_write(block b) {
      if (free_mem >= b.second)
        return b.second*access_time;
      else return 0;
    }

    size_t sim_erase(block b) {
      if (Memory::data[b])
        return access_time;
      return 0;
    } 
};
