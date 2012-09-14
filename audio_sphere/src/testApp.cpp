#include "testApp.h"

/*
 * audio reactive sphere, by Pete Werner 2012
 *
 * in a nutshell it gets the spectral data from the playing song
 * bins the frequencies via a bark map, then uses an ofPath to 
 * generate a curve.
 *
 * that curve is passed to a pingpong fbo as a texture, and the fbo
 * uses a frag shader to update displacement coeffcients for every particle
 *
 * in draw() we pass the fbo texture to a different shader which calculates
 * the actual displacement and colour
 *
 * we use axis_loc to restrict applying the eq texture to a specific part of 
 * the y axis. 
 *
 * the sphere itself is the mesh from ofSphere() drawn as verticies (i.e. the
 * verts are the particles. The FBO texture is sized equal to the sphere resolution
 * so we should have 1 pixel for each vertex on the sphere. 
 *
 * use 'f' to see the fbo updating
 * use 'e' to see the eq/spectral curve
 * use 'r' to toggle auto rotation
 * use 'p' to play the song again
 * it uses easycam as well so you can drag the window to change pov
 */

ofTexture eq_tex; //the eq curve texture we pass to the fbo
ofPath eq_path; //we use this path to build the texture
ofVboMesh vm; //the mesh of the sphere
ofSoundPlayer player; //the sound player

//bark mapping
#define BARK_MAX 25
#define SPECTRAL_BANDS 512
#define FREQ_MAX 44100.0

float bins[BARK_MAX]; //where we sum up our spectral info
int barkmap[SPECTRAL_BANDS]; //if i is the index of spectrum band, barkmap[i] gives which bin it goes to.

//fft data
float * fftSmoothed;
int nBandsToGet = 256;
float fftmax;

int rad = 300; //sphere radius
int res = 300; //sphere resolution
int fbo_res; //fbo resolution, will be sphere resolution
float axis_loc; //which part of the axis to update

float posdecayrate = 0.995f;

bool drawFBO;
bool autoRotate;
bool drawEQ;

int bark(float f) {
    float b = 13 * atan(0.00076 * f) + 3.5 * atan(pow(f / 7500.0f, 2));
    return ((int)floor(b));
}

float ang = 90.0f; //start offset for xaxis
float angincr;

//--------------------------------------------------------------
void testApp::setup(){
    
    ofBackground(0);
    ofSetFrameRate(30);
    ofEnableAlphaBlending();
    //    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //    glEnable(GL_DEPTH_TEST);
    glPointSize(1.0);
    drawFBO = false;
    autoRotate = true;
    drawEQ = false;
    
    //not really needed
    //    glEnable(GL_ALPHA_TEST);
    //    glAlphaFunc(GL_GREATER, 0.10f);
    
    //generate the mesh points
    buildSphereMesh(rad, res, vm);
    cout << "nverts: " << vm.getNumVertices() << endl;
    cout << "arb: " << ofGetUsingArbTex() << ", norm: " << ofGetUsingNormalizedTexCoords() << endl;
    
    //load the texture shader
    shader.load("tex.vert", "tex.frag");
    
    //fft init
    fftSmoothed = new float[8192];
    memset(fftSmoothed, 0x00, sizeof(float) * 8192);
    
    //map the frequencies to bark bands
    float freq_spc = FREQ_MAX / (float)SPECTRAL_BANDS;
    
	for (int i = 0; i < SPECTRAL_BANDS; i++) {
        int bidx = bark(i * freq_spc);
        barkmap[i] = bidx;
    }
    
    //load the position updating frag shader
    pos_shader.load("", "position.frag");
    
    //for the sphere we set this to the resolution which = #of verts along each axis
    fbo_res = res;
    
    //init the fbo's with blank data
    vector<ofVec3f> fbo_init_data;
    fbo_init_data.assign(fbo_res * fbo_res, ofVec3f(0.0, 0.0, 0.0));
    
    posbuf.allocate(fbo_res, fbo_res, GL_RGB32F);
    posbuf.src->getTextureReference().loadData((float *)&fbo_init_data[0], fbo_res, fbo_res, GL_RGB);
    posbuf.dst->getTextureReference().loadData((float *)&fbo_init_data[0], fbo_res, fbo_res, GL_RGB);
    
    //reuse fbo_init_data for no real reason, it just needs to be blank
    eq_tex.allocate(fbo_res, 1, GL_RGB32F_ARB);
    eq_tex.loadData((float *)&fbo_init_data[0], fbo_res, 1, GL_RGB);
    
    axis_loc = fbo_res;
    angincr = 180.0/(float)fbo_res;

    player.loadSound("jhfd.mp3");
    
    player.play(); //go
}

//--------------------------------------------------------------
void testApp::update(){
    
    /* get the spectral data */
    float * val = ofSoundGetSpectrum(nBandsToGet);
	for (int i = 0; i < nBandsToGet; i++){
		
		// let the smoothed calue sink to zero:
		fftSmoothed[i] *= 0.96f;
		
		// take the max, either the smoothed or the incoming:
		if (fftSmoothed[i] < val[i])
            fftSmoothed[i] = val[i];
        
        if (fftSmoothed[i] > fftmax) {
            fftmax = fftSmoothed[i];
        }
	}
    
    /* update our bark map'd frequency bins */
    memset(bins, 0x00, sizeof(bins));
    for (int i = 0; i < SPECTRAL_BANDS; i++) {
        int idx = barkmap[i];
        bins[idx] += fftSmoothed[i] * 20;
    }
    
    /* put the eq vals into a path to turn them into a curve */
    int line_len = fbo_res;
    float ppseg = line_len /(float)(BARK_MAX+1);
    eq_path.clear();
    eq_path.curveTo(0, -bins[0]);
    for (int i = 0; i < BARK_MAX; i++) {
        //        bins[i] = max(5.0f, bins[i]); //set a lower bound on the bin val
        eq_path.curveTo(i * ppseg, -bins[i]);
    }
    
    //    eq_path.curveTo(BARK_MAX * ppseg, -bins[0]);
    //    eq_path.curveTo(BARK_MAX * ppseg, -bins[0]);
    
    //smooth this out a little at the end so the eq texture wraps, 25 = BARK_MAX
    eq_path.curveTo(25 * ppseg, -(bins[0] + bins[24] + bins[23] + bins[22])/4.0f);
    eq_path.curveTo(26 * ppseg, -bins[0]);
    eq_path.curveTo(26 * ppseg, -bins[0]);
    
    ofMesh eq_m = eq_path.getTessellation();
    
    //load up the eq curve into a texture
    eq_tex.loadData((float *)eq_m.getVerticesPointer(), fbo_res, 1, GL_RGB);
    
    //update where on the axis we will apply the latest eq data
    axis_loc--;
    
    if (axis_loc < 0)
        axis_loc = fbo_res;
    
    //use fbo to work out displacement coeffcients
    posbuf.dst->begin();
    ofClear(0);
    pos_shader.begin();
    pos_shader.setUniformTexture("u_prevDisp", posbuf.src->getTextureReference(), 0);
    pos_shader.setUniformTexture("u_newDisp", eq_tex, 1); //pass the new displacement data
    pos_shader.setUniform1f("u_axisloc", axis_loc);
    pos_shader.setUniform1f("u_decayRate", posdecayrate);
    
    ofSetColor(255, 255, 255, 255);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
    glTexCoord2f(fbo_res, 0); glVertex3f(fbo_res, 0, 0);
    glTexCoord2f(fbo_res, fbo_res); glVertex3f(fbo_res, fbo_res, 0);
    glTexCoord2f(0, fbo_res); glVertex3f(0, fbo_res, 0);
    glEnd();
    
    pos_shader.end();
    posbuf.dst->end();
    posbuf.swap();
    
}

//--------------------------------------------------------------
void testApp::draw(){
    
    //curve eq
    if (drawEQ) {
        ofPushMatrix();
        ofTranslate(600, ofGetHeight() - 50);
        ofMesh eq_mesh = eq_path.getTessellation();
        eq_mesh.drawVertices();
        ofPopMatrix();
    }
    
    /* see what the fbo has in it */
    if (drawFBO) {
        ofPushMatrix();
        ofTranslate(50, 50);
        ofDrawBitmapString("FBO", 0, 0);
        ofPopMatrix();
        
        ofPushMatrix();
        ofTranslate(50, 70);
        posbuf.src->getTextureReference().bind();
        
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
        glTexCoord2f(fbo_res, 0); glVertex3f(fbo_res, 0, 0);
        glTexCoord2f(fbo_res, fbo_res); glVertex3f(fbo_res, fbo_res, 0);
        glTexCoord2f(0, fbo_res); glVertex3f(0, fbo_res, 0);
        glEnd();
        
        posbuf.src->getTextureReference().unbind();
        ofPopMatrix();
    }
    
    //actual sphere
    
    ofPushMatrix();
    ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
    
    cam.begin();
    
    if (autoRotate) {
        ofRotateY(-30);
        ofRotateX(ang += angincr);
        ofRotateZ(ofGetFrameNum() * 0.005);
    } else {
        ofRotateX(0.0);
        ofRotateY(0.0);
        ofRotateZ(0.0);
    }
    
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    
    shader.begin();
    shader.setUniformTexture("u_OffMap", posbuf.src->getTextureReference(), 0);
    shader.setUniform1f("u_fboRes", (float)fbo_res);
    
    vm.drawVertices();
    
    shader.end();
    
    glDisable(GL_POINT_SMOOTH);
    
    cam.end();
    ofPopMatrix();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    cout << "fps: " << ofToString(ofGetFrameRate()) << endl;
    cout << "frame: " << ofToString(ofGetFrameNum()) << endl;
    
    if (key == 'f')
        drawFBO = !drawFBO;
    if (key == 'r')
        autoRotate = !autoRotate;
    if (key == 'p')
        player.play();
    if (key == 'e')
        drawEQ = !drawEQ;
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

//from ofSetSphereResolution
void testApp::buildSphereMesh(int radius, int res, ofMesh & sphereMesh) {
    
    
    int n = res * 2;
    float ndiv2=(float)n/2;
    
    /*
     Original code by Paul Bourke
     A more efficient contribution by Federico Dosil (below)
     Draw a point for zero radius spheres
     Use CCW facet ordering
     http://paulbourke.net/texture_colour/texturemap/
     */
    
    float theta2 = TWO_PI;
    float phi1 = -HALF_PI;
    float phi2 = HALF_PI;
    //    float r = 1.f; // normalize the verts
    float r = radius;
    
    sphereMesh.clear();
    sphereMesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
    
    int i, j;
    float theta1 = 0.f;
    float jdivn,j1divn,idivn,dosdivn,unodivn=1/(float)n,t1,t2,t3,cost1,cost2,cte1,cte3;
    cte3 = (theta2-theta1)/n;
    cte1 = (phi2-phi1)/ndiv2;
    dosdivn = 2*unodivn;
    ofVec3f e,p,e2,p2;
    
    if (n < 0){
        n = -n;
        ndiv2 = -ndiv2;
    }
    if (n < 4) {n = 4; ndiv2=(float)n/2;}
    if(r <= 0) r = -r;
    
    t2=phi1;
    cost2=cos(phi1);
    j1divn=0;
    
    ofVec3f vert, normal;
    ofVec2f tcoord;
    
    for (j=0;j<ndiv2;j++) {
        t1 = t2;
        t2 += cte1;
        t3 = theta1 - cte3;
        cost1 = cost2;
        cost2 = cos(t2);
        e.y = sin(t1);
        e2.y = sin(t2);
        p.y = r * e.y;
        p2.y = r * e2.y;
        
        idivn=0;
        jdivn=j1divn;
        j1divn+=dosdivn;
        for (i=0;i<=n;i++) {
            t3 += cte3;
            e.x = cost1 * cos(t3);
            e.z = cost1 * sin(t3);
            p.x = r * e.x;
            p.z = r * e.z;
            
            normal.set( e.x, e.y, e.z );
            tcoord.set( idivn, jdivn);
            vert.set( p.x, p.y, p.z );
            
            sphereMesh.addNormal(normal);
            sphereMesh.addTexCoord(tcoord);
            sphereMesh.addVertex(vert);
            
            e2.x = cost2 * cos(t3);
            e2.z = cost2 * sin(t3);
            p2.x = r * e2.x;
            p2.z = r * e2.z;
            
            normal.set(e2.x, e2.y, e2.z);
            tcoord.set(idivn, j1divn);
            vert.set(p2.x, p2.y, p2.z);
            
            sphereMesh.addNormal(normal);
            sphereMesh.addTexCoord(tcoord);
            sphereMesh.addVertex(vert);
            
            idivn += unodivn;
            
        }
    }
}
