//
// Created by Anlan Yu on 11/23/19.
//
#include "../HLB/inst_simulate.h"
#ifndef HLB_HLB_API_H
#define HLB_HLB_API_H


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

    void hash_insert(T1 key, T2 value, int rehash_count = 0);
    T2 hash_lookup(T1 key);
    void hash_erase(T1 key);

    T1 hash_begin();
    T1 hash_end();
    T1 hash_next();
    void clear_hlb();
    void rehash_hlb(size_t indicator);

private:

    vector<vector<size_t>> victim_vector;
    unordered_map<T1,value_mem_hash<T2>> mem_hash;
    bool mem_read;
    typename unordered_map<T1,value_mem_hash<T2>>::iterator it;



    int threshold = 5;


};
template<class T1,class T2>
void HLB<T1,T2>::rehash_hlb(size_t indicator) {
    //if extend is called, indicate it's extendable now
    //if shrink is called, indicate it's shrinkable now
    //first step is flush all used elements from hlb to vector by use hash_erase_inst
    //double row_end in hlb by using hash_iterator_inst
    //change hash function by changing shift, if extend then shift -- shrink then shift ++
    //for all elements we hash_insert them.
    size_t reg_key = 0;
    size_t reg;
    size_t rehash = 10;
    hash_iterator_inst(reg_key);
    if(reg_key == NOVAL) {
        //nothing in hlb for now
        reg = 3;
        hash_iterator_inst(reg);
    }else {
        //we need to do everything mentioned above
        size_t begin = reg_key;
        size_t end = -1;
        hash_iterator_inst(end);
        size_t iter = begin;
        //flush
        while( iter != end ) {
            size_t reg_result;
            hash_lookup_inst(iter,reg_result); //iter now means kv pair's key need to be flush
            std::vector<size_t> temp = {iter,reg_result};
            victim_vector.push_back(temp);
            hash_erase_inst(iter,reg_result);
            iter = 1;
            hash_iterator_inst(iter);
        }

        //////////////////////////  only available for extend and shrink case //////////////////////////
        //double row end
        if(indicator == 3){
            hash_iterator_inst(indicator);
            hash_iterator_inst(rehash);
            shift --;
        }//half row end
        else if(indicator == 4){
            hash_iterator_inst(indicator); // extend
            hash_iterator_inst(rehash);
            shift ++;
        }
            ////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////   just rehash, not change size /////////////////////////////////////
        else{
            hash_iterator_inst(rehash);
        }
        //////////////////////////////////////////////////////////////////////////////////////////////

        //insert all victims
        for(auto kv : victim_vector) {
            hash_insert(kv[0], kv[1]);
        }
    }


}

template<class T1,class T2>
void HLB<T1,T2>::clear_hlb() {
    //set the ROW_END to 1
    size_t begin = 0;
    size_t end = -1;
    size_t reg;
    reg = 6;
    hash_iterator_inst(reg);

    //clear everything inside the table
    hash_iterator_inst(begin);
    hash_iterator_inst(end);
    size_t iter = begin;
    while(iter != end){
        hash_erase_inst(iter, reg);
        iter = 1;
        hash_iterator_inst(iter);
    }

}



template<class T1,class T2>
void HLB<T1,T2>::hash_insert(T1 k, T2 v, int rehash_count){
    size_t reg_key = k;
    size_t reg_v = v;
    hash_insert_inst(reg_key, reg_v);
    if (k != reg_key){
        reg_v = 2; //to check if hash_table is extendable
        hash_iterator_inst(reg_v);
        if(reg_v != 0) {
            //in this case hash_table is extendable
            //extend the hash_table to double, then insert the kv pair want to insert
            rehash_hlb(3);
            hash_insert(k,v,rehash_count);
        }else{
            if(rehash_count < REHASH_MAX && reg_v < LOAD_FACTOR) {
                //in this case hash_table is not extendable && the occupied rate is not high, we need to change the hash function
                //in such case, we still can rehash because rehash_count by now is still smaller then REHASH_MAX
                //only change hash_function parameter s, not changing size
                rehash_hlb(0);
                hash_insert(k,v,rehash_count+1);

            }else {
                //in this case, if we still do rehash we may fall into a dead loop, also it's not efficient. so we use mem_hash to do insert
                size_t reg_value;
                hash_lookup_inst(reg_key, reg_value);
                T2 value = (T2)reg_value;
                value_mem_hash<T2> tmp(value,0);
                mem_hash[reg_key] = tmp;

                size_t key = k;
                size_t value_ = v;
                hash_insert_inst(key, value_);
            }

        }

    }
    //else means insert success

}
template<class T1,class T2>
T2 HLB<T1,T2>::hash_lookup(T1 key) {
    size_t result;
    T2 value;
    size_t key_tmp = key;
    hash_lookup_inst(key_tmp, result);
    if(result == NOT_FOUND){
        //means the key is not in hlb, check memory
        if(mem_hash.find(key) != mem_hash.end()){
            value = mem_hash[key].value;
            mem_hash[key].counter++;
            if (mem_hash[key].counter >= threshold){
                mem_hash.erase(key);
                hash_insert(key, value);
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
void HLB<T1,T2>::hash_erase(T1 key){
    size_t key_tmp = key;
    size_t value;
    //if erase success return the current occupied rate
    //else we get FAIL
    hash_erase_inst(key_tmp, value);
    if(value <= SHRINK_RATE) {
        size_t reg = 5;
        hash_iterator_inst(reg);
        if(reg){
            //means it is shrinkable
            rehash_hlb(4);
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
    T1 key = reg;
    return key;
}
template<class T1,class T2>
T1 HLB<T1,T2>::hash_end() {
    auto result = mem_hash.end();
    return result->first;
}
template<class T1,class T2>
T1 HLB<T1,T2>::hash_next() {
    size_t reg = 1;
    hash_iterator_inst(reg);
    T1 key = reg;
    if (! mem_read){
        return key;
    }

    if (reg == END && ! mem_read){
        mem_read = true;
        it = mem_hash.begin();
        return it->first;
    }

    if (mem_read){
        it++;
    }
}
#endif //HLB_HLB_API_H
