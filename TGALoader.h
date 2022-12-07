#pragma once
using namespace std;

#include <queue>
#include <stdint.h>
#include <map>
#include <iostream>
#include <algorithm>
typedef queue<char> charq;

enum Mode{maxSNR,minMSE};

class TGALoader {
private:
    charq* bQ;
    //
    uint8_t idLen;
    bool CMT;
    uint8_t imageType;
    uint8_t pixelDepth;
    uint16_t xOrigin,yOrigin;
    int width;
    int height;
    int size;
    uint8_t* header;
    int hc = 0;
    uint8_t* footer;
    int fc = 0;

    void loadHeader();
    template <class T> T nBytes(int n);

    uint8_t** pixelsR;
    uint8_t** pixelsG;
    uint8_t** pixelsB;
public: 
    TGALoader(charq* Q);
    ~TGALoader();
    void saveFile(char* filename, uint8_t** RP, uint8_t** BP, uint8_t** GP);
    uint8_t** Quantizize(int bits, uint8_t** pixels, double* MSESum, double* SNRSum, bool pexport);
    void FindOptimalDistribution(int bits, enum Mode mode, char* outputFile);
    void createPixelMap();
};