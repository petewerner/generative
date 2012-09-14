#pragma once

#include "ofMain.h"

//from GPUparticleSystem example

struct pingPongBuffer {
public:
    void allocate( int _width, int _height, int _internalformat = GL_RGBA, float _dissipation = 1.0f){
        // Allocate
        for(int i = 0; i < 2; i++){
            FBOs[i].allocate(_width,_height, _internalformat );
        }
        
        // Clean
        clear();
        
        // Set everything to 0
        flag = 0;
        swap();
        flag = 0;
    }
    
    void swap(){
        src = &(FBOs[(flag)%2]);
        dst = &(FBOs[++(flag)%2]);
    }
    
    void clear(){
        for(int i = 0; i < 2; i++){
            FBOs[i].begin();
            ofClear(0,255);
            FBOs[i].end();
        }
    }
    
    ofFbo& operator[]( int n ){ return FBOs[n];}
    
    ofFbo   *src;       // Source       ->  Ping
    ofFbo   *dst;       // Destination  ->  Pong
private:
    ofFbo   FBOs[2];    // Real addresses of ping/pong FBO«s
    int     flag;       // Integer for making a quick swap
};


class testApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void buildSphereMesh(int radius, int res, ofMesh & sphereMesh);
    ofShader shader;
    ofShader disp_shader;
    ofEasyCam cam;
    
    ofShader pos_shader;
    pingPongBuffer posbuf;
};
