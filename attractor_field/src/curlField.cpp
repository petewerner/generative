//
//  curl.cpp
//  curlfield_8
//
//  Created by Peter Werner on 10/01/13.
//
//

#include "curlField.h"


curlField::curlField()
{
    width = height = 256;
    minmag = GL_NEAREST;
    scale = 10.0f;//0.5;
    isSetup = false;
}

void
curlField::setup(ofImage &img)
{
    this->setup(img.getWidth(), img.getHeight(), this->minmag);
}

void
curlField::setup(int width, int height)
{
    this->setup(width, height, this->minmag);
}

void
curlField::setup(int width, int height, GLint minmag)
{
//    this->width = ofNextPow2(width);
//    this->height = ofNextPow2(height);
    this->width = width;
    this->height = height;
    
    curlFbo.allocate(this->width, this->height, GL_RGBA32F);
    curlFbo.getTextureReference().setTextureMinMagFilter(minmag, minmag);
    curlShader.load("", "curlfield.frag");
    needPix = false;
    isSetup = true;
}

void
curlField::build(ofImage &img)
{
    this->build(img.getTextureReference());
}

void
curlField::build(ofTexture &tex)
{
    if (!isSetup) {
        ofLog(OF_LOG_ERROR, "curlField::build: not setup");
        return;
    }
    
    int w = this->width;
    int h = this->height;
    
    curlFbo.begin();
    ofClear(0);
    curlShader.begin();
    curlShader.setUniformTexture("tex", tex, 0);
    curlShader.setUniform1f("curl_scale", this->scale);
    
    glBegin(GL_QUADS);
    glTexCoord2d(0, 0); glVertex2d(0, 0);
    glTexCoord2d(w, 0); glVertex2d(w, 0);
    glTexCoord2d(w, h); glVertex2d(w, h);
    glTexCoord2d(0, h); glVertex2d(0, h);
    glEnd();
    curlShader.end();
    curlFbo.end();
    needPix = true;
}

ofVec2f
curlField::getCurl(int x, int y)
{
    if (needPix) {
        curlFbo.readToPixels(pix);
        needPix = false;
    }
        
    ofFloatColor c = pix.getColor(x, y);
    return(ofVec2f(c.r, c.b));
}

ofTexture &
curlField::getTextureReference()
{
    if (!isSetup) {
        ofLog(OF_LOG_ERROR, "curlField::getTexRef: not set up");
    }
    return(curlFbo.getTextureReference());
}

void
curlField::draw(float x, float y)
{
    if (!isSetup) {
        ofLog(OF_LOG_ERROR, "curlField::draw: not set up");
    }

    curlFbo.draw(0, 0);
}

void
curlField::draw(float x, float y, float width, float height)
{
    if (!isSetup) {
        ofLog(OF_LOG_ERROR, "curlField::draw: not set up");
    }

    curlFbo.draw(x, y, width, height);
}

int
curlField::getWidth()
{
    return(this->width);
}

int
curlField::getHeight()
{
    return(this->height);
}

float
curlField::getScale()
{
    return(this->scale);
}

void
curlField::setScale(float s)
{
    this->scale = s;
}

ofFloatPixels
curlField::getPixels()
{
    if (needPix) {
        curlFbo.readToPixels(this->pix);
        needPix = false;
    }
    
    return(this->pix);
}

void
curlField::getPixels(ofPixels &pix)
{
    curlFbo.readToPixels(pix);
}

