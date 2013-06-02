#pragma once

#include "ofMain.h"

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
   
        void customDraw3d();
    
        void drawFbo();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		  
    
    int ecran = (ofGetWidth()/2);
    
    ofFbo Mainfbo;
    ofFbo fbo1;
    ofFbo fbo2;

    vector<ofVec3f> points;

    ofMesh mesh1;
   
};
