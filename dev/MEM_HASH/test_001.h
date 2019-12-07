//
// Created by Anlan Yu on 12/4/19.
//
#include <iostream>
#include <cassert>
#include <random>
#include <unordered_map>
#include <sstream>

#include "hashmap.h"
#define ITER_NUM 100000
#define KEY_RANGE 10000
//this test aims to test whether the behaviour is correct
// if we insert and erase kv_pair interleavely

#include <set>
void test(){
    int lookup_num = 0;
    int erase_num = 0;
    int insert_num = 0;
    hashmap<size_t,size_t> mymap(CACHE_SIZE);

    for(int i = 0;i<ITER_NUM;i++){
        int rate = rand(); //use this rate to determine whether insert erase or lookup, rules are shown below
        rate = rate % 10;
        if(rate == 0 || rate == 1){
            size_t key = (size_t)rand()%KEY_RANGE;
            size_t value = (size_t)rand()%KEY_RANGE;
            mymap.set(key,value);
        }else if(rate == 2 || rate == 3){
            size_t key = rand() %KEY_RANGE;
            if(mymap.contain(key))
                mymap.remove(key);
        }
        else{
                int key = rand() % KEY_RANGE;
                if(mymap.contain(key))
                    mymap.get(key);

        }
    }
    cout<<"refer memory "<<mymap.mem_lookup_count<<" times\n"<<
    "refer cache "<<mymap.cache_lookup_count<<" times\n";

}
