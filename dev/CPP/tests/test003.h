//
// Created by Anlan Yu on 11/30/19.
//

#ifndef HLB_TEST003_H
#define HLB_TEST003_H
#define KEY_NUM 100000
//this test aims to test whether the behaviour is correct
// if we insert and erase kv_pair interleavely
#include "../hlb_api.h"
#include <set>
void test3(){
    set<vector<size_t >> insert_kv;//this contains kv we have inserted by now
    set<vector<size_t >> erase_kv;//this contains kv we have erased by now
    set<vector<size_t >> diff_kv;//this contains the difference between current_kv and erase_kv
    set<vector<size_t >> read_out_kv; // this contains kv we read_out from myhlb by now
    HLB<uint32_t, uint32_t> hlb;
    hlb.clear_hlb();

    for(int i = 0;i<KEY_NUM;i++){
        int rate = rand(); //use this rate to determine whether insert erase or lookup, rules are shown below
        rate = rate % 10;
        if(rate == 0 || rate == 1){
            size_t key = (size_t)rand();
            size_t value = (size_t)rand();
            hlb.hash_insert(key,value);
            vector<size_t > tmp = {key,value};
            insert_kv.insert(tmp);
            printf("insert size: %d \n", insert_kv.size());

        }else if(rate == 2 || rate == 3){
            if(insert_kv.size()){
                int id = rand() % insert_kv.size();
                auto it = insert_kv.begin();
                advance(it,id);
                hlb.hash_erase((*it)[0]);
                erase_kv.insert(*it);
                printf("erase size: %d \n",erase_kv.size());

            }else{
                continue;
            }

        }
        else{
            int d = insert_kv.size();
            if(d){
                int id = rand() % d;
                auto it = insert_kv.begin();
                advance(it,id);
                size_t value = hlb.hash_lookup((*it)[0]);
//                printf("i: %d, lookup key: %d, Value:  %d \n",i, k, value);
            }
            else{
                continue;
            }

        }
    }
    //read out from myhlb
    size_t it = hlb.hash_begin();
    size_t end_hlb = hlb.hash_end();
    for(; it != end_hlb; it = hlb.hash_next()) {
        size_t v = hlb.hash_lookup(it);
        vector<size_t > tmp = {it,v};
        read_out_kv.insert(tmp);
        printf("read_out_kv size is: %d \n", read_out_kv.size());
    }
    size_t v = hlb.hash_lookup(it);
    vector<size_t > tmp = {it,v};
    read_out_kv.insert(tmp);
    printf("read_out_kv size is: %d \n", read_out_kv.size());

    set_difference(insert_kv.begin(), insert_kv.end(), erase_kv.begin(), erase_kv.end(), inserter(diff_kv, diff_kv.begin()));

    //check if read_out_kv equals current_kv - erase_kv, if not, fail
    for(auto item : diff_kv) {
        if(read_out_kv.find(item) == read_out_kv.end()) {
            perror("test fail due to read_out not equals insert");
            break;
        }
    }
}



#endif //HLB_TEST003_H
