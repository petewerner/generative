#include "ofMain.h"
#include "testApp.h"
#include "ofAppGlutWindow.h"

//========================================================================
int main( ){

    ofAppGlutWindow window;
//	ofSetupOpenGL(&window, 1024, 768, OF_WINDOW);
//	ofSetupOpenGL(&window, 1280, 800, OF_WINDOW);
//    ofSetupOpenGL(&window, 1024,1024, OF_WINDOW);
//    ofSetupOpenGL(&window, 720, 720, OF_WINDOW);
//    ofSetupOpenGL(&window, 720, 720, OF_FULLSCREEN);
    ofSetupOpenGL(&window, 1280, 720, OF_FULLSCREEN);
//    ofSetupOpenGL(&window, 1280, 720, OF_WINDOW);


	ofRunApp( new testApp());

}
