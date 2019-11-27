/*#include <iostream>
#include "hlb_api.h"
#include <vector>
using namespace std;
int main() {
//    std::cout << "Hello, World!" << std::endl;
    HLB<uint32_t, uint32_t> hlb;
    hlb.clear_hlb();
    vector<uint64_t> insert_kv;
    for(int i = 0;i<50000;i++){
        int key = rand();
        int value = rand();
        hlb.hash_insert(key, value);
        insert_kv.push_back(key);
        printf("insert key:%d, value:%d, current insert_kv size: %d \n", key, value, insert_kv.size());
    }
    int current_kv_size = insert_kv.size();
    if(current_kv_size){
        int id = rand() % current_kv_size;
        uint64_t k = insert_kv[id];
        hlb.hash_erase(k);
        insert_kv.erase(insert_kv.begin() + id - 1);
        printf("erase key: %d, current insert_kv size: %d \n", k, insert_kv.size());

    }
//    if(current_kv_size){
//        int id = rand() % current_kv_size;
//        uint64_t k = insert_kv[id];
//        uint64_t value = hlb.hash_lookup(k);
//        printf("lookup key: %d, value: %d, current insert_kv size:%d \n", k, value, insert_kv.size());
//    }



//    for(int i = 0;i<100000;i++){
//        int rate = rand(); //use this rate to determine whether insert erase or lookup, rules are shown below
//        rate = rate % 10;
//        if(rate == 0 || rate == 1){
//            uint64_t key = (uint64_t)rand();
//            uint64_t value = (uint64_t)rand();
//            hlb.hash_insert(key,value);
//            insert_kv.push_back(key);
//            printf("i: %d, insert Key: %d, Value: %d \n", i, key, value);
////            for(int j = 0;j<insert_kv.size();j++){
////                printf("key: %d ", insert_kv[j]);
////            }
////            printf("\n");
//
//        }else if(rate == 2 || rate == 3){
//            int current_kv_size = insert_kv.size();
//            if(current_kv_size){
//                int id = rand() % current_kv_size;
//                uint64_t k = insert_kv[id];
//                hlb.hash_erase(k);
//                insert_kv.erase(insert_kv.begin() + id);
//                printf("i: %d, erase key: %d \n",i, k);
////                for(int j = 0;j<insert_kv.size();j++){
////                    printf("key: %d ", insert_kv[j]);
////                }
////                printf("\n");
//            }else{
//                continue;
//            }
//
//        }
//        else{
//            int d = insert_kv.size();
//            if(d){
//
//                int id = rand() % d;
//                uint64_t k = insert_kv[id];
//                uint32_t value = hlb.hash_lookup(k);
//                printf("i: %d, lookup key: %d, Value:  %d \n",i, k, value);
////                for(int j = 0;j<insert_kv.size();j++){
////                    printf("key: %d ", insert_kv[j]);
////                }
////                printf("\n");
//            }
//            else{
//                continue;
//            }
//
//        }
//    }
    return 0;
}*/

#include "tests/test001.h"
int main(){
    test();
}