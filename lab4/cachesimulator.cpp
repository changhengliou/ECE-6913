/*
Cache Simulator
Level one L1 and level two L2 cache parameters are read from file (block size, line per set and set per cache).
The 32 bit address is divided into tag bits (t), set index bits (s) and block offset bits (b)
s = log2(#sets)   b = log2(block size)  t=32-s-b
*/
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <bitset>

using namespace std;

#define __DEBUG__

//access state:
#define NA 0 // no action
#define RH 1 // read hit
#define RM 2 // read miss
#define WH 3 // Write hit
#define WM 4 // write miss


struct config {
  int L1blocksize;
  int L1setsize;
  int L1size;
  int L2blocksize;
  int L2setsize;
  int L2size;
};

__wrap_iter<vector<unsigned int, allocator<unsigned int>> *>
find_first_empty_way(vector<vector<uint32_t>>& cache, int index) {
  return find_if(begin(cache), end(cache), [&index](const auto& way){
    return way[index] >> 31 == 0;
  });
}

// TODO: cache data structure
struct cache {
  u_int32_t blockSize;
  u_int32_t setSize;
  u_int32_t cacheSize;
  u_int32_t blockEntries;
  u_int32_t offset;
  u_int32_t index;
  u_int32_t tag;
  u_int32_t evictionCounter;
  vector<vector<uint32_t>> data;

  cache(const int blockSize, const int setSize, const int cacheSize) : blockSize(blockSize), setSize(setSize),
                                                                       cacheSize(cacheSize),
                                                                       evictionCounter(0) {
    // 8 1 16
    // 16 4 32
    if (!setSize) { // fully associate
      blockEntries = 1;
      data.resize(pow(2, 10) * cacheSize / blockSize);
      for (auto &&s : data) {
        s.resize(blockEntries);
      }
    } else {
      data.resize(setSize);
      blockEntries = pow(2, 10) * cacheSize / blockSize / setSize;
      for (auto &&s : data) {
        s.resize(blockEntries);
      }
    }

    cout << "-----------------------" << endl;
    cout << "SetSize: " << data.size() << " , blockSize: " << blockSize << ",  " << blockEntries << endl;
    offset = log2(blockSize);
    index = log2(blockEntries);
    tag = 32 - offset - index;
    cout << "TagSize: " << tag << ", Offset: " << offset << ", Index: " << index << endl;
  }
};

int main(int argc, char *argv[]) {
  config cacheconfig;
  ifstream cache_params;
  string dummyLine;
#ifdef __DEBUG__
  cache_params.open("/Users/line/Desktop/nyu/Computer architecture ECE-GY 6913/lab4/cacheconfig.txt");
#else
  cache_params.open(argv[1]);
#endif
  // read config file
  // L1: 8 1 16
  // L2: 16 4 32
  while (!cache_params.eof()) {
    cache_params >> dummyLine;
    cache_params >> cacheconfig.L1blocksize;
    cache_params >> cacheconfig.L1setsize;
    cache_params >> cacheconfig.L1size;
    cache_params >> dummyLine;
    cache_params >> cacheconfig.L2blocksize;
    cache_params >> cacheconfig.L2setsize;
    cache_params >> cacheconfig.L2size;
  }

  // TODO: Implement by you:
  // initialize the hierarchy cache system with those configs
  // probably you may define a Cache class for L1 and L2, or any data structure you like
  cache l1_cache{cacheconfig.L1blocksize, cacheconfig.L1setsize, cacheconfig.L1size};
  cache l2_cache{cacheconfig.L2blocksize, cacheconfig.L2setsize, cacheconfig.L2size};

  int L1AcceState = NA; // L1 access state variable, can be one of NA, RH, RM, WH, WM;
  int L2AcceState = NA; // L2 access state variable, can be one of NA, RH, RM, WH, WM;

  ifstream traces;
  ofstream tracesout;
  string outname;

#ifdef __DEBUG__
  outname = "/Users/line/Desktop/nyu/Computer architecture ECE-GY 6913/lab4/trace.txt.out";
  traces.open("/Users/line/Desktop/nyu/Computer architecture ECE-GY 6913/lab4/trace.txt");
#else
  outname = string(argv[2]) + ".out";
  traces.open(argv[2]);
#endif
  tracesout.open(outname.c_str());

  string line;
  string accesstype;  // the Read/Write access type from the memory trace;
  string xaddr;       // the address from the memory trace store in hex;
  unsigned int addr;  // the address from the memory trace store in unsigned int;
  bitset<32> accessaddr; // the address from the memory trace store in the bitset;

  if (traces.is_open() && tracesout.is_open()) {
    while (getline(traces, line)) {   // read mem access file and access Cache

      istringstream iss(line);
      if (!(iss >> accesstype >> xaddr)) { break; }
      stringstream saddr(xaddr);
      saddr >> std::hex >> addr;
      accessaddr = bitset<32>(addr);

      u_int32_t l1_offset = addr & (static_cast<int>(pow(2, l1_cache.offset) - 1));
      u_int32_t l1_index = (addr >> l1_cache.offset) & (static_cast<int>(pow(2, l1_cache.index) - 1));
      u_int32_t l1_tag = addr >> (l1_cache.offset + l1_cache.index);

      u_int32_t l2_offset = addr & (static_cast<int>(pow(2, l2_cache.offset) - 1));
      u_int32_t l2_index = (addr >> l2_cache.offset) & (static_cast<int>(pow(2, l2_cache.index) - 1));
      u_int32_t l2_tag = addr >> (l2_cache.offset + l2_cache.index);

      L1AcceState = NA;
      L2AcceState = NA;

      // access the L1 and L2 Cache according to the trace;
      if (accesstype == "R") {
        // TODO: Implement by you:
        // read access to the L1 Cache,
        // and then L2 (if required),
        // update the L1 and L2 access state variable;

        if (any_of(begin(l1_cache.data), end(l1_cache.data), [&l1_cache, &l1_index, &l1_tag](const vector<uint32_t>& way){return way[l1_index] == (l1_tag | 0x80000000);})) {
          L1AcceState = RH;
        } else {
          // read miss for L1
          L1AcceState = RM;
          if (any_of(begin(l2_cache.data), end(l2_cache.data), [&l2_cache, &l2_index, &l2_tag](const vector<uint32_t>& way){return way[l2_index] == (l2_tag | 0x80000000);})) {
            // read hit for L2
            L2AcceState = RH;
          } else {
            // read missed for both L1 and L2
            L2AcceState = RM;
          }
          auto it = find_first_empty_way(l1_cache.data, l1_index);
          if (it == l1_cache.data.end()) {
            // no space left, eviction happened.
            l1_cache.data[l1_cache.evictionCounter++][l1_index] = l1_tag | 0x80000000;
            if (l1_cache.evictionCounter == l1_cache.setSize) l1_cache.evictionCounter = 0;
          } else {
            // find a space, just write back to L1
            (*it)[l1_index] = l1_tag | 0x80000000;
          }
          if (L2AcceState == RM) {
            auto it2 = find_first_empty_way(l2_cache.data, l2_index);
            if (it2 == l2_cache.data.end()) {
              // no space left, eviction happened.
              l2_cache.data[l2_cache.evictionCounter++][l2_index] = l2_tag | 0x80000000;
              if (l2_cache.evictionCounter == l2_cache.setSize) l2_cache.evictionCounter = 0;
            } else {
              // find a space, just write back to L1
              (*it2)[l2_index] = l2_tag | 0x80000000;
            }
          }
        }
      } else {
        // TODO: Implement by you:
        // write access to the L1 Cache,
        // and then L2 (if required),
        // update the L1 and L2 access state variable;
        // <valid, dirty, ...., tag>
        auto it = find_first_empty_way(l1_cache.data, l1_index);
        if (it != l1_cache.data.end()) {
          (*it)[l1_index] = l1_tag | 0x80000000;
          L1AcceState = WH;
        } else {
          // no empty spot left, WRITE_MISS
          // WRITE_NO_ALLOCATE, do nothing!!
          L1AcceState = WM;
          auto it2 = find_first_empty_way(l2_cache.data, l2_index);
          if (it2 != l2_cache.data.end()) {
            (*it2)[l2_index] = l2_tag | 0x80000000;
            L2AcceState = WH;
          } else {
            L2AcceState = WM;
            cout << "Write missed for both L1 and L2: " << xaddr << ", [L1]index = " << l1_index
                 << ", [L2]index = " << l2_index << endl;
          }
        }
      }

      tracesout << L1AcceState << " " << L2AcceState
                << endl;  // Output hit/miss results for L1 and L2 to the output file;
    }
    traces.close();
    tracesout.close();
  } else cout << "Unable to open trace or traceout file ";
  return 0;
}
