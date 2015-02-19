#include "testApp.h"

/*
 * this is just a basic app to explore what curl looks like and some of the
 * various things you can do with it.
 *
 * in particular the way I store noise/gradients/curl in a vector is more 
 * about looking at the details than a useful way to do things.
 *
 * you can look at a plane and what the gradient/curl looks like as well as 
 * simple particle advection
 */

float width, height;
float gridw, gridh;

vector<float> noisegrid;
vector<ofVec2f> gradgrid;
vector<ofVec2f> curlgrid;

ofMesh m;
float t;

//--------------------------------------------------------------
void testApp::setup(){

    ofSetFrameRate(30);
    
    ofSetBackgroundAuto(false);
    
    width = ofGetWidth();
    height = ofGetHeight();
    
    showGui = true;
    setupGui(10, 10);
    
    noiseDirty = true;
    planeDirty = true;
    
    setupParts(PART_MAX);
    t = 0;
}

//--------------------------------------------------------------
void testApp::update(){
    
    //you can vary the noise over time by setting dt
    if (dt > 0) {
        t += dt/10000.0f;
        noiseDirty = true;
    }
    
    if (noiseDirty)
        buildGrid(noiseGridSize, noiseGridSize, signedNoise);
    
    if (planeDirty)
        setupMesh(noiseGridSize + 1, noiseGridSize + 1);

    if (showParts) {
        advect();
        vbo.setVertexData(&parts[0], partCnt, GL_STREAM_DRAW);
    }
    
}

//--------------------------------------------------------------
void testApp::draw(){
    
    ofPushStyle();
    if (doBlend)
        ofSetColor(20, 20);
    else
        ofSetColor(0, 255);
    
    ofRect(0, 0, width, height);
    ofPopStyle();

    ofSetColor(255);
    
    cam.begin();
   
    if (showAxes)
        ofDrawAxis(300);
    
    if (showPlane) {
        ofPushStyle();
        ofSetColor(20, 20, 200);
        m.drawWireframe();
        ofPopStyle();
    }

    ofPushMatrix();
    ofTranslate(width * -0.5, height * -0.5);
    
    if (showNoise || showVectors || showGrad || showCurl) {
        drawGrid();
    }
    
    if (showParts) {
        ofSetColor(255, partAlpha);
        glPointSize(1);
        vbo.draw(GL_POINTS, 0, partCnt);
    }

    ofPopMatrix();
        
    
    cam.end();
    
    if (showGui) {
        ctlGui.draw();
    }


}

void testApp::drawGrid()
{
    
    ofPushStyle();
    ofSetColor(255);
    
    for (int y = 0; y < gridh; y++) {
        for (int x = 0; x < gridw; x++) {
            int xloc = x/(gridw - 1) * width;
            int yloc = y/(gridh - 1) * height;
            int idx = y * gridh + x;
            float n = noiseAt(x/gridw, y/gridh);
            
            if (showNoise) {
                float nv = n;
                if (signedNoise)
                    nv = (n * 0.5) + 0.5;
                ofSetColor(nv * 255);
                ofRect(xloc, yloc, width/gridw, height/gridh);
            }
            
            if (showVectors) {
                ofSetColor(200, 20, 20);
                ofVec2f v = noise2dAt(x/gridw, y/gridh);
                int x1 = xloc + v.x * vecLength * 0.25;
                int y1 = yloc + v.y * vecLength * 0.25;
                ofLine(xloc, yloc, 1, x1, y1, 1);
            }
            
            if (showGrad) {
                ofVec3f g = gradAt(x/gridw, y/gridh);
                ofSetColor(20, 200, 20);
                float gx, gy, gz;
                gx = xloc + g.x * vecLength;
                gy = yloc + g.y * vecLength;
                gz = n * planeDisp;
                if (!signedNoise)
                    gz = max(2.f, gz);
                ofLine(xloc, yloc, gz, gx, gy, gz);
            }
            
            if (showCurl) {
                ofVec3f c = curlAt(x/gridw, y/gridh);
                ofSetColor(200, 200, 20);
                float cx, cy, cz;
                cx = xloc + c.x * vecLength;
                cy = yloc + c.y * vecLength;
                cz = n * planeDisp;
                if (!signedNoise)
                    cz = max(3.f, cz);
                ofLine(xloc, yloc, cz, cx, cy, cz);
            }
        }
    }
    
    ofPopStyle();
}

//parts functions

void testApp::advect()
{
    for (int i = 0; i < partCnt; i++) {
        ofVec3f &p = parts[i];
        if (p.x <= 1 || p.x >= width - 2 || p.y <= 1 || p.y >= height - 2)
            p = new_loc();
        
        switch (advectMode) {
            case ADVECT_MODE_FIXED:
                advect_fixed(i);
                break;
            case ADVECT_MODE_NOISE:
                advect_noise(i);
                break;
            case ADVECT_MODE_GRAD:
                advect_grad(i);
                break;
            case ADVECT_MODE_CURL:
                advect_curl(i);
                break;
        }
    }
}
//move with fixed velocity only
void testApp::advect_fixed(int idx)
{
   parts[idx] += ofVec2f(0, fixedYvel) * partSpeed;
}

//move based of noise
void testApp::advect_noise(int idx)
{
    ofVec3f &p = parts[idx];
    ofVec2f n = noise2dAt(p.x/width, p.y/height);
    p += (n * advectAmt + ofVec2f(0, fixedYvel)) * partSpeed;
}

//move based off gradient
void testApp::advect_grad(int idx)
{
    ofVec3f &p = parts[idx];
    ofVec2f g = gradAt(p.x/width, p.y/height);
    p += (g * advectAmt + ofVec2f(0, fixedYvel)) * partSpeed;
}

//move based off curl
void testApp::advect_curl(int idx)
{
    ofVec3f &p = parts[idx];
    ofVec2f c = curlAt(p.x/width, p.y/height);
    p += (c * advectAmt + ofVec2f(0, fixedYvel)) * partSpeed;
}

/* grid lookup functions */

/*
 * turn normalized [0 - 1] x/y values into an index for the various vectors 
 */
inline int testApp::index(float xv, float yv)
{
    xv = ofClamp(xv, 0.0, 1.0);
    yv = ofClamp(yv, 0.0, 1.0);
    int x = floor(xv * (gridw - 1));
    int y = floor(yv * (gridh - 1));
    return (int)(y * gridh + x);
}

float testApp::noiseAt(float xv, float yv)
{
    return noisegrid[index(xv, yv)];
}

ofVec2f testApp::curlAt(float xv, float yv)
{
    return curlgrid[index(xv, yv)];
}

ofVec2f testApp::gradAt(float xv, float yv)
{
    return gradgrid[index(xv, yv)];
}

/*
 * generate 2d vector of noise values depending on what options are set
 */
ofVec2f testApp::noise2dAt(float xv, float yv)
{
    float n = noiseAt(xv, yv);
    ofVec2f v;
    if (angularNoise) {
        if (signedNoise)
            v = ofVec2f(cos(PI * n), sin(PI * n));
        else
            v = ofVec2f(cos(TWO_PI * n), sin(TWO_PI * n));
    } else {
        v = ofVec2f(n, n);
    }
    return v;
}

/*
 * takes a normalized x/y, returns 2D vector of gradient approximations
 */
ofVec2f testApp::calcGradAt(float xv, float yv)
{
    ofVec2f n1, n2;
    n1 = noise2dAt(xv + 1/gridw, yv);
    n2 = noise2dAt(xv - 1/gridw, yv);
    float dx = n1.x - n2.x;
    
    n1 = noise2dAt(xv, yv + 1/gridh);
    n2 = noise2dAt(xv, yv - 1/gridh);
    float dy = n1.y - n2.y;

    return ofVec2f(dx, dy);
}

/*
 * calculate the gradients for each of the noise values
 */
void testApp::buildGrad()
{
    if (gradgrid.size() != noisegrid.size())
        gradgrid.resize(noisegrid.size());
    
    for (int y = 0; y < gridh; y++) {
        for (int x = 0 ; x < gridw; x++) {
            int idx = y * gridh + x;
            gradgrid[idx] = calcGradAt(x/gridw, y/gridh);
        }
    }
}

/*
 * use the gradient values to generate the curl info
 */
void testApp::buildCurl()
{
    if (curlgrid.size() != gradgrid.size())
        curlgrid.resize(gradgrid.size());
    
    for (int i = 0; i < gradgrid.size(); i++) {
        ofVec2f g = gradgrid[i];
        curlgrid[i] = ofVec3f(g.y, -g.x);
    }

}

/*
 * generate a bunch of noise values
 */

void testApp::buildGrid(float numrows, float numcols, bool signedNoise)
{
    if (!noiseDirty)
        return;
    
    gridw = numrows;
    gridh = numcols;
    noisegrid.resize(gridw * gridh);
    
    for (int y = 0; y < gridh; y++) {
        for (int x = 0; x < gridw; x++) {
            float nx;
            int idx = y * gridh + x;
            float xv = (x/gridw + t) * noiseScale;
            float yv = (y/gridh + t) * noiseScale;
            
            if (signedNoise)
                nx = ofSignedNoise(xv, yv);
            else
                nx = ofNoise(xv, yv);
            noisegrid[idx] = nx;
        }
    }
    
    buildGrad();
    buildCurl();
    
    noiseDirty = false;
}

void testApp::setupMesh(float numrows, float numcols)
{
    m = ofPlanePrimitive(width, height, numrows, numcols).getMesh();
    
    vector<ofVec3f> & verts = m.getVertices();
    for (int i = 0; i < verts.size(); i++) {
        ofVec3f v = verts[i];
        float vx = (v.x/width) + 0.5;
        float vy = (v.y/height) + 0.5;
        float nv = noiseAt(vx, vy);
        verts[i].z = nv * planeDisp;
    }
    
    planeDirty = false;
}

ofVec3f
testApp::new_loc()
{
    //recycle particles from the bottom of the screen
    if (advectMode == ADVECT_MODE_FIXED || (advectMode == ADVECT_MODE_CURL && fixedYvel > 1e-4))
        return ofVec3f(ofRandom(1, width - 2), ofRandom(2, 50), 3);
    return rand_loc();
}

ofVec3f
testApp::rand_loc()
{
    //recycle particles anywhere in the window
    return ofVec3f(ofRandom(1, width-2), ofRandom(1, height-2), 3);
}

void testApp::setupParts(int sz)
{
    parts.resize(sz);
    for (int i = 0; i < sz; i++) {
        parts[i] = rand_loc();
    }
}

void testApp::setupGui(int x, int y)
{
    ctlGui.setup("Controls", ctlGuiFile, x, y);
    ctlGui.add(showAxes.setup("axes", true));
    ctlGui.add(noiseGridSize.setup("noise size", 30, 10, 200));
    ctlGui.add(noiseScale.setup("noise scale", 1.0, 1.0, 15.0));
    ctlGui.add(dt.setup("dt", 0, 0, 30));
    ctlGui.add(showNoise.setup("show noise", true));
    ctlGui.add(angularNoise.setup("ang noise", true));
    ctlGui.add(signedNoise.setup("signed noise", true));
    ctlGui.add(vecLength.setup("vec length", 10, 1, 200));
    ctlGui.add(showVectors.setup("show vectors", true));
    ctlGui.add(showGrad.setup("show grad", true));
    ctlGui.add(showCurl.setup("show curl", true));
    ctlGui.add(showPlane.setup("show plane", true));
    ctlGui.add(planeDisp.setup("plane z disp", 100, 0, 500));
    
    ctlGui.add(showParts.setup("show parts", true));
    ctlGui.add(partCnt.setup("part count", 10000, 100, PART_MAX));
    ctlGui.add(partAlpha.setup("part alpha", 255, 0, 255));
    ctlGui.add(fixedYvel.setup("y vel", 1.0, 0.0, 3.0));
    ctlGui.add(partSpeed.setup("speed", 1.0, 0.0, 5.0));
    ctlGui.add(advectAmt.setup("advect amount", 1.0, 0.0, 5.0));
    ctlGui.add(advectMode.setup("advect mode", 0, 0, 3));
    ctlGui.add(advectModeName.setup("mode", "blah"));
    ctlGui.add(doBlend.setup("blend", false));


    noiseGridSize.addListener(this, &testApp::ctlGuiNoiseIntChanged);
    noiseScale.addListener(this, &testApp::ctlGuiNoiseFloatChanged);
    angularNoise.addListener(this, &testApp::ctlGuiNoiseBoolChanged);
    signedNoise.addListener(this, &testApp::ctlGuiNoiseBoolChanged);
    planeDisp.addListener(this, &testApp::ctlGuiPlaneIntChanged);
    advectMode.addListener(this, &testApp::ctlGuiAdvectModeChanged);
    
    
    ctlGui.loadFromFile(ctlGuiFile);

    partCnt = ofClamp(partCnt, 100, PART_MAX);
}

//when noise params change, rebuild the plane as its size is the same as the noise size for this example
void testApp::ctlGuiNoiseFloatChanged(float &v) { noiseDirty = true; planeDirty = true; }
void testApp::ctlGuiNoiseIntChanged(int &v) { noiseDirty = true; planeDirty = true; }
void testApp::ctlGuiNoiseBoolChanged(bool &v) { noiseDirty = true; planeDirty = true; }
void testApp::ctlGuiPlaneIntChanged(int &v) { planeDirty = true; }

void testApp::ctlGuiAdvectModeChanged(int &v)
{
    string s;
    switch (v) {
        case ADVECT_MODE_FIXED:
            s = "fixed";
            break;
        case ADVECT_MODE_NOISE:
            s = "noise";
            break;
        case ADVECT_MODE_GRAD:
            s = "grad";
            break;
        case ADVECT_MODE_CURL:
            s = "curl";
            break;
    }
    advectModeName = s;
}

void testApp::exit()
{
    ctlGui.saveToFile(ctlGuiFile);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    
    switch (key) {
        case 'g':
            showGui = !showGui;
            break;
        case 'r':
            setupParts(partCnt);
            break;
        case 's': {
            stringstream ss;
            
            ss << ofGetTimestampString("%Y%m%d_%H%M%s") << ".png";
            ofSaveScreen(ss.str());
            cout << "saved to " << ss.str() << endl;
        }
        default:
            break;
    }
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
    if (x >= 10 && x < ctlGui.getWidth() + 10 && y >= 10 && y < ctlGui.getHeight())
        cam.disableMouseInput();
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
    cam.enableMouseInput();
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
