Steps to run the project

1. Go to the src director using command cd ./src/
2. Check the traces available in the current directory
3. Open terminal and type in the below commands used to generate results of a trace

    ./main <Replacement_policy> <cache_size> <line_size> <associativity> <tracefile>

    The replacement policy can be 'LRU', 'MRU', 'LFU' or 'RAND'
    Cache size must be a multiple of 2 and greater than line size. It represents the total size of the cache
    Line size must also be a multiple of 2. must be smaller than cache size, It represents the size of a single cache block in the cache

    Associativity is the number of cache blocks in a single set
    Trace file is the file for which we want the results

    Example command ./main LRU 32768 2048 4 trace5.txt

4. After runnning the command, the output file is generated output.txt, where we can see the simulated results