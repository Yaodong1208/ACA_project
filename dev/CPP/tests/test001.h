//
// Created by Anlan Yu on 11/23/19.
//
//this test aim to ensure the key value pair insertion in HLB is the same as we read out from it
//we first insert KEY_NUM of kv pairs and then read them out, finally compare to see if they are the same
#define KEY_NUM 100000
#include "../hlb_api.h"
#include <set>
using namespace std;
void test1(){
    set<vector<size_t >> current_kv;//this contains kv we have inserted by now
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
        printf("size is %d \n",current_kv.size());
    }

    //read out from myhlb
    int count = 0;
    size_t it = myhlb.hash_begin();
    size_t end_hlb = myhlb.hash_end();
    for(; it != end_hlb; it = myhlb.hash_next()) {
        size_t v = myhlb.hash_lookup(it);
        count ++;
        vector<size_t > tmp = {it,v};
        read_out_kv.insert(tmp);
        printf("lookup number: %d \n", count);
    }
    size_t v = myhlb.hash_lookup(it);
    count ++;
    vector<size_t > tmp = {it,v};
    read_out_kv.insert(tmp);
    printf("lookup number: %d \n", count);


    //check if read_out_kv equals current_kv, if not, fail
    for(auto item : current_kv) {
        if(read_out_kv.find(item) == read_out_kv.end()) {
            perror("test fail due to read_out not equals insert");
            break;
        }
    }
}

