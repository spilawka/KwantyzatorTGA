#pragma once
#include <math.h>

typedef map<uint8_t,int> u8map;
typedef map<uint32_t,int> u32map;

/** Policz entropię dla podanej mapy ilości wystąpień*/
template <class T> double calcEntrophy(int size, T fmap) {
    double res = 0.0;
    typename T::iterator i;
    for(i = fmap.begin(); i!= fmap.end(); i++) {
        int val = i->second;
        if (val==0) exit(1);

        double p = val*1.0/size;
        double inf = -1.0*log2(p);

        res+=p*inf;
    }
    return res;
}