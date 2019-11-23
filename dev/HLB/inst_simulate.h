//
// Created by Anlan Yu on 11/23/19.
//

#ifndef HLB_INST_SIMULATE_H
#define HLB_INST_SIMULATE_H
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

//for test
int rc = 0;
int lc = 0;
int COL_END;
int ROW_END;
int ROW_iter;
int COL_iter;

size_t hlb[ROW_NUM][COL_NUM][2];

size_t shift = 32;
unsigned long long s = 2654435769ull;

size_t hash_func_in_hlb(size_t const key){
//        static const unsigned long long s = 2654435769ull;
    const unsigned long long r = ((unsigned long long)key) * s;
    return (size_t)((r & 0xFFFFFFFF) >> shift);
}

void hash_lookup_inst(size_t& reg_key, size_t& reg_result) {
    //first we use hash_function in hlb to find corresponding row
    //if find the key then lookup success, return the value
    //if not success, return not found
    int success = 0;
    size_t i = hash_func_in_hlb(reg_key);

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


void hash_insert_inst(size_t &reg_key, size_t &reg_value) {
    //first we use hash_function in hlb to find corresponding row
    //if update = 1 means we find the key, stop searching
    //if find a NOVAL space, record its row and col for future use
    //if update = 0 means we didn't find the key, if find a NOVAL space then insert, else return key at hlb[i+rc][lc]
    //update rc lc

    int row = NOVAL;
    int col = NOVAL;
    int update = 0;
    size_t i = hash_func_in_hlb(reg_key);

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


void hash_erase_inst(size_t &reg_key, size_t &reg_result) {
    //first we use hash_function in hlb to find reg_key's corresponding row
    //if key in that row, we set such key and value in hlb to NOVAL and set reg_result to occupied rate
    //otherwise we set reg_result to FAIL

    int success = 0;
    size_t i = hash_func_in_hlb(reg_key);

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

//if input reg_val == -1 means we need the last not null's key
//if input reg_val == 0 means we need the first not null's key
//if input reg_val == 1 means we need the next not null's key
//if input reg_val == 2 means we need to know if the hlb is extendable, if the return value of reg_val = true means extendable otherwise the return value indicate occupied rate
//if input reg_val == 3 means we need to double the hlb size
//if input reg_val == 4 means we need to half the hlb size
//if input reg_val == 5 means we need to know if the hlb is shrinkable, if the return value of reg_val = true means shrinkable otherwise not shrinkable
//if input reg_val == 6 means we want to initialize hlb and set ROW_END to 1
//if input reg_val !={-1,0,1,2,3,4,5,6} means we need a parameter
void hash_iterator_inst(size_t &reg) {
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
#endif //HLB_INST_SIMULATE_H
