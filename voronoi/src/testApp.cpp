#include "testApp.h"

/*
 * messing around with ofxVoronoi, example up here http://vimeo.com/61998900
 *
 * by Pete Werner 2013
 *
 * there are two modes. one lets you set the amount of points and make the 
 * tessellation and save the image (hit s to save it, the gui will hide).
 *
 * the other adds more points each iteration. this can get slow as it rebuilds
 * the map every time. You can hit x to stop the recording, all the previously 
 * rendered frames will still be in data/recording
 *
 * press m to show/hide the gui
 *
 * let me know if you do anything cool with it.
 *
 * needs ofxVoronoi, ofxUI and ofxXmlSettings
 */

//starting points
int npoints = 50;

//various state management bits
bool recording = false;
bool savelast = false;
bool needsRedraw = true;
bool save = false;

//this stuff controls the iterative recording
//will write each frame to data/recording
//be warned for large amounts of points this can
//take a long time, as the voronoi map is rebuilt
//each frame
int startpoints = 2;
int endpoints = 500;
float scalerate = 0.33;
int cur_cnt;

//file to work on, should be in data/
string filename = "wave.jpg";

//add n random points
void
testApp::addVoronoiPoints(int n)
{
    for (int i = 0; i < n; i++) {
        ofVec3f p;
        p.x = ofRandom(1.0) * ofGetWidth();
        p.y = ofRandom(1.0) * ofGetHeight();
        voronoi.addPoint(p);
    }
    
}

//build the voronoi
void
testApp::buildVoronoi()
{

    cout << "generating voronoi" << endl;
    voronoi.generateVoronoi();
}

//this is true if the point lies in the poly
bool
pnpoly(int nvert, float *vertx, float *verty, float testx, float testy)
{
    int i, j;
    bool c = false;
    for (i = 0, j = nvert-1; i < nvert; j = i++) {
        if ( ((verty[i]>testy) != (verty[j]>testy)) &&
            (testx < (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]) )
            c = !c;
    }
    return c;
}

//get the average color for a poly within a region
//for some definition of average ...
ofColor
getAverageColor(ofImage i, ofRectangle bounds, int nverts, float *vertx, float * verty)
{
    float r, g, b;
    r = g = b = 0;
    float ncols = 0;
    
    for (int x = bounds.x; x <= bounds.width; x++) {
        for (int y = bounds.y; y <= bounds.height; y++) {
            if (pnpoly(nverts, vertx, verty, x, y)) {
                ofColor c = i.getColor(x, y);
                r += c.r;
                g += c.g;
                b += c.b;
                ncols += 1.0;
            }
        }
    }
    
    return(ofColor(r/ncols, g/ncols, b/ncols));
}

//--------------------------------------------------------------
void testApp::setup(){
    
    ofBackground(0);
    ofSetFrameRate(30);
    ofEnableSmoothing();
    
    img.loadImage(filename);
    img.resize(ofGetWidth(), ofGetHeight());
  
    cur_cnt = 0;
    
    voronoi.setBounds(0, 0, ofGetWidth(), ofGetHeight());

    //the initial picture
    addVoronoiPoints(npoints);
    buildVoronoi();

    fbo.allocate(ofGetWidth(), ofGetHeight());
    
    setGUI();
}

//--------------------------------------------------------------
void testApp::update(){

    if (!recording && !needsRedraw)
        return;
    
    //recording a sequence, add some points, rebuild
    if (recording) {
        
        int cursize = voronoi.getPoints().size();
        if (cursize >= endpoints) {
            cout << "done! drew " << voronoi.cells.size() << " cells" << endl;
            recording = false;
            needsRedraw = false;
            savelast = true;
        } else  {
            int newpoints = max((double)1.0, round(cursize * scalerate));

            if (cursize + newpoints > endpoints)
                newpoints = endpoints - cursize;
            
            cout << "adding " << newpoints << " points" << endl;
            addVoronoiPoints(newpoints);
            buildVoronoi();
        }
        
    } else { //single image, add/remove points and rebuild if necessary
        

        int pointdiff = npoints - voronoi.getPoints().size();
        if (pointdiff > 0) {

            cout << "adding " << pointdiff << " points" << endl;
            addVoronoiPoints(pointdiff);
            buildVoronoi();

        } else if (pointdiff < 0) {

            cout << "removing " << abs(pointdiff) << " points" << endl;
            vector<ofPoint>& vpts = voronoi.getPoints();
            vpts.erase(vpts.begin()+npoints, vpts.begin() + vpts.size());
            buildVoronoi();
            
        }
    }

    //draw the picture
    fbo.begin();
    for (int i = 0; i < voronoi.cells.size(); i++) {
        ofxVoronoiCell cell = voronoi.cells.at(i);
        ofRectangle bound(ofGetWidth()+1, ofGetHeight()+1, -1, -1);
        
        ofPath path;
        int nverts = cell.pts.size();
        float *vertx = new float[cell.pts.size()];
        float *verty = new float[cell.pts.size()];
        
        for (int j = 0; j < cell.pts.size(); j++) {
            ofPoint p = cell.pts[j];
            if (p.x < 0)
                p.x = 0;
            
            vertx[j] = p.x;
            verty[j] = p.y;
            
            if (p.x < bound.x)
                bound.x = p.x;
            if (p.x > bound.width)
                bound.width = p.x;
            if (p.y < bound.y)
                bound.y = p.y;
            if (p.y > bound.height)
                bound.height = p.y;
            
            path.lineTo(cell.pts[j]);
        }
        
        path.setFilled(true);
        ofColor c = getAverageColor(img, bound, nverts, vertx, verty);
        path.setFillColor(c);
        path.setStrokeColor(ofColor(0, 0, 0));
        path.draw();
        
        if (c.r == 0 && c.b == 0 && c.g == 0) {
            cout << "dead spot" << endl;
            cout << bound.x << " -> " << bound.width << ", ";
            cout << bound.y << " -> " << bound.height << endl;
            
            for (int j = 0; j < cell.pts.size(); j++) {
                ofPoint p = cell.pts[j];
                cout << p << endl;
            }
        }
        
        delete [] vertx;
        delete [] verty;
    }
    fbo.end();
    if (!recording)
        needsRedraw = false;
}



//--------------------------------------------------------------
void testApp::draw(){

    //do some dancing to hide the gui
    bool curvis;
    if (save) {
        curvis = gui->isVisible();
        gui->setVisible(false);
    }
    
    fbo.draw(0, 0);

    //save a single frame
    if (save) {
        stringstream s;
        s << "voronoi_" << ofGetUnixTime() << "_" << npoints << ".png";
        ofSaveScreen(s.str());
        cout << "wrote file to " << s.str() << endl;
        gui->setVisible(curvis);
        save = false;
    }

    //recording save
    if (recording || savelast) {
        stringstream s;
        s << "record/frame_" << ofToString(cur_cnt++, 4, '0') << ".jpg";
        ofSaveScreen(s.str());
        savelast = false;
    }

}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

    switch (key) {
        case 's':
            save = true;
            break;
        case 'm':
            gui->toggleVisible();
            break;
        case 'x':
            if (recording) {
                cout << "record aborted!" << endl;
                recording = false;
            }
        default:
            break;
    }
}

void
testApp::guiEvent(ofxUIEventArgs &e)
{
	string name = e.widget->getName();
	int kind = e.widget->getKind();
    
    if (name == "npoints") {
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        npoints = (int)slider->getScaledValue();
    } else if (name == "Generate") {
        needsRedraw = true;
    } else if (name == "startpoints") {
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        startpoints = (int)slider->getScaledValue();
    } else if (name == "endpoints") {
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        endpoints = (int)slider->getScaledValue();
    } else if (name == "scalerate") {
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        scalerate = (int)slider->getScaledValue();
    } else if (name == "Record") {
        gui->setVisible(false);
        recording = true;
        voronoi.clear();
        addVoronoiPoints(startpoints);
        buildVoronoi();
    }
}

void
testApp::setGUI()
{
    
    float xInit = OFX_UI_GLOBAL_WIDGET_SPACING;
    float w = 300 - xInit * 2;
    float vertH = 40;
    float h = 8;
    
    gui = new ofxUICanvas(0, 0, w + xInit * 2, ofGetHeight());
    gui->addWidgetDown(new ofxUILabel("Single Image", OFX_UI_FONT_MEDIUM));

    gui->addSlider("npoints", 2, 5000, npoints);
    gui->addLabelButton("Generate", false);
    gui->addWidgetDown(new ofxUILabel("hit s to save", OFX_UI_FONT_SMALL));
    
    gui->addSpacer();
    gui->addWidgetDown(new ofxUILabel("Sequence Record", OFX_UI_FONT_MEDIUM));
    gui->addSlider("startpoints", 2, 499, 2);
    gui->addSlider("endpoints", 500, 25000, 500);
    gui->addSlider("scalerate", 0.01, 1.0, scalerate);
    gui->addLabelButton("Record", false);
    gui->addWidgetDown(new ofxUILabel("hit x to abort recording", OFX_UI_FONT_SMALL));
    
    ofAddListener(gui->newGUIEvent, this, &testApp::guiEvent);
    
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

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