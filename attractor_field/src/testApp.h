#pragma once

#include "ofMain.h"
#include "dejongAttractor.h"
#include "ofxBlur.h"
#include "pingPongBuffer.h"
#include "simpleTimer.h"
#include "curlField.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"

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
    void exit();

    void randmap_rebuild(float, float);
    void setparm(float, float);
#ifdef USE_KINECT
    ofVec2f blob_to_data(ofxCvBlob &, int);
#endif
    
    ofShader partshader;
    ofShader velshader;
    ofShader posshader;
    ofShader colorShader;
    ofShader debugShader;
    ofFbo debugFbo;
    dejongAttractor old_dja;
    dejongAttractor dja;
    pingPongBuffer partsFbo;
    pingPongBuffer velFbo;
    pingPongBuffer colorFbo;
    ofFbo renderFbo;
//    ofFbo velFbo;
    ofVbo parts_vbo;
    int parts_res;
	bool needDraw;
    ofxBlur *blur;
    curlField cf;
//    float * parts_buf;
    
#ifdef USE_KINECT
    ofxKinect kinect;
	ofxCvColorImage colorImg;
	
	ofxCvGrayscaleImage grayImage; // grayscale depth image
	ofxCvGrayscaleImage grayThreshNear; // the near thresholded image
	ofxCvGrayscaleImage grayThreshFar; // the far thresholded image
	
	ofxCvContourFinder contourFinder;
    
	int nearThreshold;
	int farThreshold;
	
	int angle;
#endif
};
