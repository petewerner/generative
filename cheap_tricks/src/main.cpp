#include "ofMain.h"
#include "testApp.h"

//========================================================================
int main( ){

	ofSetupOpenGL(1024,768,OF_WINDOW);
//    ofSetupOpenGL(640, 480, OF_WINDOW);
//    ofSetupOpenGL(1920, 1080, OF_WINDOW);
	ofRunApp(new testApp());

}
