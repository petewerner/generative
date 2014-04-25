#pragma once

#include "ofMain.h"
#include "ofxGui.h"

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

    void renderNoise(ofFloatImage &img);
    
    ofxPanel gui;
    ofxFloatSlider alpha;
    ofxFloatSlider beta;
    ofxIntSlider octaveCnt;
    ofxFloatSlider offset_incr;
    ofxToggle norm;

    void octaveCntChanged(int &val);
    void floatValChanged(float &val);
    void boolValChanged(bool &val);


};
