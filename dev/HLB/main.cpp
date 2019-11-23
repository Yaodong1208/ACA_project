#include <iostream>
#include "HLB.h"
#include <vector>
using namespace std;
int main() {
//    std::cout << "Hello, World!" << std::endl;
    HLB<uint32_t, uint32_t> hlb;
    hlb.clear();
    vector<uint64_t> insert_kv;
    for(int i = 0;i<10000;i++){
        int rate = rand(); //use this rate to determine whether insert erase or lookup, rules are shown below
        rate = rate % 10;
        if(rate == 0 || rate == 1){
            uint64_t key = (uint64_t)rand();
            uint64_t value = (uint64_t)rand();
            hlb.hash_insert(key,value);
            printf("insert Key: %d, Value: %d \n", key, value);
            insert_kv.push_back(key);

        }else if(rate == 2 || rate == 3){
            int current_kv_size = insert_kv.size();
            if(current_kv_size){
                int id = rand() % current_kv_size;
                uint64_t k = insert_kv[id];
                hlb.hash_erase(k);
                printf("erase key: %d \n",k);
                insert_kv.erase(insert_kv.begin() + id - 1);
            }else{
                continue;
            }

        }
        else{
            int d = insert_kv.size();
            if(d){

                int id = rand() % d;
                uint64_t k = insert_kv[id];
                uint32_t value = hlb.hash_lookup(k);
                printf("lookup key: %d, Value:  %d \n", k, value);
            }
            else{
                continue;
            }

        }
    }
    return 0;
}