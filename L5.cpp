#include <fstream>
#include <string>
#include <math.h>
#include <string.h>

#include "TGALoader.h"
#include "Util.h"

typedef map<char,int> cmap;
/** Załaduj zawartość pliku do kolejki bajtowej */
charq* loadBytes(char* filename, long* length) {
    ifstream f;
    f.open(filename, ios_base::binary);
    if (!f.is_open()) {
        cout<<"Błąd przy otwieraniu pliku!"<<endl;
        exit(-1);
    }

    f.seekg(0,ios::end);
    // długość pliku w bajtach
    size_t size = f.tellg();
    *length = (long)size;
    vector<char> bytes(size);

    f.seekg(0,ios::beg);
    f.read(&bytes[0],size);

    cmap charFreq;
    charq* queue = new charq();
    for (const char el: bytes) {
        queue->push(el);
        
        cmap::iterator it = charFreq.find(el);
        if (it==charFreq.end()) charFreq[el] = 1;
        else it->second++;
    }
    bytes.clear();

    f.close();
    return queue;
}

int main(int argn, char** args) {
    if (argn!=5) {
        cout<<"Błędna ilość argumentów"<<endl;
        cout<<" ./L5 input output type bits"<<endl;
        cout<<" type = {SNR,MSE}"<<endl;
        return 1;
    }

    enum Mode mode;
    if (strcmp(args[3],"SNR")==0)
        mode = maxSNR;
    else if (strcmp(args[3],"MSE") == 0)
        mode = minMSE;
    else {
        cout<<"Nierozpoznawalny typ!"<<endl;
        cout<<" type = {SNR,MSE}"<<endl;
        return 1;
    }

    int bits = atoi(args[4]);
    if (bits < 0 || bits > 24) {
        cout<<"Błędna ilość bitów"<<endl;
        cout<<"zakres = [0,24]"<<endl;
        return 1;
    }


    long l;
    charq* Bytes = loadBytes(args[1],&l);
    TGALoader TL(Bytes);
    TL.createPixelMap();
    TL.FindOptimalDistribution(bits,mode,args[2]);
    //delete(c);
    //delete(ents);
    return 0;
}