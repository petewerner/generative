#include "testApp.h"

ofFloatImage img;
bool dirty;
float average;
float timeMS;

ofShader mr;

void
fill(int tgt_x)
{
    for (int x = 0; x < img.getWidth(); x++) {
        for (int y = 0; y < img.getHeight(); y++) {
            float r = x < tgt_x ? 1.0 : 0.0;
            img.setColor(x, y, ofFloatColor(r, 0.0, 0.0));
        }
    }
    img.update();
    dirty = true;
}

//--------------------------------------------------------------
void testApp::setup(){

    average = 0.0;
    timeMS = 0.0;
    dirty = false;
    
    int w = ofNextPow2(ofGetWidth());
    int h = ofNextPow2(ofGetHeight());
    
    img.allocate(w, h, OF_IMAGE_COLOR);
    
    fill(0);
    
    fbo.allocate(w, h, GL_RGB32F);
    fbo.src->getTextureReference().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
    fbo.dst->getTextureReference().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);

    mr.load("", "mr.frag");
}

//--------------------------------------------------------------
void testApp::update(){

    if (!dirty)
        return;
    
    fbo.src->begin();
    ofClear(0);
    img.draw(0, 0);
    fbo.src->end();
    
    int lw = img.getWidth();
    int lh = img.getHeight();

    unsigned long long startTime, endTime;
    startTime = ofGetElapsedTimeMicros();
    
    while (lw >= 1) {

        fbo.dst->begin();
        ofClear(0);
        mr.begin();
        mr.setUniformTexture("tex", fbo.src->getTextureReference(), 0);
        glBegin(GL_QUADS);
        glTexCoord2d(0, 0); glVertex2d(0, 0);
        glTexCoord2d(lw, 0); glVertex2d(lw, 0);
        glTexCoord2d(lw, lh); glVertex2d(lw, lh);
        glTexCoord2d(0, lh); glVertex2d(0, lh);
        glEnd();
        mr.end();
        fbo.dst->end();
        fbo.swap();
        
        lw = lw/2.0;
        lh = lh/2.0;
    }
    endTime = ofGetElapsedTimeMicros();
    timeMS = (endTime-startTime)/1000.0;

    float b1;
    fbo.dst->begin();
    glReadPixels(0, 0, 1, 1, GL_RGB, GL_FLOAT, &b1);
    fbo.dst->end();
    average = b1;

    dirty = false;
    
}

//--------------------------------------------------------------
void testApp::draw(){
    
    img.draw(0, 0, ofGetWidth(), ofGetHeight());
    
    stringstream msg;
    msg << "average: " << ofToString(average, 3) << endl;
    msg << "time (ms): " << ofToString(timeMS, 3) << endl;

    ofDrawBitmapString(msg.str(), 20, 20);

}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
    fill(x/(float)ofGetWidth() * img.getWidth());
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}