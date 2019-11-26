//
// Created by Anlan Yu on 11/23/19.
//

#ifndef HLB_INST_SIMULATE_H
#define HLB_INST_SIMULATE_H
#include <cstddef>
#include <cinttypes>
#include <iostream>
using namespace std;

void hash_lookup_inst(size_t& reg_key, size_t& reg_result) {
    double a = 2.0;
    double b = 1.0;
    double c;
    __asm__(
    "fld %1\n"   //load a to st(0)
    "fdivr %2\n" //st(0) = st(0)/b
    "fstl %0\n"  //store st(0) to c


    :"+m" (c) /* %0: Out */  //c is in memory and can be write and read
    :"m" (a), "m"(b) /* %1, %2: In */ // a,b are in memory
    : /* Overwrite */
    );
    cout<<c;  //expect c to be 2.0
}


void hash_insert_inst(size_t &reg_key, size_t &reg_value) {

}


void hash_erase_inst(size_t &reg_key, size_t &reg_result) {

}

void hash_iterator_inst(size_t &reg) {

}
#endif //HLB_INST_SIMULATE_H
