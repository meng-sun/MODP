#pragma once
#include <utility>
#include <cstring>
#include <unordered_map>
#include <map>
#include <vector>
#include <functional>
#include <iostream>
#include <string>

// Describes how memory functions.

// A block represents a unit of readable or writeable data.
// These units of data are at least 64B for DRAM and
// 256B for NVRAM. These units of data are static.

// the first element of block is its ID. The second is
// its size.
typedef std::pair<int, size_t> block;

std::ostream& operator<<(std::ostream& os, const block& b) {
  os << b.first << ", " << b.second << std::endl;
}

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
    std::string name;
    std::unordered_map<block, data_st, hash_pair> data;

  public:
    Memory(int a, std::unordered_map<block, data_st, hash_pair> d, size_t s, std::string n="DRAM") : access_time(a),
              data(d), size(s), name(n) {
      free_mem = size;
    }

    void change_name(std::string n) {
      name = n;
    }
    
    // get data
    const std::unordered_map<block, data_st, hash_pair>& get() const {
      return data;
    }

    std::string get_data_short() const {
      std::string mem_short;
      for (auto it = data.begin(); it != data.end(); ++it)
        mem_short += std::to_string(it->first.first) + " ";
      return mem_short.substr(0, mem_short.size()-2);
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

    friend std::ostream& operator<<(std::ostream& os, const Memory<bool>& m);
};

std::ostream& operator<<(std::ostream& os, const Memory<bool>& m) {
  os << "Printing contents of " << m.name << "--------------" << std::endl;
  for (auto i = m.data.begin(); i != m.data.end(); ++i)
    os << "block: " << i->first.first << " " << i->first.second << std::endl;
  os << "---------------------------------------" << std::endl;
  os << "FreeMEM: " << m.free_mem << std::endl;
  os << "---------------------------------------" << std::endl;
}

// implementation of Memory where the data structures
// are contiguous blocks of memory.
class BlockMemory : public Memory<bool> {
    bool debug = true;
  public:
    BlockMemory(int a, std::unordered_map<block, bool, hash_pair> d, size_t s, std::string n="DRAM") :
      Memory<bool>(a,d,s,n){}

    BlockMemory(const BlockMemory &m) : Memory(m) {}

    // Read time is the size of the block times the access
    // time if block is present, otherwise return 0
    size_t read(block b) {
      if(Memory::data[b]) {
        if (debug) std::cout << name << "| reading block " << b.first << " of size " << b.second << std::endl;
        return access_time;
      } else { return 0; }
    }

    // Write time is the size of the block times the access
    // time if enough space is present, otherwise return 0
    // Does not check if item exists already.
    size_t write(block b) {
      if (Memory::free_mem >= b.second) {
        if (debug) std::cout << name << "| writing block " << b.first << " of size " << b.second  << std::endl;
        Memory::data[b] = true;
        Memory::free_mem -= b.second;
        return b.second*access_time;
      }
      if (debug) std::cout << name << "| couldn't write block " << b.first << " of size " << b.second << std::endl;
      return 0;
    }

    // Write time is the size of the block times the access
    // time if enough space is present, otherwise return 0
    size_t erase(block b) {
      if (Memory::data.find(b) != Memory::data.end()) {
        if (debug) std::cout << name << "| erasing block " << b.first << " of size " << b.second << std::endl;
        Memory::data.erase(b);
        Memory::free_mem += b.second;
        return access_time;
      }
      if (debug) std::cout << name << "| didn't find block to erase " << b.first << " of size " << b.second << std::endl;
      if (debug) std::cout << name << "| Memory contents: " << Memory::data[b] << " " << Memory::get_data_short() << std::endl;
      return 0;
    } 

    size_t sim_write(block b) {
      if (Memory::free_mem >= b.second)
        return b.second*access_time;
      else return 0;
    }

    size_t sim_erase(block b) {
      if (Memory::data[b])
        return access_time;
      return 0;
    } 
};
