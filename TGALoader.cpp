#include "TGALoader.h"
#include "Util.h"
#include "math.h"
#include <fstream>

TGALoader::TGALoader(charq* Q) {
    bQ = Q;
    header = new uint8_t[18];
    loadHeader();
}

TGALoader::~TGALoader() {
    delete(header);
    for(int i=0;i<height;i++) {
        free(pixelsR[i]);
        free(pixelsG[i]);
        free(pixelsB[i]);
    }
    
}

template <class T> T TGALoader::nBytes(int n) {
    T num = (T)0;
    for (int i=0; i<n; i++) {
        header[hc++] = bQ->front();
        num += (uint8_t)bQ->front()<<(i*8);
        bQ->pop();
    }
    return num;
}

void TGALoader::loadHeader() {
    
    // ID Length
    header[hc++] = bQ->front();
    idLen = (uint8_t)bQ->front();  bQ->pop();
    // Color Map Type
    header[hc++] = bQ->front();
    CMT = (bool)bQ->front();  bQ->pop();
    // Image Type
    header[hc++] = bQ->front();
    imageType = (uint8_t)bQ->front();  bQ->pop();
    // Color Map Spec
    for(int i=0;i<5;i++) {
        header[hc++] = bQ->front();
        bQ->pop();
    }

    //Image spec (10B)
    xOrigin = nBytes<uint16_t>(2);
    yOrigin = nBytes<uint16_t>(2);
    width = nBytes<uint16_t>(2);
    height = nBytes<uint16_t>(2);
    size = width*height;
    header[hc++] = bQ->front();
    pixelDepth = (uint8_t)bQ->front();  bQ->pop();
    header[hc++] = bQ->front();
    bQ->pop();

}

void TGALoader::createPixelMap() {
    pixelsR = new uint8_t *[height];
    pixelsG = new uint8_t *[height];
    pixelsB = new uint8_t *[height];
    for (int i=0; i<height; i++) {
        pixelsR[i] = new uint8_t[width];
        pixelsG[i] = new uint8_t[width];
        pixelsB[i] = new uint8_t[width];
    } 

    uint8_t r,g,b;
    for (int i=0; i<height; i++) for (int j=0; j<width; j++) {
        b = bQ->front(); bQ->pop();
        pixelsB[i][j] = b;
        g = bQ->front(); bQ->pop();
        pixelsG[i][j] = g;
        r = bQ->front(); bQ->pop();
        pixelsR[i][j] = r;
    }

    footer = new uint8_t[bQ->size()];
    for(fc=0; !bQ->empty(); fc++) {
        footer[fc] = bQ->front();
        bQ->pop();
    }
}

void TGALoader::saveFile(char* filename, uint8_t** RP, uint8_t** GP, uint8_t** BP) {
    ofstream f(filename);

    for(int i=0; i<hc; i++) {
        f<<header[i];
    }

    for (int i=0; i<height; i++) for (int j=0; j<width; j++) {
        f<<BP[i][j]<<GP[i][j]<<RP[i][j];
    }

    for(int i=0; i<fc; i++) {
        f<<footer[i];
    }

    f.close();
}

uint8_t** TGALoader::Quantizize(int bits, uint8_t** pixels, double* MSESum, double* SNRSum, bool pexport) {
    int steps = (int)pow(2,bits);
    
    uint8_t val,qval;
    double stepSize = 255.0/(steps-1),
    halfStep = stepSize/2.0,
    valStepSize,quan,diffSum=0.0,signalSum=0.0;

    uint8_t** exportPixels = nullptr;
    if (pexport) {
        exportPixels = new uint8_t*[height];
        for(int i=0;i<height;i++) exportPixels[i] = new uint8_t[width];
    }

    if (steps==1) valStepSize = 0.0;
    else valStepSize = 255.0/(steps-1);

    for (int i=0;i<height;i++) {
        for (int j=0;j<width; j++) {
            val = pixels[i][j];
            quan = (val*1.0-halfStep)/stepSize;
            //wartość mieszcząca się w {steps} bitach
            qval = (uint8_t)ceil(quan);
            //wartość znormalizowana do standardu TGA
            qval = (uint8_t)round(qval*1.0*valStepSize);
            
            if(pexport) {
                exportPixels[i][j] = qval;
            }

            diffSum+=pow((val-qval)*1.0,2.0);
            signalSum+=pow(qval*1.0,2.0);
        }
    }

    *MSESum = diffSum;
    *SNRSum = signalSum;
    return exportPixels;
}

void TGALoader::FindOptimalDistribution(int bits, enum Mode mode, char* outputFile) {
    double MSEMin = INFINITY;
    double SNRMax = -1.0;
    double RMSE, GMSE, BMSE, AllMSE;
    double RSNR, GSNR, BSNR, AllSNR;
    double t1,t2;
    int bestR=-1,bestG=-1,bestB=-1;

    for(int iR = 0; iR<=8; iR++) for (int iG = 0; iG<=8; iG++) for(int iB=0; iB<=8; iB++) {
        if (iR + iG + iB == bits) {
            Quantizize(iR,pixelsR,&t1,&t2,false);
            RMSE = t1/size;
            RSNR = t2/t1;
            Quantizize(iG,pixelsG,&t1,&t2,false);
            GMSE = t1/size;
            GSNR = t2/t1;
            Quantizize(iB,pixelsB,&t1,&t2,false);
            BMSE = t1/size;
            BSNR = t2/t1;
            AllMSE = (RMSE + GMSE + BMSE)/3.0;
            if (mode == minMSE) {
                
                t1 = max(max(RMSE,GMSE),BMSE);
              
                if (t1 < MSEMin) {
                    MSEMin = t1;
                    bestR = iR; bestG = iG; bestB = iB;
                }
            }
            else if (mode == maxSNR) {
                t1 = min(min(RSNR,GSNR),BSNR);
                if (t1 > SNRMax) {
                    SNRMax = t1;
                    bestR = iR; bestG = iG; bestB = iB;
                }
            }
        }
    }
    cout<<"Best: ";
    cout<<"R("<<bestR<<") G("<<bestG<<") B("<<bestB<<")"<<endl;

    uint8_t** rPixels = Quantizize(bestR,pixelsR,&t1,&t2,true);
    RMSE = t1;
    RSNR = t2;
    uint8_t** gPixels = Quantizize(bestG,pixelsG,&t1,&t2,true);
    GMSE = t1;
    GSNR = t2;
    uint8_t** bPixels = Quantizize(bestB,pixelsB,&t1,&t2,true);
    BMSE = t1;
    BSNR = t2;

    saveFile(outputFile,rPixels,gPixels,bPixels);

    for(int i=0;i<height;i++) {
        free(rPixels[i]);
        free(gPixels[i]);
        free(bPixels[i]);
    }
    free(rPixels);
    free(gPixels);
    free(bPixels);

    AllMSE = (RMSE + GMSE + BMSE)/(size*3.0);
    cout<<"MSE   ="<<AllMSE<<endl;
    cout<<"MSE(r)="<<RMSE/size<<endl;
    cout<<"MSE(g)="<<GMSE/size<<endl;
    cout<<"MSE(b)="<<BMSE/size<<endl;
    cout<<"SNR   ="<<(RSNR+GSNR+BSNR)/(RMSE+GMSE+BMSE)<<endl;
    cout<<"SNR(r)="<<RSNR/RMSE<<endl;
    cout<<"SNR(g)="<<GSNR/GMSE<<endl;
    cout<<"SNR(b)="<<BSNR/BMSE<<endl;
}

