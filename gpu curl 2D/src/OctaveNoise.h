//
//  OctaveNoise.h
//  gpu curl
//
//  Created by Peter Werner on 4/02/15.
//
//

#include "ofMain.h"

class OctaveNoise {
public:
    OctaveNoise(): octaves(3), alpha(2.0), beta(2.0), useSign(false) {};
    void setup(int octaves, double alpha, double beta, bool useSign = false);
    float noise(float x, float y);
    
    int getOctaves() { return octaves; }
    double getAlpha() { return alpha; }
    double getBeta() { return beta; }
    
    void setOctaves(int octaves);
    void setAlpha(double alpha);
    void setBeta(double beta);
    void setSigned(bool isSigned);
    void renderToImage(ofFloatImage &img, float xoff = 0.0, float yoff = 0.0);
    void normalizeImage(ofFloatImage &img);
    void normalizeImage(ofFloatImage &img, float minval, float maxval);
private:
    int octaves;
    double alpha;
    double beta;
    bool useSign;
};