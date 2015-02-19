#include "testApp.h"

/*
 * A simple example of curl in 3D.
 */

ofEasyCam cam;
OctaveNoise noiseGen;

//these are our noise planes
//the images are just for info/debugging
ofFbo curlTargetXY, curlTargetXZ, curlTargetYZ;
ofFloatImage noiseImageXY, noiseImageXZ, noiseImageYZ;
ofFloatImage curlDrawXY, curlDrawXZ, curlDrawYZ;

Timer t;
float step;
float dt;

float boundX, boundY, boundZ;

static GLuint vaoVec[2];
ofShader advectShader;
GLuint advectProgram;

static GLuint ParticleBufferB;
static GLuint ParticleBufferA;

const int partcnt = 1000000;
vector<ofVec3f> particles;

GLuint SlotPosition;
u_int curvao;

float update_time;
float width, height;

bool drawGui;

//--------------------------------------------------------------
void testApp::setup(){

    width = ofGetWidth();
    height = ofGetHeight();

    //these are the particle spawn dimensions
    boundX = width;
    boundY = height * 2.0; //x2 just to make the bounding box a bit taller
    boundZ = 1000;
    
    ofDisableArbTex();
    ofEnableNormalizedTexCoords();
    
    setupCtlGui(10, 10);
    
    //dimensions of noise texutres
    float w = 64.0f;
    float h = w;
    
    noiseGen.setup(8, 4.0, 3.0);
    noiseGen.setSigned(false);
    
    noiseImageXY.allocate(w, h, OF_IMAGE_GRAYSCALE);
    noiseImageXZ.allocate(w, h, OF_IMAGE_GRAYSCALE);
    noiseImageYZ.allocate(w, h, OF_IMAGE_GRAYSCALE);
    
    step = 0;
    dt = 0.005;
    
    curl.load("curl");
    
    GLint wrapMode = GL_REPEAT;
    
    ofLogLevel ll = ofGetLogLevel();
    ofSetLogLevel(OF_LOG_WARNING);

    curlTargetXY.allocate(w, h, GL_RGBA32F_ARB);
    curlTargetXY.getTextureReference().setTextureMinMagFilter(GL_LINEAR, GL_LINEAR);
    curlTargetXY.getTextureReference().setTextureWrap(wrapMode, wrapMode);

    curlTargetXZ.allocate(w, h, GL_RGBA32F_ARB);
    curlTargetXZ.getTextureReference().setTextureMinMagFilter(GL_LINEAR, GL_LINEAR);
    curlTargetXZ.getTextureReference().setTextureWrap(wrapMode, wrapMode);

    curlTargetYZ.allocate(w, h, GL_RGBA32F_ARB);
    curlTargetYZ.getTextureReference().setTextureMinMagFilter(GL_LINEAR, GL_LINEAR);
    curlTargetYZ.getTextureReference().setTextureWrap(wrapMode, wrapMode);

    ofSetLogLevel(ll);
    
    setupTranformFeedback();
    drawGui = true;
}

//--------------------------------------------------------------
void testApp::update(){

    setupNoise();
    
    step += dt;
    
    t.begin("update");
    advect();
    t.end();
    update_time = t.getval();
}

//--------------------------------------------------------------
void testApp::draw(){
    
    cam.setFov(fov);
    ofClear(0, 255);
    ofSetColor(255);
    
    cam.begin();
    ofTranslate(-width * 0.5, -height * 0.5);
    
    ofPushStyle();
    glEnable(GL_BLEND);
    
    ofSetColor(254, colorAlpha);
    glPointSize(pointSize);
    
    /* draw the particles */
    glBindVertexArray(vaoVec[curvao]);
    glDrawArrays(GL_POINTS, 0, partcnt);
    glBindVertexArray(0);
    
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    
    ofPopStyle();
    cam.end();
    
    if (drawGui) {
        ofEnableAlphaBlending();
        ctlGui.draw();

        if (noiseDebugView) {
            float dispw, disph;
            disph = dispw = 200.0;
//            disph = dispw = height/5.0f;
            ofPushMatrix();
            
            ofTranslate(ofGetWidth() - dispw * 3, 0);
            noiseImageXY.draw(0, 0, dispw, disph);
            ofTranslate(dispw, 0);
            curlTargetXY.draw(0, 0, dispw, disph);
            ofTranslate(dispw, 0);
            curlDrawXY.draw(0, 0, dispw, disph);
            
            ofTranslate(-dispw * 2.0, disph);
            noiseImageXZ.draw(0, 0, dispw, disph);
            ofTranslate(dispw, 0);
            curlTargetXZ.draw(0, 0, dispw, disph);
            ofTranslate(dispw, 0);
            curlDrawXZ.draw(0, 0, dispw, disph);
            
            ofTranslate(-dispw * 2.0, disph);
            noiseImageYZ.draw(0, 0, dispw, disph);
            ofTranslate(dispw, 0);
            curlTargetYZ.draw(0, 0, dispw, disph);
            ofTranslate(dispw, 0);
            curlDrawYZ.draw(0, 0, dispw, disph);
            
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
    cout << "X/Y" << endl;
    renderCurl(curlTargetXY, noiseImageXY, curlDrawXY);
    cout << "X/Z" << endl;
    renderCurl(curlTargetXZ, noiseImageXZ, curlDrawXZ);
    cout << "Y/Z" << endl;
    renderCurl(curlTargetYZ, noiseImageYZ, curlDrawYZ);
}

void testApp::renderCurl(ofFbo &fbo, ofFloatImage &data, ofFloatImage &draw)
{
    fbo.begin();
    ofClear(0, 255);
    ofSetColor(255, 255);
    curl.begin();
    curl.setUniformTexture("uPtex", data.getTextureReference(), 0);
    curl.setUniform2f("dims", data.getWidth(), data.getWidth());
    curl.setUniform1f("gradScale", gradScale);
    ofRect(0, 0, fbo.getWidth(), fbo.getHeight());
    curl.end();
    
    fbo.end();

    draw.allocate(fbo.getWidth(), fbo.getHeight(), OF_IMAGE_COLOR_ALPHA);
    ofLogLevel ll = ofGetLogLevel();
    ofSetLogLevel(OF_LOG_FATAL_ERROR);
    fbo.getTextureReference().readToPixels(draw.getPixelsRef());
    ofSetLogLevel(ll);
    draw.update();
    
    imgMinMax(draw);
    
    noiseGen.normalizeImage(draw);
}

void testApp::advect()
{
    advectShader.begin();

    advectShader.setUniformTexture("uXY", curlTargetXY.getTextureReference(), 0);
    advectShader.setUniformTexture("uXZ", curlTargetXZ.getTextureReference(), 1);
    advectShader.setUniformTexture("uYZ", curlTargetYZ.getTextureReference(), 2);
    advectShader.setUniform1f("Time", step);
    advectShader.setUniform1f("curlamt", curlAmt);
    advectShader.setUniform3f("constVel", constXval, constYval, constZval);
    advectShader.setUniform1f("advectDt", advectDt);
    advectShader.setUniform3f("dims", boundX, boundY, boundZ);
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
    for (int i = 0; i < partcnt; i++) {
        particles[i].x = ofRandom(0, boundX);
        particles[i].y = ofRandom(0, boundY);
        particles[i].z = ofRandom(-boundZ, 0);
    }
    
    advectShader.setupShaderFromFile(GL_VERTEX_SHADER, "advect.vert");
    advectShader.bindDefaults();
    advectProgram = advectShader.getProgram();

    const char* varyings[1] = { "vPosition"};
    glTransformFeedbackVaryings(advectProgram, 1, varyings, GL_INTERLEAVED_ATTRIBS);

    advectShader.linkProgram();
    
    SlotPosition =  advectShader.getAttributeLocation("Position");
    

    glGenBuffers(1, &ParticleBufferA);
    glBindBuffer(GL_ARRAY_BUFFER, ParticleBufferA);
    glBufferData(GL_ARRAY_BUFFER, partcnt * sizeof(ofVec3f), &particles[0], GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &ParticleBufferB);
    glBindBuffer(GL_ARRAY_BUFFER, ParticleBufferB);
    glBufferData(GL_ARRAY_BUFFER, partcnt * sizeof(ofVec3f), 0, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    //setup VAOs
    glGenVertexArrays(2, vaoVec);
    
    glBindVertexArray(vaoVec[0]);
    
    glBindBuffer(GL_ARRAY_BUFFER, ParticleBufferA);
    glEnableVertexAttribArray(SlotPosition);
    glVertexAttribPointer(SlotPosition, 3, GL_FLOAT, GL_FALSE, sizeof(ofVec3f), 0);
    
    glBindVertexArray(vaoVec[1]);
    
    glBindBuffer(GL_ARRAY_BUFFER, ParticleBufferB);
    glEnableVertexAttribArray(SlotPosition);
    glVertexAttribPointer(SlotPosition, 3, GL_FLOAT, GL_FALSE, sizeof(ofVec3f), 0);
    
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
    ctlGui.add(noiseDebugView.setup("noise images", false));
    
    ctlGui.add(constXval.setup("x amt", 0.0, -2.0, 2.0));
    ctlGui.add(constYval.setup("y amt", 0.0, -10.0, 10.0));
    ctlGui.add(constZval.setup("z amt", 0.0, -2.0, 2.0));
    ctlGui.add(gradScale.setup("grad scale", 1.0, 0.5, 2.0));
    ctlGui.add(curlAmt.setup("curl amt", 0.25, 0.0, 5.0));
    ctlGui.add(advectDt.setup("advect dt", 1.0, 0.0, 2.0));
    
    ctlGui.add(colorAlpha.setup("col alpha", 255, 0, 255));
    ctlGui.add(pointSize.setup("pt size", 1.0, 1.0, 10.0));
    
    ctlGui.add(fov.setup("fov", 45.0, 20.0, 90.0));
    
    ctlGui.loadFromFile(ctlGuiFile);
 
    numOctaves.addListener(this, &testApp::noiseParamIntChanged);
    noiseAlpha.addListener(this, &testApp::noiseParamFloatChanged);
    noiseBeta.addListener(this, &testApp::noiseParamFloatChanged);
    noiseSigned.addListener(this, &testApp::noiseParamBoolChanged);
    gradScale.addListener(this, &testApp::noiseParamFloatChanged);
    
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
    noiseGen.renderToImage(noiseImageXY, NoisePlaneXY);
    noiseGen.renderToImage(noiseImageXZ, NoisePlaneXZ);
    noiseGen.renderToImage(noiseImageYZ, NoisePlaneYZ);

    genCurl();
}

void testApp::noiseParamBoolChanged(bool &v) { noiseDirty = true; }
void testApp::noiseParamFloatChanged(float &v) { noiseDirty = true; }
void testApp::noiseParamIntChanged(int &v) { noiseDirty = true; }

#pragma mark - oF events -

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    switch (key) {
        case 'g':
            drawGui = !drawGui;
            break;
        case 's': {
            stringstream ss;
            
            ss << ofGetTimestampString("%Y%m%d_%H%M%s") << ".png";
            ofSaveScreen(ss.str());
            cout << "saved to " << ss.str() << endl;
            break;
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