//
//  OctaveNoise.cpp
//
//  Created by Peter Werner on 4/02/15.
//
//
/*
 * XX change p[] to ofVec2f/ofVec3f
 */
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
    
    float mult = 1.0;
    
    p[0] = x * mult;
    p[1] = y * mult;
    
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

float
OctaveNoise::noise(float x, float y, float z)
{
    double sum = 0;
    double scale = 1.0;
    double p[3];
    
    float mult = 1.0;
    p[0] = x * mult;
    p[1] = y * mult;
    p[2] = z * mult;
    
    for (int k = 0; k < octaves; k++) {
        double val;
        if (useSign)
            val = ofSignedNoise(p[0], p[1], p[2]);
        else
            val = ofNoise(p[0], p[1], p[2]);
        sum += val / scale;
        scale *= alpha;
        p[0] *= beta;
        p[1] *= beta;
        p[2] *= beta;
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
OctaveNoise::renderToImage(ofFloatImage &img, NoisePlane plane, float xoff, float yoff, float zoff)
{
    float w = img.getWidth();
    float h = img.getHeight();
    
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            float x1, y1, z1;
            float x0 = (x / (w - 1));
            float y0 = (y / (h - 1));
            
            switch (plane) {
                case NoisePlaneXY:
                    x1 = x0 + xoff;
                    y1 = y0 + yoff;
                    z1 = 0.0f;
                    break;
                case NoisePlaneXZ:
                    x1 = x0 + xoff;
                    y1 = 0.0f;
                    z1 = y0 + zoff;
                    break;
                case NoisePlaneYZ:
                    x1 = 0.0f;
                    y1 = x0 + yoff;
                    z1 = y0 + zoff;
            }
            
            float v = this->noise(x1, y1, z1);
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
OctaveNoise::normalizeImageRGB(ofFloatImage &img)
{
    ofVec4f minvals, maxvals;
    imgMinMax(img, minvals, maxvals);
    
    for (int x = 0; x < img.getWidth(); x++) {
        for (int y = 0; y < img.getHeight(); y++) {
            ofFloatColor v1 = img.getColor(x, y);
            ofFloatColor v2;
            v2.r = ofMap(v1.r, minvals.x, maxvals.x, 0.0, 1.0);
            v2.g = ofMap(v1.g, minvals.y, maxvals.y, 0.0, 1.0);
            v2.b = ofMap(v1.b, minvals.z, maxvals.z, 0.0, 1.0);
            v2.a = 1.0f;
            img.setColor(x, y, v2);
        }
    }
    img.update();

}

void OctaveNoise::imgMinMax(ofFloatImage &img, ofVec4f &minvals, ofVec4f &maxvals)
{
    minvals = ofVec4f(9000.0f);
    maxvals = ofVec4f(-9000.0f);
    
    for (int x = 0; x < img.getWidth(); x++) {
        for (int y = 0; y < img.getHeight(); y++) {
            ofFloatColor a = img.getColor(x, y);
            minvals.x = min(minvals.x, a.r);
            minvals.y = min(minvals.y, a.g);
            minvals.z = min(minvals.z, a.b);
            minvals.w = min(minvals.w, a.a);
            maxvals.x = max(maxvals.x, a.r);
            maxvals.y = max(maxvals.y, a.g);
            maxvals.z = max(maxvals.z, a.b);
            maxvals.w = max(maxvals.w, a.a);
            
        }
    }
//    cout << "minvals: " << minvals << endl;
//    cout << "maxvals: " << maxvals << endl;
    
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
