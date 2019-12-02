//
// Created by Anlan Yu on 11/30/19.
//

#ifndef LRUCACHE_LRUCACHE_H
#define LRUCACHE_LRUCACHE_H

#include <list>
#include <unordered_map>
// We can use stl container list as a double
// ended queue to store the cache keys, with
// the descending time of reference from front
// to back and a set container to check presence
// of a key. But to fetch the address of the key
// in the list using find(), it takes O(N) time.
// This can be optimized by storing a reference
//     (iterator) to each key in a hash map.
using namespace std;

class LRUCache {

    // store keys of cache
    list<uint64_t > dq;

    // store references of key in cache
    unordered_map<uint64_t , list<uint64_t>::iterator> ma;
    uint64_t csize; // maximum capacity of cache

public:
    int inst_in_cache = 0;
    int inst_in_mem = 0;
    LRUCache(uint64_t);
    bool refer(void*);
};

// Declare the size
LRUCache::LRUCache(uint64_t n)
{
    csize = n;
}

// Refers key x with in the LRU cache
bool LRUCache::refer(void * x)
{
    uint64_t x_ = (uint64_t)x/8;  //get block address of x
    bool in_cache = false;
    // not present in cache
    for(auto item : ma) {
        uint64_t addr_ = (uint64_t)item.first; //get block address of addr
        if(x_ == addr_) {
            // cache is full
            if (dq.size() == csize) {
                // delete least recently used element
                uint64_t last = dq.back();

                // Pops the last elmeent
                dq.pop_back();

                // Erase the last
                ma.erase(last);
            }
            in_cache = true;
            break;
        }
    }

        // present in cache
    if(in_cache){
        inst_in_cache ++;
        dq.erase(ma[(uint64_t)x/8]);
    }else {
        inst_in_mem++;
    }


    // update reference
    dq.push_front((uint64_t)x/8);
    ma[(uint64_t)x/8] = dq.begin();

    return in_cache;
}
#endif //LRUCACHE_LRUCACHE_H
