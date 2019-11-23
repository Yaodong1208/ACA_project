//
// Created by Anlan Yu on 11/16/19.
//
#ifndef HLB_HLB_H
#define HLB_HLB_H

#define ROW_NUM  1024
#define COL_NUM 64
#define SHRINK_RATE 10
#define LOAD_FACTOR 80
#define REHASH_MAX 3

#include <iostream>
#include <unordered_map>
#include <functional>
#include <vector>

using namespace std;
enum Result {NOT_FOUND = 0XFFFFFFFF, // lookup fail
        NOVAL, // no value in this entry
        FAIL, // erase fail
        END, // HLB iterator end
        ACCESS_NOT_ALLOWED, // if row_end >= row_num, then access not allowed
};

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

    //for test
    int rc = 0;
    int lc = 0;

    size_t hlb[ROW_NUM][COL_NUM][2];
    void hash_insert(T1 key, T2 value, int rehash_count = 0);
    T2 hash_lookup(T1 key);
    void hash_erase(T1 key);
    T1 hash_begin();
    T1 hash_end();
    T1 hash_next();
    void clear();

private:

    size_t shift = 32;
    unsigned long long s = 2654435769ull;
    vector<vector<size_t>> victim_vector;
    unordered_map<T1,value_mem_hash<T2>> mem_hash;
    bool mem_read;
    typename unordered_map<T1,value_mem_hash<T2>>::iterator it;
    size_t hash_func(size_t const key){
//        static const unsigned long long s = 2654435769ull;
        const unsigned long long r = ((unsigned long long)key) * s;
        return (size_t)((r & 0xFFFFFFFF) >> shift);
    }

    // instructions
    void hash_lookup_inst(size_t& reg_key, size_t& reg_result);
    void hash_insert_inst(size_t& reg_key, size_t& reg_value);
    void hash_erase_inst(size_t& reg_key, size_t& reg_result);
    void hash_iterator_inst(size_t& reg);

    // functions used to do rehash
    void rehash(size_t indicator);

    int threshold = 5;
    int COL_END;
    int ROW_END;
    int ROW_iter;
    int COL_iter;

};
template<class T1,class T2>
void HLB<T1,T2>::rehash(size_t indicator) {
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
void HLB<T1,T2>::clear() {
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
void HLB<T1,T2>::hash_lookup_inst(size_t& reg_key, size_t& reg_result) {
    //first we use hash_function in hlb to find corresponding row
    //if find the key then lookup success, return the value
    //if not success, return not found
    int success = 0;
    size_t i = hash_func(reg_key);

    for(int j = 0;j<COL_NUM;j++){
        if(hlb[i][j][0] == reg_key){
            reg_result = hlb[i][j][1];
            success = 1;
            break;
        }
    }

    if(!success){
        reg_result = NOT_FOUND;
    }

}

template<class T1,class T2>
void HLB<T1,T2>::hash_insert_inst(size_t &reg_key, size_t &reg_value) {
    //first we use hash_function in hlb to find corresponding row
    //if update = 1 means we find the key, stop searching
    //if find a NOVAL space, record its row and col for future use
    //if update = 0 means we didn't find the key, if find a NOVAL space then insert, else return key at hlb[i+rc][lc]
    //update rc lc

    int row = NOVAL;
    int col = NOVAL;
    int update = 0;
    size_t i = hash_func(reg_key);

    for(int j = 0;j<COL_NUM;j++){
        if(hlb[i][j][0] == reg_key){
            hlb[i][j][1] = reg_value;
            update = 1;
            break;
        }

        if(hlb[i][j][0] == NOVAL){
            row = i;
            col = j;
        }
    }
    if (!update){
        if(row != NOVAL){
            hlb[row][col][0] = reg_key;
            hlb[row][col][1] = reg_value;
        }else{
            reg_key = hlb[i+rc][lc][0];
        }
    }

    if(lc == COL_NUM-1 && rc != ROW_NUM-1){
        lc = 0;
        rc ++;
    }
    else if(lc == COL_NUM-1 && rc == ROW_NUM-1){
        rc = 0;
        lc = 0;
    }
    else{
        lc ++;
    }
}

template<class T1,class T2>
void HLB<T1,T2>::hash_erase_inst(size_t &reg_key, size_t &reg_result) {
    //first we use hash_function in hlb to find reg_key's corresponding row
    //if key in that row, we set such key and value in hlb to NOVAL and set reg_result to occupied rate
    //otherwise we set reg_result to FAIL

    int success = 0;
    size_t i = hash_func(reg_key);

    for(int j = 0;j<COL_NUM;j++){
        if(hlb[i][j][0] == reg_key){
            hlb[i][j][0] = NOVAL;
            hlb[i][j][1] = NOVAL;
            success = 1;
            break;
        }
    }

    if(!success){
        reg_result = FAIL;
    }else{
        int count = 0;
        for(int k=0;k<ROW_END;k++){
            for(int j=0;j<COL_NUM;j++){
                if(hlb[k][j][0] != NOVAL){
                    count++;
                }
            }
        }
        reg_result = (count<<2)/((ROW_END+1)*COL_NUM);
    }
}

template<class T1,class T2>
//if input reg_val == -1 means we need the last not null's key
//if input reg_val == 0 means we need the first not null's key
//if input reg_val == 1 means we need the next not null's key
//if input reg_val == 2 means we need to know if the hlb is extendable, if the return value of reg_val = true means extendable otherwise the return value indicate occupied rate
//if input reg_val == 3 means we need to double the hlb size
//if input reg_val == 4 means we need to half the hlb size
//if input reg_val == 5 means we need to know if the hlb is shrinkable, if the return value of reg_val = true means shrinkable otherwise not shrinkable
//if input reg_val == 6 means we want to initialize hlb and set ROW_END to 1
//if input reg_val !={-1,0,1,2,3,4,5,6} means we need a parameter
void HLB<T1,T2>::hash_iterator_inst(size_t &reg) {
    //if input reg_val == -1 means we need the last not null's key
    if(reg == -1){
        COL_END = COL_NUM - 1;

        while (hlb[ROW_END][COL_END][0] == NOVAL){
            if(!COL_END){
                COL_END = COL_NUM - 1;
                ROW_END = ROW_END - 1;
            }else{
                COL_END --;
            }
        }
        if(ROW_END<0){
            reg = NOT_FOUND;
            ROW_END = 0;
        }else{
            reg = hlb[ROW_END][COL_END][0];
        }
    }

    //if input reg_val == 0 means we need the first not null's key
    else if(reg == 0){
        ROW_iter = 0;
        COL_iter = 0;
        while(hlb[ROW_iter][COL_iter][0] == NOVAL){
            if(COL_iter == COL_NUM - 1){
                ROW_iter ++;
                COL_iter = 0;
            }else{
                COL_iter ++;
            }
        }
        reg = hlb[ROW_iter][COL_iter][0];
    }

    //if input reg_val == 1 means we need the next not null's key
    else if(reg == 1){

        //first point to the next position
        if(COL_iter == COL_NUM - 1){
            ROW_iter ++;
            COL_iter = 0;
        }else{
            COL_iter ++;
        }

        //then check whether the next position allowed to be accessed or not
        if(ROW_iter >= ROW_NUM){
            reg = ACCESS_NOT_ALLOWED;
        }

        //check whether the next position is NOVAL, if it is then continue to next position until find a position with value
        while(hlb[ROW_iter][COL_iter][0] == NOVAL){
            if(COL_iter == COL_NUM - 1){
                ROW_iter ++;
                COL_iter = 0;
            }else{
                COL_iter ++;
            }
        }
        if(ROW_iter >= ROW_NUM){
            reg = ACCESS_NOT_ALLOWED;
        }else{
            reg = hlb[ROW_iter][COL_iter][0];
        }
    }
    //if input reg_val == 2 means we need to know if the hlb is extendable, if the return value of reg_val = true means extendable
    // otherwise the return value indicate occupied rate
    else if(reg == 2){
        if(ROW_END == ROW_NUM - 1){
            int count = 0;
            for(auto & i : hlb){
                for(int j=0;j<COL_NUM;j++){
                    if(i[j][0] != NOVAL){
                        count ++;
                    }
                }
            }
            reg = (count<<2)/(ROW_NUM*COL_NUM);
        }else{
            reg = true;

        }
    }

    //if input reg_val == 3 means we need to double the hlb size
    else if(reg == 3){
        ROW_END = 2 * (ROW_END+1) - 1;
    }

    //if input reg_val == 4 means we need to half the hlb size
    else if(reg == 4){
        ROW_END = (ROW_END+1)/2 - 1;
    }

    //if input reg_val == 5 means we need to know if the hlb is shrinkable,
    //if the return value of reg_val = true means shrinkable otherwise not shrinkable
    else if(reg == 5){
        reg = ROW_END != 0;
    }
    //if input reg_val == 6 means we want to initialize hlb and set ROW_END to 1
    else if(reg == 6){
        ROW_END = 0;
    }

    else{
        s = rand();
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
            rehash(3);
            hash_insert(k,v,rehash_count);
        }else{
            if(rehash_count < REHASH_MAX && reg_v < LOAD_FACTOR) {
                //in this case hash_table is not extendable && the occupied rate is not high, we need to change the hash function
                //in such case, we still can rehash because rehash_count by now is still smaller then REHASH_MAX
                //only change hash_function parameter s, not changing size
                rehash(0);
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
            rehash(4);
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


#endif //HLB_HLB_H
