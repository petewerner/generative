#include "testApp.h"

/*
 * Cheap Tricks, Pete Werner 2013
 * Uses kinect if available or falls back to device camera
 * Use far threshold for camera thresholding & space to learn the background
 *
 *
 * Requires the following addons
 * ofxTriangle - https://github.com/obviousjim/ofxTriangle
 * ofxFFT - https://github.com/julapy/ofxFFT
 * 
 * Uses ofxKinect, ofxOpenCv, ofxXmlSettings & ofxGui which should be part of oF 0.8
 * can optionally use ofxSoundStream
 */

int drawmode = 0;
int num_modes = 2;
string guiPath;

//--------------------------------------------------------------
void testApp::setup(){

	ofSetFrameRate(30);
    
    gpuBlur = NULL;
    useKinect = false;
    kinectOpen = false;
    cameraOpen = false;
    learnBG = false;
    showGui = false;
    inputWidth = 640;
    inputHeight = 480;
    
    guiPath = "guisettings.xml";
    
//    if (ofxKinect::numAvailableDevices() >= 1) {
        cout << "using kinect" << endl;
        openKinect();
        useKinect = true;
//    } else {
//        cout << "using camera" << endl;
//        openCamera();
//        useKinect = false;
//    }
    
	colorImg.allocate(inputWidth, inputHeight);
	grayImage.allocate(inputWidth, inputHeight);
	grayImagePrev.allocate(inputWidth, inputHeight);
	grayThreshNear.allocate(inputWidth, inputHeight);
	grayThreshFar.allocate(inputWidth, inputHeight);
    grayBg.allocate(inputWidth, inputHeight);
    
	nearThreshold = 255;
	farThreshold = 45;
    
    setupGpuBlur(inputWidth, inputHeight);
    
#ifdef USE_OFX_SOUNDSTREAM
    xss.setDeviceId(0);
    xss.setup(0, 1, this, 44100, fftLive.getBufferSize(), 4);
#else
    xss.setup(this, 0, 1, 44100, fftLive.getBufferSize(), 4);
#endif

    fftLive.setMirrorData(audioMirror);
    
    lows = 0;
    
    nbins = (int)(fftLive.getBufferSize() * 0.5);
    rawFFT = new float[nbins];
    
    setupColorMesh();
    
    colors.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
    updateColors();
    drawmode = 0;
    
    gui.setup("Settings", guiPath, 20, 20);
    gui.add(audioThreshold.setup("audioThreshold", 0.5, 0.0, 1.0));
    gui.add(audioPeakDecay.setup("audioPeakDecay", 0.95, 0.0, 1.0));
    gui.add(audioMaxDecay.setup("audioMaxDecay", 1.0, 0.0, 1.0));
    gui.add(audioMirror.setup("audioMirror", false));
    gui.add(nearThreshold.setup("nearThreshold", 255, 0, 255));
    gui.add(farThreshold.setup("farThreshold", 45, 0, 255));
    gui.add(blobMin.setup("blobMin", 2000, 200, 20000));
    gui.add(mirrorInput.setup("mirrorInput", true));
    gui.add(drawFPS.setup("FPS", true));
    gui.add(useGPU.setup("GPUBlur", true));
    gui.add(triangleMax.setup("triangleMax", 15, 3, 60));
    gui.add(glowExtra.setup("glowExtra", 1.0, 0.0, 10.0));
    gui.add(lineWidth.setup("lineWidth", 10.0, 1.0, 20.0));
    gui.add(jitterSize.setup("jitterSize", 0.5, 0.0, 2.0));
    
    gui.loadFromFile(guiPath);
    
   
}

//--------------------------------------------------------------
void testApp::update(){
    
    fftLive.setThreshold(audioThreshold);
    fftLive.setPeakDecay(audioPeakDecay);
    fftLive.setMaxDecay(audioMaxDecay);
    fftLive.setMirrorData(audioMirror);
    fftLive.update();
    
    fftLive.getFftPeakData(rawFFT, nbins);
    
    bool newFrame = useKinect ? readKinect() : readCamera();
    
    if (newFrame) {
        diff.absDiff(grayImage, grayImagePrev);
        diffFloat = diff;
        diffFloat *= (1.0f + glowExtra * lows);
        
        if (!bufferFloat.bAllocated)
            bufferFloat = diffFloat;
        else {
            bufferFloat *= 0.85;
            bufferFloat += diffFloat;
        }
        
		contourFinder.findContours(grayImage, blobMin, (inputWidth*inputHeight)/2, 100, false);
    }

}

//--------------------------------------------------------------
void testApp::draw(){
    
    int w = ofGetWidth();
    int h = ofGetHeight();
    
    if (drawmode == 0) {
        
        fftLive.getFftPeakData(rawFFT, nbins);
        float curlow = (rawFFT[0] + rawFFT[1] + rawFFT[2]) / 3.0f;
        
        if (curlow > lows)
            lows = curlow;
        else
            lows *= 0.90;

        ofEnableAlphaBlending();
        
        if (useGPU) {
            
            gpuBlur->beginDrawScene();
            ofSetColor(255);
            
            w = gpuBlur->getWidth();
            h = gpuBlur->getHeight();
            
            colors.draw(0, 0, w, h);
            if (grayImagePrev.bAllocated) {
                bufferBlur = bufferFloat;
                bufferBlur.blur(11);
                ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
                bufferBlur.draw(0, 0, w, h);
            }
            
            gpuBlur->endDrawScene();
            gpuBlur->performBlur();
            
            w = ofGetWidth();
            h = ofGetHeight();
            
            ofEnableBlendMode(OF_BLENDMODE_ALPHA);
            gpuBlur->drawSceneFBO(0, 0, w, h);
            ofEnableBlendMode(OF_BLENDMODE_ADD);
            gpuBlur->drawBlurFbo(0, 0, w, h);
            
        } else {
            
            ofSetColor(255);
            if (grayImagePrev.bAllocated) {
                bufferBlur = bufferFloat;
                bufferBlur.blur(31);
                bufferBlur *= 5.0;
                ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
                bufferBlur.draw(0, 0, w, h);
                colors.draw(0, 0, w, h);
                ofEnableBlendMode(OF_BLENDMODE_ADD);
            }
        }
        
        ofVec2f scale = ofVec2f(w/(float)inputWidth, h/(float)inputHeight);
        
        for (int i = 0; i < contourFinder.nBlobs; i++) {
            
            ofxCvBlob b = contourFinder.blobs[i];
            triangle.clear();
            triangle.triangulate(b, max(3.0f, b.pts.size()/(float)triangleMax));
            
            ofMesh m;
            m.clear();
            m.setMode(OF_PRIMITIVE_TRIANGLES);

            for (int k = 0; k < triangle.nTriangles; k++){
                ofVec2f a = triangle.triangles[k].a * scale;
                ofVec2f b = triangle.triangles[k].b * scale;
                ofVec2f c = triangle.triangles[k].c * scale;
                
                ofPoint tr[3];
                tr[0] = a;
                tr[1] = b;
                tr[2] = c;
                
                //displacement amount as a function of triangle size
                float base = a.distance(b);
                float height = b.distance(c);
                float area = base * height * 0.5f;
                float lowsDisp = sqrt(area) * jitterSize;
                
                ofVec2f center = triangle.getTriangleCenter(tr);
                
                ofVec2f da, db, dc;
                da = (a - center).normalize();
                db = (center - b).normalize() * 0.5;
//                dc = (c - center).normalize();
                
                a = a + da * (lowsDisp * lows);
                b = b + db * (lowsDisp * lows);
//                c = c + dc * (lowsDisp * lows);
                
                m.addTexCoord(a);
                m.addVertex(a);
                m.addTexCoord(b);
                m.addVertex(b);
                m.addTexCoord(c);
                m.addVertex(c);
            }
            
            ofSetLineWidth(2.0f + lineWidth * lows);
            colors.getTextureReference().bind();
            m.drawWireframe();
            colors.getTextureReference().unbind();
        }
        ofDisableAlphaBlending();
        
    } else {
        
        ofBackground(40);
        
        ofEnableAlphaBlending();
        
        ofSetColor(255, 255, 255);
        if (useKinect) {
            
            kinect.drawDepth(10, 10, 400, 300);
            kinect.draw(420, 10, 400, 300);
            ofDrawBitmapString("Kinect Depth", 10, 10);
            ofDrawBitmapString("Kinect Input", 420, 10);
            
        } else {
            
//            vid.draw(10, 10, 400, 300);
//            if (diffFloat.bAllocated)
//                bufferFloat.draw(420, 10, 400, 300);
            
            grayImage.draw(420, 10, 400, 300);
            grayBg.draw(10, 10, 400, 300);
            ofDrawBitmapString("Background", 10, 10);
            ofDrawBitmapString("Thresh Img", 420, 10);
        }
        
        
//        contourFinder.draw(10, 320, 400, 300);
//        ofDrawBitmapString("CV Contours", 10, 320);
        grayImage.draw(10, 320, 400, 300);
        ofDrawBitmapString("Gray Img", 10, 320);
        
        
        ofPushMatrix();
        ofTranslate(420, 320);
        ofEnableAlphaBlending();
        fftLive.draw(0, 0);//, 400, 300);
        ofDrawBitmapString("FFT", 0, 0);
        
        ofDisableAlphaBlending();
        ofPopMatrix();
        
    }
    
    if (drawFPS) {
        ofEnableAlphaBlending();
        ofSetColor(255);
        stringstream msg;
        msg << "FPS: " << ofToString(ofGetFrameRate(), 0);
        ofDrawBitmapString(msg.str(), 20, ofGetHeight() - 20);
        ofDisableAlphaBlending();
    }
    
    if (showGui)
        gui.draw();
    

}

#pragma mark - Color Functions -

void testApp::updateColors()
{
    int n1 = ofGetFrameNum() % 255;
    int n2 = (n1 + 50) % 255;
    ofColor c1 = ofColor::fromHsb(n1, 200, 255);
    ofColor c2 = ofColor::fromHsb(n2, 200, 255);
    
    setMeshColors(c1, c2);
}

void testApp::setupColorMesh()
{
    int w = ofGetWidth();
    int h = ofGetHeight();
    
    colorMesh.clear();
    colorMesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
    colorMesh.addVertex(ofVec2f(0, 0));
    colorMesh.addVertex(ofVec2f(0, h));
    colorMesh.addVertex(ofVec2f(w, 0));
    colorMesh.addVertex(ofVec2f(w, h));
}

void testApp::setMeshColors(ofColor c1, ofColor c2)
{
    colorMesh.clearColors();
    colorMesh.addColor(c1);
    colorMesh.addColor(c1);
    colorMesh.addColor(c2);
    colorMesh.addColor(c2);
    
    colors.begin();
    ofClear(255, 255, 255, 255);
    colorMesh.draw();
    colors.end();
}


#pragma mark - Camera Functions -

void testApp::openCamera()
{
    vid.setVerbose(true);
    
    if (!vid.initGrabber(inputWidth, inputHeight) ){
        cout << "init vid grabber failed" << endl;
    }
    learnBG = true;
    cameraOpen = true;
}

void testApp::closeCamera()
{
    vid.close();
    cameraOpen = false;
}

bool testApp::readCamera()
{
    vid.update();
    if (vid.isFrameNew()) {
        updateColors();
        
        grayImagePrev = grayImage;
        
        colorImg.setFromPixels(vid.getPixels(), inputWidth, inputHeight);
        grayImage = colorImg;
        if (mirrorInput)
            grayImage.mirror(false, true);
        
        if (learnBG) {
            grayBg = grayImage;
            learnBG = false;
        }
        
        grayImage.absDiff(grayBg);
        grayImage.threshold(farThreshold);
		grayImage.flagImageChanged();
        
        return true;
    }
    return false;
}

#pragma mark - Kinect Functions -

bool testApp::openKinect()
{
    if (kinect.isConnected())
        kinect.close();
    
    kinect.setRegistration(true);
	kinect.init();
	if (!kinect.open()) {
        cout << "open kinect failed" << endl;
        kinectOpen = false;
        return false;
    }
    return true;
    
}

void testApp::closeKinect()
{
    if (kinect.isConnected()) {
        kinect.close();
        kinectOpen = false;
    }
}

bool testApp::readKinect()
{
	kinect.update();
	
	// there is a new frame and we are connected
	if(kinect.isFrameNew()) {
        updateColors();

        grayImagePrev = grayImage;
        
		// load grayscale depth image from the kinect source
		grayImage.setFromPixels(kinect.getDepthPixels(), inputWidth, inputHeight);
        if (mirrorInput)
            grayImage.mirror(false, true);
        
        grayThreshNear = grayImage;
        grayThreshFar = grayImage;
        grayThreshNear.threshold(nearThreshold, true);
        grayThreshFar.threshold(farThreshold);
        cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
        
		grayImage.flagImageChanged();
        
		return true;
	}
    return false;
}

#pragma mark - Util Functions -

testApp::~testApp() {
    xss.stop();
}


void testApp::setupGpuBlur(int width, int height)
{
    if (gpuBlur) {
        delete gpuBlur;
    }
    
    gpuBlur = new ofxFboBlur();
    
    ofFbo::Settings s;
	s.width = width;
	s.height = height;
	s.internalformat = GL_RGBA;
	s.textureTarget = GL_TEXTURE_RECTANGLE_ARB;
	s.maxFilter = GL_LINEAR; GL_NEAREST;
	s.numSamples = 4;
	s.numColorbuffers = 3;
	s.useDepth = false;
	s.useStencil = false;
    
	gpuBlur->setup(s, true);
	gpuBlur->setBackgroundColor(ofColor(0,0));
    
	gpuBlur->blurOffset = 2.14;
	gpuBlur->blurPasses = 5;
	gpuBlur->numBlurOverlays = 2;
	gpuBlur->blurOverlayGain = 200;
    
    printf("setup gpu blur w %d h %d\n", s.width, s.height);
}

void testApp::setScreenSize(string name)
{
    if (name == "1024x768") {
        ofSetWindowShape(1024, 768);
    } else if (name == "960x540") {
        ofSetWindowShape(960, 540);
    } else if (name == "1920x1080") {
        ofSetWindowShape(1920, 1080);
    } else if (name == "640x480") {
        ofSetWindowShape(640, 480);
    }
}


void testApp::exit() {

    if (useKinect && kinectOpen)
        kinect.close();
    if (!useKinect && cameraOpen)
        vid.close();
    
    gui.saveToFile(guiPath);
    
}

void testApp::audioReceived(float * input, int bufferSize, int nChannels) {
//    printf("audio in\n");
    fftLive.audioIn(input, bufferSize, nChannels);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	switch (key) {
        case 'm':
            drawmode = (drawmode + 1) % num_modes;
            break;
        case 'd':
            drawFPS = !drawFPS;
            break;
        case 'g':
            showGui = !showGui;
            break;
        case 'f':
            ofToggleFullscreen();
            break;
        case ' ':
            learnBG = true;
            break;
        case 's': {
            stringstream fn;
            fn << "screen_" << ofToString(ofGetElapsedTimef(), 0) << ".png";
            ofSaveScreen(fn.str());
            break;
		} case OF_KEY_UP:
			angle++;
			if(angle>30) angle=30;
			kinect.setCameraTiltAngle(angle);
			break;
			
		case OF_KEY_DOWN:
			angle--;
			if(angle<-30) angle=-30;
			kinect.setCameraTiltAngle(angle);
			break;
	}

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
