#ifndef __CACHE_H__
#define __CACHE_H__

#include <iostream>
#include <vector>

using namespace std;

struct config {
  int L1blocksize;
  int L1setsize;
  int L1size;
  int L2blocksize;
  int L2setsize;
  int L2size;
};

struct info {
  unsigned int addr;
  bool valid;
};

struct cache {
  u_int32_t blockSize;
  u_int32_t setSize;
  u_int32_t cacheSize;
  u_int32_t blockEntries;
  u_int32_t offset;
  u_int32_t index;
  u_int32_t tag;
  u_int32_t evictionCounter;
  vector<vector<info>> data;

  cache(int blockSize, int setSize, int cacheSize);
};

cache::cache(int blockSize, int setSize, int cacheSize) : blockSize(blockSize),
                                                          setSize(setSize),
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

unsigned int tag(int addr, cache &_cache);

unsigned int tag(unsigned int addr, int index, int offset);

unsigned int tag(int addr, cache &_cache) {
  return tag(addr, _cache.index, _cache.offset);
}

unsigned int tag(unsigned int addr, int index, int offset) {
  return addr >> (index + offset);
}

__wrap_iter<vector<info, allocator<info>> *>
find_first_empty_way(vector<vector<info>> &cache, int index) {
  return find_if(begin(cache), end(cache), [&index](const auto &way) {
    return !way[index].valid;
  });
}

/**
 * find first valid cache with the same tags in numbers of ways
 * @param _cache - cache data structure
 * @param index - index of ways
 * @param addr - pc counter
 * @return the iterator of ways
 */
__wrap_iter<vector<info, allocator<info>> *> find_match(cache &_cache, int index, int addr) {
  return find_if(begin(_cache.data), end(_cache.data), [&](const auto &way) {
    return tag(way[index].addr, _cache) == tag(addr, _cache) && way[index].valid;
  });
}

#endif //__CACHE_H__
