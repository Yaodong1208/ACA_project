//
// Created by Anlan Yu on 11/23/19.
//
#include "../HLB/inst_simulate.h"
#include "../LRUCACHE/LRUcache.h"
#ifndef HLB_HLB_API_H
#define HLB_HLB_API_H
#define CACHE_SIZE


#include <iostream>
#include <unordered_map>
#include <functional>
#include <vector>

using namespace std;


template<class T>
class value_mem_hash{
public:
    T value;
    int counter;
    value_mem_hash () {}
    value_mem_hash(T value, int counter){
        this->value = value;
        this->counter = counter;
    }
};

template<class T1, class T2>
class HLB{
public:

    LRUCache cache();
    void hash_insert(T1 key, T2 value, int rehash_count = 0);
    T2 hash_lookup(T1 key, int rehash_count = 0);
    void hash_erase(T1 key, int rehash_count = 0);

    T1 hash_begin();
    T1 hash_end();
    T1 hash_next();
    void clear_hlb();
    void rehash_hlb(size_t indicator, int rehash_count);

    int mem_insert_count = 0;
    int cache_insert_count = 0;
    int mem_erase_count = 0;
    int cache_erase_count = 0;
    int mem_lookup_count = 0;
    int cache_lookup_count = 0;

    int hlb_insert_count = 0;
    int hlb_erase_count = 0;
    int hlb_lookup_count = 0;
    int hlb_iterator_count = 0;

private:

    vector<vector<size_t>> victim_vector;
    unordered_map<T1,value_mem_hash<T2>> mem_hash;
    bool mem_read;
    typename unordered_map<T1,value_mem_hash<T2>>::iterator IT;
    int threshold = 5;
};
template<class T1,class T2>
void HLB<T1,T2>::rehash_hlb(size_t indicator, int rehash_count) {
    //if extend is called, indicate it's extendable now indicator == 3
    //if shrink is called, indicate it's shrinkable now indicator == 4
    //first step is flush all used elements from hlb to vector by use hash_erase_inst
    //double row_end in hlb by using hash_iterator_inst
    //change hash function by changing shift, if extend then shift -- shrink then shift ++
    //for all elements we hash_insert them.
    size_t reg_key = 0;
    size_t rehash = 10;
    hash_iterator_inst(reg_key);
    hlb_iterator_count++;
    if(reg_key == NOVAL) {
        //nothing in hlb for now
        //no need to do rehash
    }else {
        //we need to do everything mentioned above
        size_t begin = reg_key;
        size_t end = -1;
        hash_iterator_inst(end);
        hlb_iterator_count++;
        size_t iter = begin;
        //flush
        //clear all victims and insert a new set of data
        int c = count();
//        printf("count before rehash: %d \n", c);
        while( iter != end ) {
            size_t reg_result;
            hash_lookup_inst(iter,reg_result); //iter now means kv pair's key need to be flush
            hlb_lookup_count++;
            std::vector<size_t> temp = {iter,reg_result};
            victim_vector.push_back(temp);
            hash_erase_inst(iter,reg_result);
            hlb_erase_count++;
            iter = 1;
            hash_iterator_inst(iter);
            hlb_iterator_count++;
        }
        size_t reg_result;
        hash_lookup_inst(iter,reg_result); //iter now means kv pair's key need to be flush
        hlb_lookup_count++;
        std::vector<size_t> temp = {iter,reg_result};
        victim_vector.push_back(temp);
        hash_erase_inst(iter, reg_result);
        hlb_erase_count++;

        int a = 0;
        a = victim_vector.size();
//        printf("victim size: %d \n", a);

        //////////////////////////  only available for extend and shrink case //////////////////////////
        //double row end
        if(indicator == 3){
            hash_iterator_inst(indicator);
            hlb_iterator_count++;
            hash_iterator_inst(rehash);
            hlb_iterator_count++;
            shift --;
        }//half row end
        else if(indicator == 4){
            hash_iterator_inst(indicator); // shrink
            hlb_iterator_count++;
            hash_iterator_inst(rehash);
            hlb_iterator_count++;
            shift ++;
        }
            ////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////   just rehash, not change size /////////////////////////////////////
        else{
            hash_iterator_inst(rehash);
            hlb_iterator_count++;
        }


        //////////////////////////////////////////////////////////////////////////////////////////////
        //insert all victims
//        for(int i =0; i <victim_vector.size();i++){
//            hash_insert(victim_vector[i][0], victim_vector[i][1], rehash_count);
//            victim_vector.erase(victim_vector.begin()+i);
//        }
        auto it = victim_vector.begin();
        for(;it!=victim_vector.end();){
            hash_insert((*it)[0],(*it)[1], rehash_count);
            if(victim_vector.size() == 0){
                break;
            }else{
                it = victim_vector.erase(it);
            }
            //printf("victim size during earse: %d \n", victim_vector.size());

        }
//        printf("count after rehash: %d \n", count());

    }


}

template<class T1,class T2>
void HLB<T1,T2>::clear_hlb() {
    //set the ROW_END to 1
    size_t reg;
    reg = 7;
    hash_iterator_inst(reg);
    hlb_iterator_count++;
    reg = 6;
    hash_iterator_inst(reg);
    hlb_iterator_count++;
}



template<class T1,class T2>
void HLB<T1,T2>::hash_insert(T1 k, T2 v, int rehash_count){
    size_t reg_key = k;
    size_t reg_v = v;
    hash_insert_inst(reg_key, reg_v);
    hlb_insert_count++;
    if (k != reg_key){
        reg_v = 2; //to check if hash_table is extendable
        hash_iterator_inst(reg_v);
        hlb_iterator_count++;
        if(reg_v == 0) {
            //in this case hash_table is extendable
            //extend the hash_table to double, then insert the kv pair want to insert
            rehash_hlb(3, rehash_count);
            hash_insert(k,v,rehash_count);
        }else{
            //hash_table not extendable
            if(rehash_count < REHASH_MAX && reg_v < LOAD_FACTOR) {
                //in this case hash_table is not extendable && the occupied rate is not high, we need to change the hash function
                //in such case, we still can rehash because rehash_count by now is still smaller then REHASH_MAX
                //only change hash_function parameter s, not changing size
                printf("occupied rate: %d \n", reg_v);
                printf("rehash_count: %d \n", rehash_count);
                rehash_count ++;
                rehash_hlb(0, rehash_count);
                hash_insert(k,v,rehash_count);

            }else {
                //we are sure hlb is crowded, so we move victim to memory and insert the kv pair which we intend to insert at first to the hlb
                //in this case, if we still do rehash we may fall into a dead loop, also it's not efficient. so we use mem_hash to do insert

                //move victim from hlb to memory
                size_t reg_value;
                hash_lookup_inst(reg_key, reg_value);
                hlb_lookup_count;
                T2 value = (T2)reg_value;
                value_mem_hash<T2> tmp(value,0);
                mem_hash[reg_key] = tmp;

                hash_erase_inst(reg_key,reg_value);
                hlb_erase_count++;

                //insert kv to hlb
                size_t key = k;
                size_t value_ = v;
                hash_insert_inst(key, value_);
                hlb_insert_count++;
            }

        }

    }
    //else means insert success
    printf("hlb count: %d, mem_hash count: %d, sum count: %d \n", count(), mem_hash.size(), count()+mem_hash.size());

//    printf("row_end: %d \n", row_end());

}
template<class T1,class T2>
T2 HLB<T1,T2>::hash_lookup(T1 key, int rehash_count) {
    size_t result;
    T2 value;
    size_t key_tmp = key;
    hash_lookup_inst(key_tmp, result);
    hlb_lookup_count++;
    if(result == NOT_FOUND){
        //means the key is not in hlb, check memory
        if(mem_hash.find(key) != mem_hash.end()){
            value = mem_hash[key].value;
            mem_hash[key].counter++;
            if (mem_hash[key].counter >= threshold){
                mem_hash.erase(key);
                hash_insert(key, value, rehash_count);
            }
            return value;
        }
        else{
            return (T2)NOT_FOUND;
        }
    }
    else{
        //means the key is in hlb return result
        return (T2)result;
    }
}
template<class T1,class T2>
void HLB<T1,T2>::hash_erase(T1 key, int rehash_count){
    size_t key_tmp = key;
    size_t value;
    //if erase success return the current occupied rate
    //else we get FAIL
    hash_erase_inst(key_tmp, value);
    hlb_erase_count++;
    if(value <= SHRINK_RATE) {
        size_t reg = 5;
        hash_iterator_inst(reg);
        hlb_iterator_count++;
        if(reg){
            //means it is shrinkable
            rehash_hlb(4, rehash_count);
        }

    }
    if (value == FAIL){
        mem_hash.erase(key);
    }
}

template<class T1,class T2>
T1 HLB<T1,T2>::hash_begin() {
    size_t reg = 0;
    hash_iterator_inst(reg);
    hlb_iterator_count++;
    T1 key = reg;
    return key;
}
template<class T1,class T2>
T1 HLB<T1,T2>::hash_end() {
    if(mem_hash.size() != 0){
        auto i = mem_hash.begin();
        auto result = mem_hash.begin();
        for(;i != mem_hash.end(); i++) {
            result = i;
        }
        return result->first;
    }else{
        size_t result = -1;
        hash_iterator_inst(result);
        hlb_iterator_count++;
        return result;
    }

}
template<class T1,class T2>
T1 HLB<T1,T2>::hash_next() {
    size_t reg = 1;
    hash_iterator_inst(reg);
    hlb_iterator_count++;
    T1 key = reg;
    if (! mem_read){
        if(reg != ACCESS_NOT_ALLOWED){
            return key;
        }else{
            mem_read = true;
            IT = mem_hash.begin();
            return IT->first;
        }

    }else{
        IT++;
        return IT->first;
    }

}
#endif //HLB_HLB_API_H
