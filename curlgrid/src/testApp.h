#pragma once

#include "ofMain.h"
#include "ofxGui.h"

class testApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();
    void exit();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    int index(float xv, float yv);
    void drawGrid();
    void buildGrid(float numrows, float numcols, bool signedNoise);
    void setupGui(int x, int y);
    float noiseAt(float x, float y);
    ofVec2f calcGradAt(float x, float y);
    void buildGrad();
    void buildCurl();
    ofVec2f curlAt(float x, float y);
    ofVec2f gradAt(float x, float y);
    ofVec2f noise2dAt(float x, float y);
    void setupMesh(float numrows, float numcols);

    
    //parts
    #define PART_MAX 100000
    vector<ofVec3f> parts;
    ofVbo vbo;

    ofVec3f new_loc();
    ofVec3f rand_loc();
    void setupParts(int sz);
    
    void advect();
    void advect_fixed(int idx);
    void advect_noise(int idx);
    void advect_grad(int idx);
    void advect_curl(int idx);
    
    ofEasyCam cam;
    
    ofxPanel ctlGui;
    string ctlGuiFile = "gui.ctls.xml";
    ofxIntSlider noiseGridSize;
    ofxFloatSlider noiseScale;
    ofxIntSlider vecLength;

    ofxToggle showAxes;
    ofxToggle angularNoise;
    ofxToggle signedNoise;
    ofxToggle showNoise;
    ofxFloatSlider dt;
    ofxToggle showVectors;
    ofxToggle showGrad;
    ofxToggle showCurl;
    ofxToggle showPlane;
    ofxIntSlider planeDisp;
    ofxToggle showParts;
    ofxIntSlider partAlpha;
    ofxIntSlider partCnt;
    ofxFloatSlider partSpeed;
    ofxIntSlider advectMode;
#define ADVECT_MODE_FIXED   0
#define ADVECT_MODE_NOISE   1
#define ADVECT_MODE_GRAD    2
#define ADVECT_MODE_CURL    3
    ofxLabel advectModeName;
    ofxFloatSlider fixedYvel;
    ofxFloatSlider advectAmt;
    ofxToggle doBlend;
    

    bool noiseDirty;
    void ctlGuiNoiseFloatChanged(float &v);
    void ctlGuiNoiseIntChanged(int &v);
    void ctlGuiNoiseBoolChanged(bool &v);
    
    bool planeDirty;
    void ctlGuiPlaneIntChanged(int &v);
    
    
    void ctlGuiAdvectModeChanged(int &v);
    
    bool showGui;
};
