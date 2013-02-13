#pragma once

#include "ofMain.h"
#include "ofxUI.h"

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
    void guiEvent(ofxUIEventArgs &);
    void setGUI();
    void reset();
    
    void setLightOri(ofLight &light, ofVec3f rot);
    void setWSlider(vector<ofxUISlider *>sliders, ofColor &c, float v);
    
    ofxUICanvas *gui;
    ofEasyCam cam;
    ofLight spot;
    ofLight point;
    ofLight dir;
    ofLight amb;
    ofMaterial material;
    bool guiAlloc;
    
    ofVec3f dir_rot;
    ofVec3f spot_rot;
};
