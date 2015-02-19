#include "testApp.h"

/*
 * A simple example of generating the curl of perlin noise in 2D using the gpu
 */

OctaveNoise noiseGen;
ofFloatImage noiseImage;
ofFbo curlTarget;
ofFloatImage curlDrawImage;
ofFloatImage curlData;

Timer t;

float step;
float dt;

ofShader advectShader;

static GLuint vaoVec[2];
GLuint advectProgram;

static GLuint ParticleBufferB;
static GLuint ParticleBufferA;

GLuint SlotPosition;
u_int curvao;


int partcnt = 1000000;
vector<ofVec2f> particles;

float update_time;
float width, height;

int drawMode = 0;
int drawModeMax = 3;

bool drawGui;

//--------------------------------------------------------------
void testApp::setup(){

    width = ofGetWidth();
    height = ofGetHeight();
    
    ofDisableArbTex();
    ofEnableNormalizedTexCoords();
    
    setupCtlGui(10, 10);
    
    //dimensions of the noise texture
    float w = 64.0;
    float h = w;
    
    noiseGen.setup(3, 4.0, 3.0);
    noiseGen.setSigned(false);
    
    noiseImage.allocate(w, h, OF_IMAGE_GRAYSCALE);
    noiseGen.renderToImage(noiseImage);
    
    imgMinMax(noiseImage);
    
    step = 0;
    dt = 0.005;
    
    curl.load("curl");
    
    ofLogLevel ll = ofGetLogLevel();
    ofSetLogLevel(OF_LOG_WARNING);
    curlTarget.allocate(w, h, GL_RGBA32F_ARB);
    curlTarget.getTextureReference().setTextureMinMagFilter(GL_LINEAR, GL_LINEAR);
    curlTarget.getTextureReference().setTextureWrap(GL_CLAMP, GL_CLAMP);

    ofSetLogLevel(ll);
    
    setupTranformFeedback();

    drawGui = true;
}

//--------------------------------------------------------------
void testApp::update(){

    //this checks to see if noise params has changed and generates the curl if so
    setupNoise();
    
    step += dt;
    
    t.begin("update");
    advect();
    t.end();
    update_time = t.getval();
}

//--------------------------------------------------------------
void testApp::draw(){
    
    ofClear(0, 255);
    ofSetColor(255);
    
    switch (drawMode) {
        case 1:
            noiseImage.draw(0, 0, ofGetWidth(), ofGetHeight());
            break;
        case 2:
            curlDrawImage.draw(0, 0, ofGetWidth(), ofGetHeight());
            break;
        default:
            break;
    }
    
    glPointSize(1.0);
    glEnable(GL_BLEND);
    
    ofSetColor(254, 255 * drawAlpha);
    glBindVertexArray(vaoVec[curvao]);
    glDrawArrays(GL_POINTS, 0, partcnt);
    glBindVertexArray(0);
    
    glDisable(GL_BLEND);
    
    
    if (drawGui) {
        ofSetColor(255, 255);
        ofEnableAlphaBlending();
        ctlGui.draw();

        if (noiseDebugView) {
            float dispw, disph;
            disph = dispw = 200.0;
            
            ofPushMatrix();
            
            ofTranslate(ofGetWidth() - dispw, 0);
            noiseImage.draw(0, 0, dispw, disph);
            ofTranslate(0, disph);
            curlTarget.draw(0, 0, dispw, disph);
            ofTranslate(0, disph);
            curlDrawImage.draw(0, 0, dispw, disph);
            
            ofPopMatrix();
        }

        stringstream ss;
        ss << "FPS: " << ofToString(ofGetFrameRate(), 0) << endl
        << "upd: " << ofToString(update_time, 1) << endl
        << "parts: " << partcnt << endl
        << endl;

        ofPushStyle();
        ofPushMatrix();
        float boxw = 150.0;
        float boxh = 60.0f;
        ofTranslate(width - boxw, height - boxh);
        ofSetColor(0, 255);
        ofRect(0, 0, boxw, boxh);
        ofSetColor(255);
        ofDrawBitmapString(ss.str(), 10, 20);
        ofPopMatrix();
        ofPopStyle();
    }
    
}

void testApp::exit()
{
    saveCtlGui();
}


void testApp::genCurl()
{
    curlTarget.begin();
    ofClear(0, 255);
    ofSetColor(255, 255);
    curl.begin();
    curl.setUniformTexture("uPtex", noiseImage.getTextureReference(), 0);
    curl.setUniform2f("dims", noiseImage.getWidth(), noiseImage.getHeight());
    
    ofRect(0, 0, curlTarget.getWidth(), curlTarget.getHeight());
    curl.end();
    
    curlTarget.end();
    
    // this stuff isnt really required, done so I can see the noise/curl
    curlData.allocate(curlTarget.getWidth(), curlTarget.getHeight(), OF_IMAGE_COLOR_ALPHA);
    ofLogLevel ll = ofGetLogLevel();
    ofSetLogLevel(OF_LOG_FATAL_ERROR);
    curlTarget.getTextureReference().readToPixels(curlData.getPixelsRef());
    ofSetLogLevel(ll);
    curlData.update();
    
    cout <<"curl min/max" << endl;
    imgMinMax(curlData);
    
    curlDrawImage.allocate(curlTarget.getWidth(), curlTarget.getHeight(), OF_IMAGE_COLOR_ALPHA);
    curlDrawImage = curlData;
    noiseGen.normalizeImage(curlDrawImage);
}

void testApp::advect()
{
    advectShader.begin();
    advectShader.setUniformTexture("Sampler", curlTarget.getTextureReference(), 0);
    advectShader.setUniform1f("Time", step);
    advectShader.setUniform1f("curlamt", curlAmt);
    advectShader.setUniform2f("constVel", constXval, constYval);
    advectShader.setUniform1f("advectDt", advectDt);
    advectShader.setUniform2f("dims", ofGetWidth(), ofGetHeight());
    glBindVertexArray(vaoVec[curvao]);
    
    glEnable(GL_RASTERIZER_DISCARD);
    
    GLuint tgt = curvao == 0 ? ParticleBufferB : ParticleBufferA;
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tgt);
    
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, partcnt);
    glEndTransformFeedback();
    
    glBindVertexArray(0);
    glDisable(GL_RASTERIZER_DISCARD);
    
    advectShader.end();
    
    curvao = (curvao + 1) % 2;
}

void testApp::setupTranformFeedback()
{

    particles.resize(partcnt);
    for (int i = 0; i < partcnt; i++)
        particles[i] = ofVec2f(ofRandomWidth(), ofRandomHeight());
    
    advectShader.setupShaderFromFile(GL_VERTEX_SHADER, "advect.vert");
    advectShader.bindDefaults();
    advectProgram = advectShader.getProgram();
    
	const char* varyings[1] = { "vPosition"};
    glTransformFeedbackVaryings(advectProgram, 1, varyings, GL_INTERLEAVED_ATTRIBS);
    
    advectShader.linkProgram();
    
    SlotPosition =  advectShader.getAttributeLocation("Position");

    glGenBuffers(1, &ParticleBufferA);
    glBindBuffer(GL_ARRAY_BUFFER, ParticleBufferA);
    glBufferData(GL_ARRAY_BUFFER, partcnt * sizeof(ofVec2f), &particles[0], GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &ParticleBufferB);
    glBindBuffer(GL_ARRAY_BUFFER, ParticleBufferB);
    glBufferData(GL_ARRAY_BUFFER, partcnt * sizeof(ofVec2f), 0, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    //setup VAOs
    glGenVertexArrays(2, vaoVec);
    
    glBindVertexArray(vaoVec[0]);
    
    glBindBuffer(GL_ARRAY_BUFFER, ParticleBufferA);
    glEnableVertexAttribArray(SlotPosition);
    glVertexAttribPointer(SlotPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ofVec2f), 0);
    
    glBindVertexArray(vaoVec[1]);
    
    glBindBuffer(GL_ARRAY_BUFFER, ParticleBufferB);
    glEnableVertexAttribArray(SlotPosition);
    glVertexAttribPointer(SlotPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ofVec2f), 0);
    
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    curvao = 0;
}


#pragma mark - misc/util -

void testApp::imgMinMax(ofFloatImage &img)
{
    ofVec4f minvals(9000.0f);
    ofVec4f maxvals(-9000.0f);
    
    for (int x = 0; x < img.getWidth(); x++) {
        for (int y = 0; y < img.getHeight(); y++) {
            ofFloatColor a = img.getColor(x, y);
            minvals.x = min(minvals.x, a.r);
            minvals.y = min(minvals.y, a.g);
            minvals.z = min(minvals.z, a.b);
            minvals.w = min(minvals.w, a.a);
            maxvals.x = max(maxvals.x, a.r);
            maxvals.y = max(maxvals.y, a.g);
            maxvals.z = max(maxvals.z, a.b);
            maxvals.w = max(maxvals.w, a.a);

        }
    }
    cout << "minvals: " << minvals << endl;
    cout << "maxvals: " << maxvals << endl;
    
}

#pragma mark - Ctl Gui -

void testApp::setupCtlGui(int x, int y)
{
    ctlGui.setup("ctls", ctlGuiFile, x, y);
    
    ctlGui.add(numOctaves.setup("octaves", 1, 1, 8));
    ctlGui.add(noiseAlpha.setup("alpha", 2.0, 0.1, 8.0));
    ctlGui.add(noiseBeta.setup("beta", 2.0, 0.1, 8.0));
    ctlGui.add(noiseSigned.setup("signed", false));
    ctlGui.add(noiseDebugView.setup("noise view", false));
    
    ctlGui.add(constXval.setup("x amt", 0.0, -2.0, 2.0));
    ctlGui.add(constYval.setup("y amt", 0.0, -2.0, 2.0));
    ctlGui.add(curlAmt.setup("curl amt", 0.25, 0.0, 2.0));
    ctlGui.add(advectDt.setup("advect dt", 1.0, 0.0, 2.0));
    ctlGui.add(drawAlpha.setup("draw alpha", 1.0, 0.0, 1.0));
    
    ctlGui.loadFromFile(ctlGuiFile);
 
    numOctaves.addListener(this, &testApp::noiseParamIntChanged);
    noiseAlpha.addListener(this, &testApp::noiseParamFloatChanged);
    noiseBeta.addListener(this, &testApp::noiseParamFloatChanged);
    noiseSigned.addListener(this, &testApp::noiseParamBoolChanged);
    
    noiseDirty = true;
}

void testApp::saveCtlGui()
{
    ctlGui.saveToFile(ctlGuiFile);
}

void testApp::setupNoise()
{
    if (noiseDirty == false)
        return;
    
    noiseGen.setup(numOctaves, noiseAlpha, noiseBeta);
    noiseGen.setSigned(noiseSigned);
    updateNoise();
    noiseDirty = false;
}

void testApp::updateNoise()
{
    noiseGen.renderToImage(noiseImage);
    genCurl();
}

void testApp::noiseParamBoolChanged(bool &v) { noiseDirty = true; }
void testApp::noiseParamFloatChanged(float &v) { noiseDirty = true; }
void testApp::noiseParamIntChanged(int &v) { noiseDirty = true; }

#pragma mark - oF events -

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    switch (key) {
        case 'm':
            drawMode = (drawMode + 1) % drawModeMax;
            break;
        case 'g':
            drawGui = !drawGui;
            break;
        case 'f':
            ofToggleFullscreen();
            width = ofGetWidth();
            height = ofGetHeight();
            break;
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