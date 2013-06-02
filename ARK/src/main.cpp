#include "ofMain.h"
#include "testApp.h"
#include "ofAppGlutWindow.h"

//========================================================================
int main( ){

    ofAppGlutWindow window;
	ofSetupOpenGL(&window, 800,800, OF_WINDOW);
    ofSetWindowPosition(ofGetWidth(), 0);

	ofRunApp( new testApp());

}
