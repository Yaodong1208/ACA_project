//
// Created by Anlan Yu on 11/23/19.
//

#ifndef HLB_INST_SIMULATE_H
#define HLB_INST_SIMULATE_H
#define ROW_NUM  1024
#define COL_NUM 64
#define SHRINK_RATE 10
#define LOAD_FACTOR 60
#define REHASH_MAX 3

#include <iostream>
#include <unordered_map>
#include <functional>
#include <vector>

using namespace std;

enum Result {NOT_FOUND = 0XFFFFFFFF, // lookup fail
    NOVAL, // no value in this entry means
    FAIL, // erase fail
    END, // HLB iterator end
    ACCESS_NOT_ALLOWED, // if row_end >= row_num, then access not allowed
};


size_t lc = 0;  //to mark the victim's column number
size_t COL_END;
size_t ROW_END;
size_t ROW_iter;
size_t COL_iter;

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
    size_t success = 0;
    size_t i = hash_func_in_hlb(reg_key);

    for(size_t j = 0;j<COL_NUM;j++){
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
    //update lc

    size_t row = NOT_FOUND;
    size_t col = NOT_FOUND;
    size_t update = 0;

    size_t i = hash_func_in_hlb(reg_key);

    for(size_t j = 0;j<COL_NUM;j++){
        if(hlb[i][j][0] == reg_key){
            //if we find key in hlb equals to reg_key, we update it, and set update to 1
            hlb[i][j][1] = reg_value;
            update = 1;
            break;
        }

        if(hlb[i][j][0] == NOVAL){
            //we record first empty slot for this insertion kv, and record it
            if(row == NOT_FOUND && col == NOT_FOUND){
                row = i;
                col = j;
            }
        }
    }
    if (!update){
        //means reg_key not in hlb by now
        if(row != NOT_FOUND){
            //means there is an empty slot we can use to insert new key_value pair
            hlb[row][col][0] = reg_key;   //insert new key
            hlb[row][col][1] = reg_value;  //insert new value
        }else{
            //there is no empty slot we can find in hlb for this key value pair
            //set reg_key to victim_key
            ///TODO:if hlb is not able to extend, we first lookup to get the value of this victim then erase this victim,
            /// use mem_insert to insert it into memory
            /// finally, use hash_insert_inst to insert reg_key,reg_value pair
            reg_key = hlb[i][lc][0];
        }
    }
    //
    if(lc == COL_NUM-1){
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

    size_t success = 0;
    size_t i = hash_func_in_hlb(reg_key);

    for(size_t j = 0;j<COL_NUM;j++){
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
        size_t count = 0;
        for(size_t k=0;k<ROW_END;k++){
            for(size_t j=0;j<COL_NUM;j++){
                if(hlb[k][j][0] != NOVAL){
                    count++;
                }
            }
        }
        reg_result = (count*100)/((ROW_END+1)*COL_NUM);
    }
}

//if input reg_val == -1 means we need the last not null's key
//if input reg_val == 0 means we need the first not null's key
//if input reg_val == 1 means we need the next not null's key
//if input reg_val == 2 means we need to know if the hlb is full, if the return value of reg_val = false means extendable otherwise the return value indicate occupied rate
//if input reg_val == 3 means we need to double the hlb size
//if input reg_val == 4 means we need to half the hlb size
//if input reg_val == 5 means we need to know if the hlb is shrinkable, if the return value of reg_val = true means shrinkable otherwise not shrinkable
//if input reg_val == 6 means we want to initialize hlb and set ROW_END to 1
//if input reg_val == 7 means we want to clear the whole table to NOVAL
//if input reg_val !={-1,0,1,2,3,4,5,6} means we need a parameter
void hash_iterator_inst(size_t &reg) {
    //if input reg_val == -1 means we need the last not null's key
    if(reg == -1){
        COL_END = COL_NUM - 1;

        while (hlb[ROW_END][COL_END][0] == NOVAL){
            if(!COL_END){
                COL_END = COL_NUM - 1;
                ROW_END = ROW_END - 1;
                if(ROW_END<0){
                    break;
                }
            }else{
                COL_END --;
            }
        }
        if(ROW_END<0){
            //means nothing in the table, set the first element as the end
            reg = NOT_FOUND;
            ROW_END = 0;
            COL_END = 0;
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
                if(ROW_iter >= ROW_NUM){
                    break;
                }
            }else{
                COL_iter ++;
            }
        }
        if(ROW_iter >= ROW_NUM){
            //means nothing in the table, set the first element as begin
            reg = NOT_FOUND;
            ROW_iter = 0;
            COL_iter = 0;
        }else{
            reg = hlb[ROW_iter][COL_iter][0];
        }

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
            ROW_iter = ROW_NUM;
        }
        auto a = ROW_iter;
        auto b = COL_iter;
        //check whether the next position is NOVAL, if it is then continue to next position until find a position with value
        while(hlb[ROW_iter][COL_iter][0] == NOVAL){
            if(COL_iter == COL_NUM - 1){
                ROW_iter ++;
                COL_iter = 0;
                if(ROW_iter >= ROW_NUM){
                    break;
                }
            }else{
                COL_iter ++;
            }
        }
        if(ROW_iter >= ROW_NUM){
            reg = ACCESS_NOT_ALLOWED;
            ROW_iter = ROW_NUM - 1;
        }else{
            reg = hlb[ROW_iter][COL_iter][0];
        }
    }
        //if input reg_val == 2 means we need to know if the hlb is full, if the return value of reg_val = false means extendable
        // otherwise the return value indicate occupied rate
    else if(reg == 2){
        if(ROW_END == ROW_NUM - 1){
            size_t count = 0;
            for(size_t i = 0;i<ROW_NUM;i++){
                for(size_t j=0;j<COL_NUM;j++){
                    if(hlb[i][j][0] != NOVAL){
                        count ++;
                    }
                }
            }
//            printf("count: %d \n", count);
            reg = (count*100)/(ROW_NUM*COL_NUM);
        }else{
            reg = false;

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
        //if input reg_val == 6 means we want to initialize hlb and set ROW_END to 0
    else if(reg == 6){
        ROW_END = 0;
    }
        //if input reg_val == 7 means we want to clear the whole table to NOVAL
    else if(reg == 7){
        for(size_t i=0;i<ROW_NUM;i++){
            for(size_t j=0;j<COL_NUM;j++){
                hlb[i][j][0] = NOVAL;
                hlb[i][j][1] = NOVAL;
            }
        }

    }


    else{
        s = rand();
    }

}

//for debug
int count(){
    size_t count = 0;
    for(size_t i = 0;i<ROW_NUM;i++){
        for(size_t j=0;j<COL_NUM;j++){
            if(hlb[i][j][0] != NOVAL){
                count ++;
            }
        }
    }
    return count;
}

int row_end(){
    return ROW_END;
}
#endif //HLB_INST_SIMULATE_H
