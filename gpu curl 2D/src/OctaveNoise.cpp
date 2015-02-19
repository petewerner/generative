//
//  OctaveNoise.cpp
//
//  Created by Peter Werner on 4/02/15.
//
//

#include "OctaveNoise.h"

void
OctaveNoise::setup(int octaves, double alpha, double beta, bool useSign)
{
    this->octaves = octaves;
    this->alpha = alpha;
    this->beta = beta;
    this->useSign = useSign;
}

float
OctaveNoise::noise(float x, float y)
{
    double sum = 0;
    double scale = 1.0;
    double p[2];
    
    p[0] = x;
    p[1] = y;
    
    for (int k = 0; k < octaves; k++) {
        double val;
        if (useSign)
            val = ofSignedNoise(p[0], p[1]);
        else
            val = ofNoise(p[0], p[1]);
        sum += val / scale;
        scale *= alpha;
        p[0] *= beta;
        p[1] *= beta;
    }
   
    return sum;
    
}

void OctaveNoise::renderToImage(ofFloatImage &img, float xoff, float yoff)
{
    float w = img.getWidth();
    float h = img.getHeight();
    
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            float x0 = (x / (w - 1)) + xoff;
            float y0 = (y / (h - 1)) + yoff;
            float v = this->noise(x0, y0);
            img.setColor(x, y, ofFloatColor(v));
        }
    }
    img.update();
}

void
OctaveNoise::normalizeImage(ofFloatImage &img)
{
    float minv = 9000.0f;
    float maxv = -minv;
    
    for (int x = 0; x < img.getWidth(); x++) {
        for (int y = 0; y < img.getHeight(); y++) {
            float v = img.getColor(x, y).r;
            minv = min(minv, v);
            maxv = max(maxv, v);
        }
    }
    
    return normalizeImage(img, minv, maxv);
}

void
OctaveNoise::normalizeImage(ofFloatImage &img, float minval, float maxval)
{
    for (int x = 0; x < img.getWidth(); x++) {
        for (int y = 0; y < img.getHeight(); y++) {
            float v = img.getColor(x, y).r;
            v = ofMap(v, minval, maxval, 0.0f, 1.0f);
            img.setColor(x, y, ofFloatColor(v, v, v, 1));
        }
    }
    img.update();
}

void
OctaveNoise::setOctaves(int octaves)
{
    this->octaves = max(1, octaves);
}

void
OctaveNoise::setAlpha(double alpha)
{
    this->alpha = max(0.1, alpha);
}

void
OctaveNoise::setBeta(double beta)
{
    this->beta = max(0.1, beta);
}

void
OctaveNoise::setSigned(bool isSigned)
{
    this->useSign = isSigned;
}
