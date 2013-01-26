//
//  dejongAttractor.cpp
//  dejong_3
//
//  Created by Peter Werner on 3/01/13.
//
//

#include "dejongAttractor.h"

dejongAttractor::dejongAttractor()
{
    width = 100;
    height = 100;
    nparts = 300;
    shaderverts = 8;
    nverts = 0;
    isSetup = false;
    mindens = 0.0;
    useBlur = false;
    dirty = true;
    a = 1.4;
    b = -2.3;
    c = 2.4;
    d = -2.1;
    zoom = 0.5;
//kinda dodge but it will clip at 1.0 max anyway and a lot faster
    logmaxd = 3.0;
    
}

void
dejongAttractor::setup(int width, int height, int nparts, int shaderverts)
{
    
    if (isSetup) {
        vertvbo.clear();
        isSetup = false;
    }
    
//    width = ofNextPow2(width);
//    height = width;
    this->width = width;
    this->height = height;
    
    attractorFbo.allocate(width, height, GL_RGBA32F);
    attractorFbo.getTextureReference().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);

    drawFbo.allocate(width, height, GL_RGBA32F);
    drawFbo.getTextureReference().setTextureWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
    blurFbo.allocate(width, height, GL_RGBA32F);
    blurFbo.getTextureReference().setTextureWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
    
    this->width = width;
    this->height = height;
    this->nparts = nparts;
    this->nverts = nparts * nparts;
    this->shaderverts = shaderverts;
    
    //compile shader
    
    attractorShader.setGeometryInputType(GL_POINTS);
    attractorShader.setGeometryOutputType(GL_POINTS);
    attractorShader.setGeometryOutputCount(this->shaderverts);
    attractorShader.load("djaw.vert", "", "djaw.geom");
    
    drawShader.load("", "shade.frag");
    
    blurShader.load("", "simpleblur.frag");
    
    ofVec2f *vertbuf = new ofVec2f[this->nverts];
    for (int i = 0; i < this->nverts; i++) {
        vertbuf[i] = ofVec2f(ofRandom(-2.0f, 2.0f), ofRandom(-2.0f, 2.0f));        
    }
    vertvbo.setVertexData(vertbuf, this->nverts, GL_STATIC_DRAW);
    
    delete vertbuf;
    
    isSetup = true;
    dirty = true;
} 

void
dejongAttractor::setup(int width, int height, int nparts)
{
    this->setup(width, height, nparts, this->shaderverts);
}

void
dejongAttractor::setup(int width, int height)
{
    this->setup(width, height, this->nparts, this->shaderverts);
}

void
dejongAttractor::update()
{
    if (!isSetup) {
        ofLog(OF_LOG_WARNING, "dja: update called but not yet setup");
        return;
    }

//    float cval = 0.04;//1.0/255.0;
    float cval = 1.0;
    ofDisableAlphaBlending();
    attractorFbo.begin();
    ofClear(0);
    attractorShader.begin();
    attractorShader.setUniform4f("color", cval, cval, cval, 0);
    attractorShader.setUniform1f("a", this->a);
    attractorShader.setUniform1f("b", this->b);
    attractorShader.setUniform1f("c", this->c);
    attractorShader.setUniform1f("d", this->d);
    attractorShader.setUniform1f("zoom", this->zoom);
    attractorShader.setUniform1i("nverts", this->shaderverts);
    
    ofEnableAlphaBlending();

    glBlendEquation(GL_FUNC_ADD);
// pre color blending
//    glBlendFunc(GL_ONE, GL_ONE);
    glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ONE);
    
    vertvbo.draw(GL_POINTS, 0, this->nverts);
    
    attractorShader.end();
    attractorFbo.end();

    glDisable(GL_BLEND);
    
    drawFbo.begin();
    ofClear(0);
    drawShader.begin();
    drawShader.setUniformTexture("texmap", attractorFbo.getTextureReference(), 0);
    drawShader.setUniform1f("logmaxd", logmaxd);
    drawShader.setUniform1f("mindens", mindens);
    
    float w = attractorFbo.getWidth();
    float h = attractorFbo.getHeight();
    glBegin(GL_QUADS);
    glTexCoord2d(1, 1); glVertex2d(1, 1);
    glTexCoord2d(w, 1); glVertex2d(w, 1);
    glTexCoord2d(w, h); glVertex2d(w, h);
    glTexCoord2d(1, h); glVertex2d(1, h);
    glEnd();
    drawShader.end();
    drawFbo.end();
    
    if (useBlur) {
        glEnable(GL_POINT_SMOOTH);
        blurFbo.begin();
        ofClear(0);
        blurShader.begin();
        blurShader.setUniformTexture("texmap", drawFbo.getTextureReference(), 0);
        glBegin(GL_QUADS);
        glTexCoord2d(1, 1); glVertex2d(1, 1);
        glTexCoord2d(w, 1); glVertex2d(w, 1);
        glTexCoord2d(w, h); glVertex2d(w, h);
        glTexCoord2d(1, h); glVertex2d(1, h);
        glEnd();
        blurShader.end();
        blurFbo.end();
        glDisable(GL_POINT_SMOOTH);
    }
    dirty = false;
}

void
dejongAttractor::setParam(float a, float b, float c, float d)
{
    this->a = a;
    this->b = b;
    this->c = c;
    this->d = d;
    dirty = true;
}

ofTexture &
dejongAttractor::getTextureReference()
{
    if (!isSetup) {
        ofLog(OF_LOG_WARNING, "dja: getTextureReference: not setup");
        return;
    }
    
    if (dirty) {
        //paramenters changed so redraw
        this->update();
    }
    if (useBlur)
        return(blurFbo.getTextureReference());
    
    return(drawFbo.getTextureReference());
}

ofTexture &
dejongAttractor::getAttractorTextureReference()
{
    if (!isSetup) {
        ofLog(OF_LOG_WARNING, "dja: getAttractorTextureReference: not setup");
        return;
    }
    
    if (dirty)
        this->update();
    return(attractorFbo.getTextureReference());
}

float
dejongAttractor::getZoom()
{
    return(this->zoom);
}

void
dejongAttractor::setZoom(float zoom)
{
    if (zoom != this->zoom)
        dirty = true;
    this->zoom = zoom;
}

int
dejongAttractor::getDrawVertsCount()
{
    return(this-> nverts * this->shaderverts);
}

int
dejongAttractor::getInputVertsCount()
{
    return(this->nverts);
}

float
dejongAttractor::getMinDensity()
{
    return(this->mindens);
}

void
dejongAttractor::setMinDensity(float mindens)
{
    if (mindens != this->mindens)
        dirty = true;
    this->mindens = mindens;
}

int
dejongAttractor::getWidth()
{
    return(this->width);
}

int
dejongAttractor::getHeight()
{
    return(this->height);
}

void
dejongAttractor::blurOn()
{
    if (useBlur == false)
        dirty = true;
    useBlur = true;
}

void
dejongAttractor::blurOff()
{
    if (useBlur == true)
        dirty = true;
    useBlur = false;
}

bool
dejongAttractor::isBlurOn()
{
    return(this->useBlur);
}

void
dejongAttractor::blurToggle()
{
    this->useBlur = !this->useBlur;
    dirty = true;
}