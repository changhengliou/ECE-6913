/*
Cache Simulator
Level one L1 and level two L2 cache parameters are read from file (block size, line per set and set per cache).
The 32 bit address is divided into tag bits (t), set index bits (s) and block offset bits (b)
s = log2(#sets)   b = log2(block size)  t=32-s-b
*/
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <bitset>
#include "cache.hh"

using namespace std;

#define __DEBUG__

//access state:
#define NA 0 // no action
#define RH 1 // read hit
#define RM 2 // read miss
#define WH 3 // Write hit
#define WM 4 // write miss


// L1 L2
// RM RM -> bring back to L1
// RM RH -> bring back to L1 from L2
// RH NA
// ----------
// WH NA
// WM WH
// WM WM
// Read miss: issue a new request to low level cache, place in empty way or evicted if hit
// Write hit: lazy
// Write no allocate: forward to low level when write miss
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

      unsigned int l1_offset = addr & (static_cast<int>(pow(2, l1_cache.offset) - 1));
      unsigned int l1_index = (addr >> l1_cache.offset) & (static_cast<int>(pow(2, l1_cache.index) - 1));
      unsigned int l1_tag = addr >> (l1_cache.offset + l1_cache.index);

      unsigned int l2_offset = addr & (static_cast<int>(pow(2, l2_cache.offset) - 1));
      unsigned int l2_index = (addr >> l2_cache.offset) & (static_cast<int>(pow(2, l2_cache.index) - 1));
      unsigned int l2_tag = addr >> (l2_cache.offset + l2_cache.index);

      L1AcceState = NA;
      L2AcceState = NA;

      // access the L1 and L2 Cache according to the trace;
      auto it = find_match(l1_cache, l1_index, addr);
      if (accesstype == "R") {
        // TODO: Implement by you:
        // read access to the L1 Cache,
        // and then L2 (if required),
        // update the L1 and L2 access state variable;
        if (it != l1_cache.data.end()) {
          L1AcceState = RH;
        } else {
          // read miss for L1
          L1AcceState = RM;
          auto it2 = find_match(l2_cache, l2_index, addr);
          if (it2 != l2_cache.data.end()) {
            // read hit for L2
            // TODO: write back to L1
            // TODO: remove data on L2
            L2AcceState = RH;
            (*it2)[l2_index].valid = false;
          } else {
            // read missed for both L1 and L2
            // TODO: write back to L1
            L2AcceState = RM;
          }

          // TODO: write back to L1
          auto empty_spot = find_first_empty_way(l1_cache.data, l1_index);
          if (empty_spot != l1_cache.data.end()) {
            // empty space found, L1 write hit
            (*empty_spot)[l1_index].addr = addr;
            (*empty_spot)[l1_index].valid = true;
          } else {
            // no space left, evict round-robbin
            auto to_be_evict = l1_cache.data[l1_cache.evictionCounter][l1_index];
            l1_cache.data[l1_cache.evictionCounter++][l1_index] = {.addr = addr, .valid=true};
            l1_cache.evictionCounter = l1_cache.evictionCounter >= l1_cache.setSize ? 0 : l1_cache.evictionCounter;
            // write data to L2
            empty_spot = find_first_empty_way(l2_cache.data, l2_index);
            if (empty_spot != l2_cache.data.end()) {
              // empty space found, L2 write hit
              (*empty_spot)[l2_index].addr = to_be_evict.addr;
              (*empty_spot)[l2_index].valid = true;
            } else {
              // no space left, evict something
              l2_cache.data[l2_cache.evictionCounter++][l2_index] = {.addr = to_be_evict.addr, .valid=true};
              l2_cache.evictionCounter = l2_cache.evictionCounter >= l2_cache.setSize ? 0 : l2_cache.evictionCounter;
            }
          }
        }
      } else {
        // TODO: Implement by you:
        // write access to the L1 Cache,
        // and then L2 (if required),
        // update the L1 and L2 access state variable;
        // <valid, ...., tag>
        if (it != l1_cache.data.end()) {
          (*it)[l1_index].addr = addr; // set valid bit to 1
          (*it)[l1_index].valid = true;
          L1AcceState = WH;
        } else {
          it = find_first_empty_way(l1_cache.data, l1_index);
          if (it != l1_cache.data.end()) {
            (*it)[l1_index].addr = addr;
            (*it)[l1_index].valid = true;
            L1AcceState = WH;
          } else {
            // no empty spot left, WRITE_MISS
            // WRITE_NO_ALLOCATE, do nothing!!
            L1AcceState = WM;
            auto it2 = find_match(l2_cache, l2_index, addr);
            if (it2 != l2_cache.data.end()) {
              (*it2)[l2_index].addr = addr;
              (*it2)[l2_index].valid = true;
              L2AcceState = WH;
            } else {
              it2 = find_first_empty_way(l2_cache.data, l2_index);
              if (it2 != l2_cache.data.end()) {
                (*it2)[l2_index].addr = addr;
                (*it2)[l2_index].valid = true;
                L2AcceState = WH;
              } else {
                // L2 no empty spot left, WRITE_MISS
                L2AcceState = WM;
                cout << "Write missed for both L1 and L2: " << xaddr << ", [L1]index = " << l1_index
                     << ", [L2]index = " << l2_index << endl;
              }
            }
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
