#include "../LRUcache.h"
#include <iostream>
using namespace std;
//
// Created by Anlan Yu on 11/30/19.
//
int main(){
    LRUCache cache(128);
    int a = 89;
    int b = 98;
    int kk[] = {1,2,23,34,56,77};
    int c = 77;
    cache.refer((void*)&a);
    cache.refer((void*)&a);
    cache.refer((void*)&b);
    cache.refer((void*)&c);
    cout<<cache.inst_in_cache<<"\n"<<cache.inst_in_mem;

}
