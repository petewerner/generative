//
//  curl.h
//  curlfield_8
//
//  Created by Peter Werner on 10/01/13.
//
//

#ifndef __curlfield_8__curl__
#define __curlfield_8__curl__

#include <iostream>
#include "ofMain.h"

class curlField {
    
public:
    curlField();
    
    void setup(int width, int height);
    void setup(int width, int height, GLint minmag);
    void setup(ofImage &);
    void build(ofImage &);
    void build(ofTexture &);
    ofTexture & getTextureReference();
    int getWidth();
    int getHeight();
    float getScale();
    void setScale(float);
    void draw(float, float);
    void draw(float, float, float, float);
    ofVec2f getCurl(int x, int y);
    ofFloatPixels getPixels();
    void getPixels(ofPixels &);
    
private:
    ofFbo curlFbo;
    ofShader curlShader;
    float scale;
    int width;
    int height;
    bool isSetup;
    bool setMinMag;
    GLint minmag;
    bool needPix;
    ofFloatPixels pix;
};

#endif /* defined(__curlfield_8__curl__) */
