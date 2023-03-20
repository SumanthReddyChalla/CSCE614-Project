#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <string>
class cache_system;

class replacement_policies {

public:
  virtual int eviction_index(unsigned int, unsigned int) { return 0; }

  virtual void cache_access(cache_system *, unsigned int, unsigned int) {
    return;
  }
};

class cache_performance_measurement {
public:
  unsigned int access;
  unsigned int hits;
  unsigned int misses;
  unsigned int write_back;

  friend class cache_system;
};

enum cache_status { INVALID = 0, EXCLUSIVE = 1, MODIFIED = 2 };

class cache_line {
public:
  unsigned int tag;
  enum cache_status status;

  friend class cache_system;
};

class cache_system {
  const static int address_bits = 48;
  unsigned int line_size, sets, associativity;
  unsigned int index_bits, tag_bits, offset_bits;
  unsigned int offset_mask, index_mask;

public:
  cache_performance_measurement perf_measure;
  replacement_policies *replacement_policy;
  cache_line *cache_lines; // Storing the cache lines in a flat array.
  cache_system(unsigned int line_size, unsigned int sets,
               unsigned int associativity) {
    this->line_size = line_size;
    this->sets = sets;
    this->associativity = associativity;
    perf_measure.access = 0;
    perf_measure.hits = 0;
    perf_measure.misses = 0;
    perf_measure.write_back = 0;
    index_bits = log2(sets);
    offset_bits = log2(line_size);
    tag_bits = address_bits - offset_bits - index_bits;
    offset_mask = 0xffffffffffff >> (address_bits - offset_bits);
    index_mask = 0xffffffffffff >> tag_bits;
    cache_lines = new cache_line[associativity * sets];
    replacement_policy = nullptr;
  }

  ~cache_system() { delete[] cache_lines; }

  void cache_system_mem_access(unsigned long long int address, char RW) {
    perf_measure.access = perf_measure.access + 1;

    unsigned int offset = address & offset_mask;
    unsigned int id = (address & index_mask) >> offset_bits;
    unsigned int tag = address >> (offset_bits + index_bits);

    cache_line *line = cache_system_find_cache_line(id, tag);

    if (line == NULL || line->status == INVALID) { // cache_miss
      perf_measure.misses = perf_measure.misses + 1;

      int invalid_index = -1;
      for (int i = 0; i < associativity; i++) {
        if (cache_lines[id * associativity + i].status == INVALID) {
          invalid_index = i;
          break;
        }
      }

      if (invalid_index < 0) {
        int evicted_index =
            replacement_policy->eviction_index(id, associativity);

        if (evicted_index<0 | evicted_index> associativity) {
          printf("Evicted Index Out of bound");
        }

        if (cache_lines[id * associativity + evicted_index].status ==
            MODIFIED) {
          perf_measure.write_back = perf_measure.write_back + 1;
        }

        invalid_index = evicted_index;
      }

      line = &cache_lines[id * associativity + invalid_index];
      line->tag = tag;
      if (RW == 'W')
        line->status = MODIFIED;
      else
        line->status = EXCLUSIVE;
    } else { // cache hit
      perf_measure.hits = perf_measure.hits + 1;
      if (RW == 'W')
        line->status = MODIFIED;
    }

    // Let the replacement policy know that the cache line was accessed. TODO
    replacement_policy->cache_access(this, id, tag);
    // printf("Misses %d, Hits:%d \n", perf_measure.misses, perf_measure.hits);

    return;
  }

  cache_line *cache_system_find_cache_line(unsigned int id,
                                           unsigned int tag_bits) {
    for (int i = 0; i < associativity; i++) {
      if (cache_lines[id * associativity + i].tag == tag_bits) {
        return &cache_lines[id * associativity + i];
      }
    }
    return NULL;
  }
};

class lru_replacement_policy : public replacement_policies {
  int **lru_count;
  int sets;
  int associativity;

public:
  lru_replacement_policy(unsigned int sets, unsigned int associativity) {
    this->sets = sets;
    this->associativity = associativity;
    // memory allocation
    lru_count = new int *[sets];
    for (int i = 0; i < sets; i++) {
      lru_count[i] = new int[associativity];
    }
    // initialize array
    for (int i = 0; i < sets; i++) {
      for (int j = 0; j < associativity; j++) {
        lru_count[i][j] = j;
      }
    }
  }

  ~lru_replacement_policy() {
    for (int i = 0; i < sets; i++) {
      delete[] lru_count[i];
    }
    delete[] lru_count;
  }

  void cache_access(cache_system *cache_system, unsigned int id,
                    unsigned int tag) {
    int temp;
    int hit = 0;
    for (int i = 0; i < associativity; i++) {
      if (cache_system->cache_lines[id * associativity + i].tag == tag) {
        // if(i != associativity-1) {
        temp = lru_count[id][i];
        hit = 1;
        //  for (int j=i; j<associativity; j++) {
        // lru_count[id][j] = lru_count[id][j+1];
        //  }
        //  lru_count[id][associativity-1]   = temp;
        break;
        //}
      }
    }

    if (hit == 1) {
      for (int i = 0; i < associativity; i++) {
        if (lru_count[id][i] == temp) {
          lru_count[id][i] = associativity - 1;
        } else if (lru_count[id][i] > temp) {
          lru_count[id][i] = lru_count[id][i] - 1;
        } else {
          lru_count[id][i] = lru_count[id][i];
        }
      }
    }
    return;
  }

  int eviction_index(unsigned int id, unsigned int associativity) {
    // return   lru_count[id][0];
    for (int i = 0; i < associativity; i++) {
      if (lru_count[id][i] == 0) {
        return i;
      }
    }
  }
};

class rand_replacement_policy : public replacement_policies {

public:
  void cache_access() { return; }

  int eviction_index(unsigned int id, unsigned int associativity) {
    return rand() % associativity + 0;
  }
};

class mru_replacement_policy : public replacement_policies {
  int *mru_count;
  int sets;
  int associativity;

public:
  mru_replacement_policy(unsigned int sets, unsigned int associativity) {
    this->sets = sets;
    this->associativity = associativity;
    // memory allocation
    mru_count = new int[sets];

    // initialize array
    for (int i = 0; i < sets; i++) {
      mru_count[i] = 0;
    }
  }

  ~mru_replacement_policy() { delete[] mru_count; }

  void cache_access(cache_system *cache_system, unsigned int id,
                    unsigned int tag) {
    for (int i = 0; i < associativity; i++) {
      if (cache_system->cache_lines[id * associativity + i].tag == tag) {
        mru_count[id] = i;
      }
    }
    return;
  }

  int eviction_index(unsigned int id, unsigned int associativity) {
    return mru_count[id];
  }
};

class lfu_replacement_policy : public replacement_policies {
  int **lfu_count;
  int sets;
  int associativity;

public:
  lfu_replacement_policy(unsigned int sets, unsigned int associativity) {
    this->sets = sets;
    this->associativity = associativity;
    // memory allocation
    lfu_count = new int *[sets];
    for (int i = 0; i < sets; i++) {
      lfu_count[i] = new int[associativity];
    }
    // initialize array
    for (int i = 0; i < sets; i++) {
      for (int j = 0; j < associativity; j++) {
        lfu_count[i][j] = 0;
      }
    }
  }

  ~lfu_replacement_policy() {
    for (int i = 0; i < sets; i++) {
      delete[] lfu_count[i];
    }
    delete[] lfu_count;
  }

  void cache_access(cache_system *cache_system, unsigned int id,
                    unsigned int tag) {
    for (int i = 0; i < associativity; i++) {
      if (cache_system->cache_lines[id * associativity + i].tag == tag) {
        lfu_count[id][i]++;
      }
    }
    return;
  }


  int eviction_index(unsigned int id, unsigned int associativity) {
    int small = lfu_count[id][0];
    int small_idx = 0;
    for (int i = 1; i < associativity; i++) {
      if (small > lfu_count[id][i]) {
        small = lfu_count[id][i];
        small_idx = i;
      }
    }
    lfu_count[id][small_idx] = 0;
    return small_idx;
  }
};