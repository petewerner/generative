#pragma once

#include "ofMain.h"
#include "ofxKinect.h"
#include "ofxOpenCv.h"
#include "ofxFFTLive.h"
#include "ofxFboBlur.h"
#include "ofxTriangle.h"
#include "ofxGui.h"

//#define USE_OFX_SOUNDSTREAM

#ifdef USE_OFX_SOUNDSTREAM
#include "ofxSoundStream.h"
#endif

class testApp : public ofBaseApp{

public:
    ~testApp();
    void setup();
    void update();
    void draw();
    void exit();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    void setupColorMesh();
    void setMeshColors(ofColor c1, ofColor c2);
    void updateColors();
    bool readKinect();
    bool readCamera();
    void closeCamera();
    void closeKinect();
    void openCamera();
    bool openKinect();
    void setScreenSize(string name);
    
    void audioReceived(float * input, int bufferSize, int nChannels);
    void setupGpuBlur(int width, int height);
    
#ifdef USE_OFX_SOUNDSTREAM
    ofxSoundStream xss;
#else
    ofSoundStream xss;
#endif
    
	ofxKinect kinect;
	ofxTriangle triangle;
    
	ofxCvColorImage colorImg;
	
	ofxCvGrayscaleImage grayImage; // grayscale depth image
	ofxCvGrayscaleImage grayThreshNear; // the near thresholded image
	ofxCvGrayscaleImage grayThreshFar; // the far thresholded image
    ofxCvGrayscaleImage grayBg; //background subtraction for video input
    
	//The current and the previous video frames as grayscale images
	ofxCvGrayscaleImage grayImagePrev;
    
	ofxCvGrayscaleImage diff;		//Absolute difference of the frames
	ofxCvFloatImage diffFloat;		//Amplified difference images
	ofxCvFloatImage bufferFloat;	//Buffer image
    ofxCvFloatImage bufferBlur;
	
	ofxCvContourFinder contourFinder;
	
    float lows;
    ofxFboBlur *gpuBlur;
    int angle;
    
    ofxFFTLive fftLive;
    
    float *rawFFT;
    int nbins;
    
    ofMesh colorMesh;
    ofFbo colors;
    
    ofxPanel gui;
    ofxSlider<float> audioThreshold;
    ofxSlider<float> audioPeakDecay;
    ofxSlider<float> audioMaxDecay;
    ofxToggle audioMirror;
    
    ofxIntSlider nearThreshold;
    ofxIntSlider farThreshold;
    ofxIntSlider triangleMax;
    ofxIntSlider blobMin;
    ofxVec2Slider cvThresholds;
    ofxToggle mirrorInput;
    ofxToggle useGPU;
    ofxToggle drawFPS;
    ofxFloatSlider glowExtra;
    ofxFloatSlider lineWidth;
    ofxFloatSlider jitterSize;
    
    ofxGuiGroup fart;
    
    bool useKinect;
    bool kinectOpen;
    bool cameraOpen;
    bool learnBG;
    bool showGui;
    
    ofVideoGrabber vid;
    
    int inputWidth;
    int inputHeight;
    
};
