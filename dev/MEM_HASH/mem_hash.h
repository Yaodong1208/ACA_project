//
// Created by Anlan Yu on 12/2/19.
//

#ifndef MEM_HASH_MEM_HASH_H
#define MEM_HASH_MEM_HASH_H
#include "../HLB/inst_simulate.h"
#include <cstddef>
template<class T1, class T2>
class
template<class T1, class T2>
class MEM_HASH {
public:


    MEM_HASH() {};
    void hash_insert(T1 key, T2 value, int rehash_count = 0);
    T2 hash_lookup(T1 key, int rehash_count = 0);
    void hash_erase(T1 key, int rehash_count = 0);

    T1 hash_begin();
    T1 hash_end();
    T1 hash_next();
    void clear();
    void rehash(size_t indicator, int rehash_count);
private:
    vector<vector<size_t>> mem_hash;
    vector<vector<size_t>> victim_vector;
};
template<class T1,class T2>
T1 MEM_HASH<T1,T2>::hash_begin() {
    size_t reg = 0;
    hash_iterator_inst(reg);
    T1 key = reg;
    return key;
}


#endif //MEM_HASH_MEM_HASH_H

