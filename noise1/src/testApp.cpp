#include "testApp.h"

/*
 * app to mess around with fractional sums of perlin noise, Pete Werner 2014
 * http://petewerner.blogspot.com/2014/04/fractional-sums-of-perlin-noise.html
 *
 * see also:
 *
 * http://mrl.nyu.edu/~perlin/doc/oscar.html
 * http://paulbourke.net/texture_colour/perlin/
 *
 */

/*
 * i generate a small noise map and use image.resize() to scale it up
 * it makes computation a lot quicker, and the scaling is passable even
 * at quite low res.
 * anyway, change these numbers to use less scaling
 */
int img_w = 640 * 0.06125;
int img_h = 480 * 0.06125;

float offset;

ofFloatImage img; //final noise image
vector<ofFloatImage>images; //the successive octaves, we only keep these to show them in the app
bool dirty = true;

//--------------------------------------------------------------
void testApp::setup(){
    ofBackground(20);
    ofSetFrameRate(30);
    
    gui.setup("Perlin Noise", "", 20, 20);
    gui.add(alpha.setup("alpha", 2.0, 0.5, 10.0));
    gui.add(beta.setup("beta", 2.0, 0.5, 5.0));
    gui.add(octaveCnt.setup("octaves", 6, 1, 8));
    gui.add(norm.setup("normalize", true));
    gui.add(offset_incr.setup("speed", 0.0, 0.0, 0.2));
    alpha.addListener(this, &testApp::floatValChanged);
    beta.addListener(this, &testApp::floatValChanged);
    octaveCnt.addListener(this, &testApp::octaveCntChanged);
    norm.addListener(this, &testApp::boolValChanged);
    
    img.allocate(img_w, img_h, OF_IMAGE_GRAYSCALE);
    
    for (int i = 0; i < octaveCnt; i++) {
        ofImage tmp;
        tmp.allocate(img_w, img_h, OF_IMAGE_GRAYSCALE);
        images.push_back(tmp);
    }
    
    offset = 0;

}

//--------------------------------------------------------------
void testApp::update(){


    if (offset_incr > 0) {
        offset += offset_incr;
        dirty = true;
    }

    if (dirty)
        renderNoise(img);
}

//--------------------------------------------------------------
void testApp::draw(){

    int disp_img_w = 153;
    int disp_img_h = 120;
    int disp_img_off_x = 250;
    int disp_img_off_y = 10;
    int spc = 10;

    for (int i = 0; i < images.size(); i++) {
        int row = i / 4;
        int col = i % 4;
        ofPushMatrix();
        ofTranslate(disp_img_off_x + (disp_img_w + spc) * col, disp_img_off_y + (disp_img_h + spc) * row);
        images[i].draw(0, 0, disp_img_w, disp_img_h);
        ofPopMatrix();
    }


    ofPushMatrix();
    ofTranslate(disp_img_off_x, (disp_img_h + spc) * 2 + spc);
    img.draw(0, 0, 640, 480);
    ofPopMatrix();
    
    gui.draw();
//    ofDrawBitmapString(ofToString(ofGetFrameRate(), 0), 20, ofGetHeight() - 20);
}

void
testApp::renderNoise(ofFloatImage &img)
{
    double maxval = 0.0;
    double minval = 10.0;
    
    for (int i = 0; i < img.getWidth(); i++) {
        for (int j = 0; j < img.getHeight(); j++) {
            double sum = 0;
            double scale = 1.0;
            double p[2];
            p[0] = i /(float)img.getWidth() + offset;
            p[1] = j /(float)img.getHeight();
            
            for (int k = 0; k < images.size(); k++) {
                double val = ofNoise(p[0], p[1]);
                images[k].setColor(i, j, val);
                sum += val / scale;
                scale *= alpha;
                p[0] *= beta;
                p[1] *= beta;
            }
            img.setColor(i, j, sum);
            if (sum > maxval)
                maxval = sum;
            if (sum < minval)
                minval = sum;
        }
    }
    
    if (norm) {
        for (int i = 0; i < img.getWidth(); i++) {
            for (int j = 0; j < img.getHeight(); j++) {
                float v = img.getColor(i, j).r;
                img.setColor(i, j, v/maxval);
//                img.setColor(i, j, ofMap(v, 0.0, maxval, 0.0, 1.0));
//                img.setColor(i, j, ofMap(v, minval, maxval, 0.0, 1.0));
            }
        }
    }
    
    img.update();
    
    for (ofFloatImage &i : images)
        i.update();
    
    dirty = false;
    
}


void
testApp::octaveCntChanged(int &val)
{
    if (octaveCnt > images.size()) {
        for (int i = images.size(); i < octaveCnt; i++) {
            ofFloatImage tmp;
            tmp.allocate(img_w, img_h, OF_IMAGE_GRAYSCALE);
            images.push_back(tmp);
        }
        dirty = true;
    } else if (octaveCnt < images.size()) {
        images.erase(images.begin() + octaveCnt);
        dirty = true;
    }
}

void testApp::floatValChanged(float &val) { dirty = true; }
void testApp::boolValChanged(bool &val) { dirty = true; }

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){

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