//
//  dejongAttractor.h
//  dejong_3
//
//  Created by Peter Werner on 3/01/13.
//
//

#ifndef __dejong_3__dejongAttractor__
#define __dejong_3__dejongAttractor__

#include <iostream>
#include "ofMain.h"
#include "pingPongBuffer.h"


class dejongAttractor {
  
public:
    dejongAttractor();
    void setup(int width, int height);
    void setup(int width, int height, int nparts);
    void setup(int width, int height, int nparts, int shaderverts);
    void update(); //fill the attractor. note will mess with alpha blending
    void setParam(float a, float b, float c, float d);
    ofTexture & getTextureReference();
    ofTexture & getAttractorTextureReference();
    float getZoom();
    void setZoom(float zoom);
    int getDrawVertsCount(); //total verts for final image
    int getInputVertsCount(); //number of input verts
    float getMinDensity();
    void setMinDensity(float mindens);
    int getWidth();
    int getHeight();
    void blurOn();
    void blurOff();
    bool isBlurOn();
    void blurToggle();
    float logmaxd;
    
private:
    ofFbo attractorFbo;
    ofFbo drawFbo;
    ofShader attractorShader;
    ofShader drawShader;
    ofShader blurShader;
    ofFbo blurFbo;
    ofVbo vertvbo;
    bool useBlur;
    bool dirty;
    
    int width;
    int height;
    int nparts; //number of particles used to create attractor
    int nverts; //number of vertexs actually used, will be nparts * nparts * shaderverts;
    int shaderverts; //how many verts to emit in geom shader
    bool isSetup;
    float mindens; //how many hits on an x, y to "count"

    float a;
    float b;
    float c;
    float d;
    float zoom;
    
    
};

#endif /* defined(__dejong_3__dejongAttractor__) */
