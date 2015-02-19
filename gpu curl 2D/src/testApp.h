#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "OctaveNoise.h"

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
    
    void setupCtlGui(int x, int y);
    void saveCtlGui();
    void setupNoise();
    void updateNoise();
    
    ofShader curl;
    
    void genCurl();
    
    void setupTranformFeedback();
    
    void advect();
    
    void imgMinMax(ofFloatImage &img);
    
    ofxPanel ctlGui;
    string ctlGuiFile = "gui.ctls.xml";
    ofxIntSlider numOctaves;
    ofxFloatSlider noiseAlpha;
    ofxFloatSlider noiseBeta;
    ofxToggle noiseSigned;
    ofxToggle noiseDebugView;    
    ofxFloatSlider constXval;
    ofxFloatSlider constYval;
    ofxFloatSlider curlAmt;
    ofxFloatSlider advectDt;
    ofxFloatSlider drawAlpha;

    
    void noiseParamIntChanged(int & v);
    void noiseParamFloatChanged(float & v);
    void noiseParamBoolChanged(bool & v);
    bool noiseDirty;
};



class Timer {
public:
    
    void begin() {
        begin("timer");
    }
    void begin(string name) {
        val = 0;
        ts = ofGetElapsedTimef();
        te = 0;
        this->name = name;
    }
    void end() {
        te = ofGetElapsedTimef();
        val = (te - ts) * 1000.0f;
//        print();
    }
    void print() {
        cout << name << ": " << ofToString(val, 4, 7, ' ') << endl;
    }
    float getval() { return val; }
private:
    string name;
    float ts, te, val;
    
};