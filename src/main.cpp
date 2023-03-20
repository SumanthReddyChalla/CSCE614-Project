#include "mem.cpp"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>

using namespace std;

int main(int argc, char **argv) {
  if (argc != 6) {
    printf("Incorrect number of arguments %d", argc);
    return 0;
  }

  std::string replacement_policy_method = argv[1];
  int cache_size = std::atoi(argv[2]);
  int line_size = std::atoi(argv[3]);
  int associativity = std::atoi(argv[4]);
  std::string input_file = argv[5];

  // TODO
  int sets = ceil(cache_size / (line_size * associativity));
  int cache_lines = ceil(cache_size / (line_size));
  cache_system cache_system_obj(line_size, sets, associativity);
  replacement_policies *replacement_policy;

  if (replacement_policy_method == "LRU") {
    replacement_policy = new lru_replacement_policy(sets, associativity);
  } else if (replacement_policy_method == "RAND") {
    replacement_policy = new rand_replacement_policy();
  } else if (replacement_policy_method == "LFU") {
    replacement_policy = new lfu_replacement_policy(sets, associativity);
  } else if (replacement_policy_method == "MRU") {
    replacement_policy = new mru_replacement_policy(sets, associativity);
  } else {
    printf("Unknown replacement policy %s", replacement_policy_method);
    return 1;
  }

  cache_system_obj.replacement_policy = replacement_policy;

  ifstream infile(input_file);

  // Check if the file was opened successfully
  if (!infile) {
    cerr << "Error: could not open input file" << endl;
    return 1;
  }

  string line;

  while (getline(infile, line)) {

    // Split the line into two columns
    string Read_W = line.substr(0, line.find(' '));
    std::string col2 = line.substr(line.find(' ') + 1);

    // Convert the columns to the desired data type
    std::string address = col2.substr(2);
    char Read_Write = Read_W.at(0);
    // Process the data as needed

    unsigned long long int addr = std::stoull(address, nullptr, 16);
    // printf("f:%d",addr);
    if (Read_Write == 'R' | Read_Write == 'W') {
      // printf("READ_WRITE:%c \n", Read_Write);
      cache_system_obj.cache_system_mem_access(addr, Read_Write);
    } else {
      break;
    }
  }

  infile.close();

  // read the data to output file

  ofstream outfile("output.txt");
  if (!outfile) { // check if the file was successfully opened
    cout << "Unable to open output file";
    return 1;
  }

  outfile << "Number of Accesses: " << cache_system_obj.perf_measure.access
          << endl;
  outfile << "Number of Hits: " << cache_system_obj.perf_measure.hits << endl;
  outfile << "Number of Misses: " << cache_system_obj.perf_measure.misses
          << endl;
  outfile << "Number of write_back: "
          << cache_system_obj.perf_measure.write_back << endl;
  outfile << "Cache Hit Ratio "
          << (float)cache_system_obj.perf_measure.hits/cache_system_obj.perf_measure.access << endl;

  outfile.close();
}
