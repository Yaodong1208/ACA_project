//
// Created by Anlan Yu on 11/23/19.
//
//this test aims to ensure after erase some key_value pairs from hlb which may happen in HLB or mem_hash,
//the key_value pairs contained in hlb is equal to kv_pairs_before - kv_pair_erased
#define KEY_NUM 1000000
#define ERASE_NUM 500000
#include "../hlb_api.h"
#include <set>
using namespace std;
void test2(){
    set<vector<size_t >> current_kv;//this contains kv we have inserted by now
    set<vector<size_t >> erase_kv;//this contains kv we have erased by now
    set<vector<size_t >> diff_kv;//this contains the difference between current_kv and erase_kv
    set<vector<size_t >> read_out_kv; // this contains kv we read_out from myhlb by now
    vector<size_t> kv_read;
    HLB<size_t ,size_t > myhlb;

    //clear myhlb
    myhlb.clear_hlb();

    //insert number of KEY_NUM kv pair to myhlb and record such info to current_kv
    for(int i = 0; i < KEY_NUM; i++) {
        vector<size_t > tmp; //key_value pair which we will insert
        int a = current_kv.size();
        tmp.push_back(rand()); //means key to insert
        tmp.push_back(rand()); //means value to insert
        myhlb.hash_insert(tmp[0],tmp[1]);
        current_kv.insert(tmp);
        printf("current_kv size is: %d \n",current_kv.size());
    }

    //erase ERASE_NUM kv pairs in current_kv and record such info to erase_kv
    for(int i = 0;i<ERASE_NUM;i++){
        int index = rand() % ERASE_NUM;
        vector<size_t> tmp;
        auto it = current_kv.begin();
        advance(it,index);
        tmp = *it;
        myhlb.hash_erase(tmp[0]);
        erase_kv.insert(tmp);
        printf("erase_kv size is: %d \n", erase_kv.size());
    }

    //read out from myhlb
    size_t it = myhlb.hash_begin();
    size_t end_hlb = myhlb.hash_end();
    for(; it != end_hlb; it = myhlb.hash_next()) {
        size_t v = myhlb.hash_lookup(it);
        vector<size_t > tmp = {it,v};
        read_out_kv.insert(tmp);
        printf("read_out_kv size is: %d \n", read_out_kv.size());
    }
    size_t v = myhlb.hash_lookup(it);
    vector<size_t > tmp = {it,v};
    read_out_kv.insert(tmp);
    printf("read_out_kv size is: %d \n", read_out_kv.size());

    //set diff_kv
    set_difference(current_kv.begin(), current_kv.end(), erase_kv.begin(), erase_kv.end(), inserter(diff_kv, diff_kv.begin()));

    //check if read_out_kv equals current_kv - erase_kv, if not, fail
    for(auto item : diff_kv) {
        if(read_out_kv.find(item) == read_out_kv.end()) {
            perror("test fail due to read_out not equals insert");
            break;
        }
    }
}


