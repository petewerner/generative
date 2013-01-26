#include "testApp.h"

/*
 * TODO:
 * 
 * [ longer term/misc ideas ]
 * - can we set the vel as a vertex attribute rather than a tex might be faster
 *
 * [ shorter term ]:
 * - need to seperate the width from the height being the same
 * - a few places assume with res of the attractor is 700.
 * - look at the fastest way to draw a bunch of particles, should we put them in a vbo and update that
 *      or is using textures like we do the best way?
 *
 * [ done ]
 * - update old_dja to do ofMap the density val so it lies between 0.1 and 1.0
 * - work out what is causing that pulsing effect, its pos.z, all parts had the same val
 * - calculate the velocity map somewhere, it is fixed per attractor density so does nto need
 *  to be redone every update(), only when attractor is redrawn
 * - then hook the position fbo to update based on the vel map
 * - see if we can make the vertex vbo static instead of sending it every update??
 * - the parts get "stuck" or bunch up. would be cool if we could work out of part A has the same loc as part
 *      B and if so move part B to a new random loc. <- unsolved but using random vels works nicely so moot.
 * - or just add some lifetime val to the part2
 * - update old_dja to do the attractor calcs via a shader, should export a greyscale texture
 * - define a subset of the attractor input space, and use a zoomed in portion
 *  that way there are less points to caclulate, and it will always have interesting stuff going on
 * - we do lots of iterations to build the attractor, mainly so it looks bright enough.
 *      should stop this so it is less computationally intensive.
 * - also a gaussian blur shader 
 * - try do a log image, see what the max is when x = y = 0.0, seems to be most dense
 *      even if we hardcode the max, given that its logs maybe its ok?
 * - should take a look @ colour as well.
 * - see if curl/grad can help, how?
 * - add contrast and brightness shaders. we can possibly get away with less iterations for the attractor
 * - look at alternate filters for the dja attractor. good practise for the post i plan todo anyway.
 * - one old_dja is using a shader and its size + window width/height is all seperate, should play with the
 *      res of old_dja to see if we can get away with a smaller one and maybe averaging it or smth (bilinear!)
 *      we can get away w/this but its not really a material cause of slowness so meh.
 *
 * [ Version log ]
 * v2: moved attractor to its own class
 * v3: started on shaders
 * v4: cleanup, moved parts verts to vbo
 * v5: try different particle effect
 * v6: bring in new dja, cleanup.
 * v7: mess around with dja shader
 * v8: debug random vel
 * v9: bring in log scaled dja and curl shader, plus simple timer;
 * v10: color ***argh
 * v11: do color in dja, looking p nice!
 * v12: make sizes independent of one another
 * v13: misc size fixes, add letterboxing
 * v14: add in kinect
 */

float part_life = 900.0;

bool fps = true;
bool debugFboOn = false;
bool pointSmoothing = true;
bool drawBlur = false;
bool drawParts = true;
bool drawOverlay = false;
bool drawTimimgs = true;
bool letterbox = true;

float speed = 0.5f;
float zoom = 0.4f;//0.5f;
float blur_scale = 0.04;
int width_offset;
float curl_scale = 0.5;//1.0;
float blob_eps = 5.0;

float a,b,c,d;

simpleTimer djaTimer;
simpleTimer curlTimer;
simpleTimer velTimer;
simpleTimer posTimer;
simpleTimer colorTimer;
simpleTimer partsTimer;
simpleTimer renderTimer;
simpleTimer updateTimer;

ofTexture randtex;

//particle stuff
#define MAX_PARTICLES	490000
float	pos[MAX_PARTICLES][3];
int	numParticles = MAX_PARTICLES;

float pointSize = 1.0;
int mode = 1;

simpleTimer timer;

#define MAX_BLOBS 4
ofPoint prev_points[MAX_BLOBS];

void testApp::setparm(float x, float y)
{
    //expects x & y to be 0 - 1
    a = PI * x;
    b = PI * y;
    c = -a;
    d = -b;
    dja.setParam(a, b, c, d);
    needDraw = true;
}

void testApp::randmap_rebuild(float rmin, float rmax)
{
    float * parts_buf = new float[numParticles * 4];
    
    for (int x = 0; x < parts_res; x++) {
        for (int y = 0; y < parts_res; y++) {
            int i = x * parts_res + y;
            parts_buf[i * 3 + 0] = ofRandom(rmin, rmax);
            parts_buf[i * 3 + 1] = ofRandom(rmin, rmax);
            parts_buf[i * 3 + 2] = ofRandom(rmin, rmax);
            parts_buf[i * 3 + 3] = ofRandom(rmin, rmax);
        }
    }
    randtex.loadData(parts_buf, parts_res, parts_res, GL_RGBA);

    delete parts_buf;
}

#ifdef USE_KINECT
ofVec2f
testApp::blob_to_data(ofxCvBlob &blob, int which)
{
    ofPoint cent = blob.centroid;
    float in_x, in_y;
    float tgt_x, tgt_y;
    in_x = cent.x;
    in_y = kinect.getHeight() - blob.boundingRect.getMinY();
    
    //not enough change
    if (abs(in_x - prev_points[which].x) < blob_eps && abs(in_y - prev_points[which].y) < blob_eps) {
        tgt_x = prev_points[which].x;
        tgt_y = prev_points[which].y;
        return(ofVec2f(0.0, 0.0));
    }
    
    tgt_x = ofLerp(prev_points[which].x, in_x, 0.8);
    tgt_y = ofLerp(prev_points[which].y, in_y, 0.8);
    prev_points[which].x = tgt_x;
    prev_points[which].y = tgt_y;
    
    tgt_x /= (float)kinect.getWidth();
    tgt_y /= (float)kinect.getHeight();
    
    return(ofVec2f(tgt_x, tgt_y));
}
#endif

//--------------------------------------------------------------
void testApp::setup(){
//    ofSetFrameRate(60);
    ofBackground(0, 0, 0);
    glDisable(GL_DEPTH_TEST);

    float w = 720; //ofGetWidth();
    float h = 720; //ofGetHeight();
    
    
//    renderFbo.allocate(ofGetWidth() * 0.75, ofGetHeight() * 0.75, GL_RGBA32F);
    renderFbo.allocate(w, h, GL_RGBA32F);
    renderFbo.getTextureReference().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
//    renderFbo.getTextureReference().setTextureWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
    
    parts_res = (int)sqrt((float)numParticles);
    numParticles = parts_res * parts_res;
    float * parts_buf = new float[numParticles * 4];
    
    for (int x = 0; x < parts_res; x++) {
        for (int y = 0; y < parts_res; y++) {
            int i = x * parts_res + y;
            parts_buf[i * 4 + 0] = ofRandom(0.0, w-1);
            parts_buf[i * 4 + 1] = ofRandom(0.0, h-1);
            parts_buf[i * 4 + 2] = ofRandom(0.0, part_life);
            parts_buf[i * 4 + 3] = 0.0;
            /* pos is now an index into the postion map fbo */
            pos[i][0] = x;
            pos[i][1] = y;
            pos[i][2] = 0;
        }
    }
    parts_vbo.setVertexData((ofVec3f *)&pos[0], numParticles, GL_STATIC_DRAW);
    
    randtex.allocate(parts_res, parts_res, GL_RGBA32F);
    randtex.setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
    randtex.loadData(parts_buf, parts_res, parts_res, GL_RGBA);
    
    partsFbo.allocate(parts_res, parts_res, GL_RGBA32F);
    partsFbo.src->getTextureReference().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
    partsFbo.dst->getTextureReference().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
    partsFbo.src->getTextureReference().loadData(parts_buf, parts_res, parts_res, GL_RGBA);
    partsFbo.dst->getTextureReference().loadData(parts_buf, parts_res, parts_res, GL_RGBA);
//    partsFbo.src->getTextureReference().setTextureWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
//    partsFbo.dst->getTextureReference().setTextureWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
    
    delete[] parts_buf;
    
    colorFbo.allocate(parts_res, parts_res, GL_RGBA32F);
    colorFbo.src->getTextureReference().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
    colorFbo.dst->getTextureReference().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
//    colorFbo.src->getTextureReference().setTextureWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
//    colorFbo.dst->getTextureReference().setTextureWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
    
    colorShader.load("", "color.frag");
    
    velFbo.allocate(parts_res, parts_res, GL_RGBA32F);
    velFbo.src->getTextureReference().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
    velFbo.dst->getTextureReference().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
//    velFbo.src->getTextureReference().setTextureWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
//    velFbo.dst->getTextureReference().setTextureWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
    velshader.load("", "velocity.frag");
    
    partshader.load("particle.vert", "");
    posshader.load("", "position.frag");
    
    debugShader.load("", "debug.frag");
    debugFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA32F);
    

    
    w = h = renderFbo.getHeight();//720;//ofGetHeight();
    
    dja.setup(w, h, 200);
    dja.logmaxd = 5.0;
    dja.setZoom(zoom);
    dja.setParam(0.5 * PI, 0.5 * PI, -0.5 * PI, -0.5 * PI);
    dja.update();
    randmap_rebuild(-1.0, 1.0);
    
    needDraw = true;
    
    blur = new ofxBlur();
//    blur->setup(ofGetWidth(), ofGetHeight());
    blur->setup(renderFbo.getWidth(), renderFbo.getHeight());
    blur->setScale(blur_scale);
    
//    printf("setup: blur dim %.0f %.0f\n", blur.getTextureReference().getWidth(), blur.getTextureReference().getHeight());
    
    cf.setup(renderFbo.getWidth(), renderFbo.getHeight());
    cf.setScale(0.5);
    cf.build(dja.getTextureReference());
//    cf.setScale(curl_scale);

    //at some stage set inital vels cause atm it looks not so good when it starts
    velFbo.src->begin();
    ofClear(ofFloatColor(0.0, 0.0, -1.0, 1.0));
    velFbo.src->end();
    
    width_offset = (int)((renderFbo.getWidth() - dja.getWidth()) / 2.0);
    printf("width_off = %d\n", width_offset);
    
#ifdef USE_KINECT
    //kinect/cv stuff
    
	kinect.setRegistration(true);
    
	kinect.init();
	kinect.open();
    
	colorImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayThreshNear.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);
    
    nearThreshold = 255; //230;
	farThreshold = 78;//196;//70;
    
	angle = 0;
//	kinect.setCameraTiltAngle(angle);
    
    for (int i = 0; i < MAX_BLOBS; i++) {
        prev_points[i] = ofPoint(kinect.width/2.0, kinect.height/2.0);
    }
#endif
}



//--------------------------------------------------------------
void testApp::update(){

    int w = ofGetWidth();
	int h = ofGetHeight();
    
    updateTimer.reset();
    updateTimer.start();
    
    if (debugFboOn) {
        //begin debug stuff
        debugFbo.begin();
        ofClear(0);
        int w = parts_res;
        int h = parts_res;

        debugShader.begin();
    //    debugShader.setUniformTexture("tex1", partsFbo.dst->getTextureReference(), 0);
        debugShader.setUniformTexture("tex1", randtex, 0);
        debugShader.setUniformTexture("tex2", dja.getTextureReference(), 1);
        
        ofSetColor(255,255,255,255);
        glBegin(GL_QUADS);
        glTexCoord2d(0, 0); glVertex2d(0, 0);
        glTexCoord2d(w, 0); glVertex2d(w, 0);
        glTexCoord2d(w, h); glVertex2d(w, h);
        glTexCoord2d(0, h); glVertex2d(0, h);
        glEnd();
        debugShader.end();
        debugFbo.end();

        //end debug stuff
    }
    
#ifdef USE_KINECT
    
    kinect.update();
    
	if(kinect.isFrameNew()) {
		
		// load grayscale depth image from the kinect source
		grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		
		// we do two thresholds - one for the far plane and one for the near plane
		// we then do a cvAnd to get the pixels which are a union of the two thresholds
        grayThreshNear = grayImage;
        grayThreshFar = grayImage;
        grayThreshNear.threshold(nearThreshold, true);
        grayThreshFar.threshold(farThreshold);
        cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
		grayImage.flagImageChanged();
		
        //min size (arg 2)
        //how many blogs (arg 4)
		contourFinder.findContours(grayImage, 8000, (kinect.width*kinect.height)/2, 2, false);

        ofVec2f tmp0, tmp1;
        float pi_fact = PI;
        
        switch (contourFinder.nBlobs) {
            case 1:
                tmp0 = blob_to_data(contourFinder.blobs[0], 0);
                if (tmp0.x != 0 && tmp0.y != 0) {
                    a = tmp0.x * pi_fact;
                    b = tmp0.y * pi_fact;
                    c = -a;
                    d = -b;
                    dja.setParam(a, b, c, d);
                    needDraw = true;
                }
                break;
            case 2:
                tmp0 = blob_to_data(contourFinder.blobs[0], 0);
                tmp1 = blob_to_data(contourFinder.blobs[1], 1);
                
                if (tmp0.x != 0 && tmp0.y != 0) {
                    a = tmp0.x * pi_fact;
                    b = tmp0.y * pi_fact;
                    needDraw = true;
                }
//                else {
//                    a = 0.5 * PI;
//                    b = 0.5 * PI;
//                }
                

                if (tmp1.x != 0 && tmp1.y != 0) {
                    c = -tmp1.x * pi_fact;
                    d = -tmp1.y * pi_fact;
                    needDraw = true;
                }
                
                if (needDraw)
                    dja.setParam(a, b, c, d);

                break;
        }

    }
#endif
    
    if (needDraw) {
        dja.setZoom(zoom);
        djaTimer.reset();
        djaTimer.start();
        dja.update();
        djaTimer.stop();
//        printf("update: dja.update took %.03f\n", djaTimer.diff());

        curlTimer.reset();
        curlTimer.start();
        cf.build(dja.getTextureReference());
        curlTimer.stop();
//        printf("update: cf.build took %.03f\n", curlTimer.diff());

        
        //update velfbo based on the curl
        velTimer.reset();
        velTimer.start();
        velFbo.dst->begin();
//        ofClear(0);
        
        velshader.begin();
        velshader.setUniformTexture("partloc", partsFbo.src->getTextureReference(), 0);
        velshader.setUniformTexture("curltex", cf.getTextureReference(), 1);
        velshader.setUniformTexture("veltex", velFbo.src->getTextureReference(), 2);
        
        w = h = parts_res;
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(0, 0);
        glTexCoord2f(w, 0); glVertex2f(w, 0);
        glTexCoord2f(w, h); glVertex2f(w, h);
        glTexCoord2f(0, h); glVertex2f(0, h);
        glEnd();
        velshader.end();
        velFbo.dst->end();
        velFbo.swap();
        velTimer.stop();
        needDraw = false;
    }
    
//    w = ofGetWidth();
//    h = ofGetHeight();

    w = renderFbo.getWidth();
    h = renderFbo.getHeight();
    
    posTimer.reset();
    posTimer.start();
    //update particle locations based on the velocity map
    partsFbo.dst->begin();
    ofClear(0);
    posshader.begin();
    posshader.setUniformTexture("u_posloc", partsFbo.src->getTextureReference(), 0);
    posshader.setUniformTexture("u_randtex", randtex, 1);
    posshader.setUniformTexture("denstex", dja.getTextureReference(), 2);
    posshader.setUniformTexture("veltex", velFbo.src->getTextureReference(), 3);
    posshader.setUniform2f("u_screen", (float)w, (float)h);
    posshader.setUniform1f("u_part_life", part_life);
    posshader.setUniform1f("speed", speed);
    posshader.setUniform1f("widthOffset", (float)width_offset);
    
    ofSetColor(255,255,255,255);
    
    w = h = parts_res;
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(0, 0);
    glTexCoord2f(w, 0); glVertex2f(w, 0);
    glTexCoord2f(w, h); glVertex2f(w, h);
    glTexCoord2f(0, h); glVertex2f(0, h);
    glEnd();
    
    posshader.end();
    partsFbo.dst->end();
    partsFbo.swap();
    
    posTimer.stop();
//    printf("update: partloc %.3f\n", posTimer.diff());
    
    colorTimer.reset();
    colorTimer.start();
    
    //update colors
    colorFbo.dst->begin();
    ofClear(0);
    colorShader.begin();
    colorShader.setUniformTexture("partloc", partsFbo.src->getTextureReference(), 0);
    colorShader.setUniformTexture("colortex", dja.getTextureReference(), 1);
    colorShader.setUniformTexture("prevcol", colorFbo.src->getTextureReference(), 2);
    colorShader.setUniform2f("screen", parts_res, parts_res);
    colorShader.setUniform2f("tex", dja.getWidth(), dja.getHeight());
    w = h = parts_res;
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(0, 0);
    glTexCoord2f(w, 0); glVertex2f(w, 0);
    glTexCoord2f(w, h); glVertex2f(w, h);
    glTexCoord2f(0, h); glVertex2f(0, h);
    glEnd();
    
    colorShader.end();
    colorFbo.dst->end();
    colorFbo.swap();
    
    colorTimer.stop();
//    printf("update: colors %.3f\n", colorTimer.diff());
    
    renderTimer.reset();
    renderTimer.start();
    
    //turn on alpha blending.
    ofEnableAlphaBlending();
    ofPushMatrix();
    renderFbo.begin();
    ofClear(0, 0, 0, 0);
    
    if (drawOverlay) {
        //draw the background
        w = ofGetWidth();
        h = ofGetHeight();
        dja.getTextureReference().bind();
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(0, 0);
        glTexCoord2f(w, 0); glVertex2f(w, 0);
        glTexCoord2f(w, h); glVertex2f(w, h);
        glTexCoord2f(0, h); glVertex2f(0, h);
        glEnd();
        dja.getTextureReference().unbind();
    }
    
    if (drawParts) {
        
        w = renderFbo.getWidth();
        h = renderFbo.getHeight();
        partsTimer.reset();
        partsTimer.start();
        glPointSize(pointSize);
        if (pointSmoothing)
            glEnable(GL_POINT_SMOOTH);
        partshader.begin();
        
        partshader.setUniformTexture("partloc", partsFbo.dst->getTextureReference(), 0);
        partshader.setUniformTexture("colortex", colorFbo.src->getTextureReference(), 1);
        //these 2 not used right now but i expect that will come
        partshader.setUniform2f("screensize", w, h);
        partshader.setUniform2f("colsz", dja.getWidth(), dja.getHeight());
        partshader.setUniform1f("widthOffset", (float)width_offset);
        //send the vertex data
        parts_vbo.draw(GL_POINTS, 0, numParticles);

        partshader.end();

        if (pointSmoothing)
            glDisable(GL_POINT_SMOOTH);
        
        partsTimer.stop();
    }
    
    renderFbo.end();
    ofPopMatrix();
    renderTimer.stop();
//    printf("update: render %.3f\n", renderTimer.diff());
    
    updateTimer.stop();
//    printf("update: full update time: %.3f\n", updateTimer.diff());
    
    ofDisableAlphaBlending();
}

ofPoint
pointScale(ofPoint p, float width, float height, int newWidth, int newHeight)
{
    ofPoint q;
    q.x = (p.x/width) * newWidth;
    q.y = (p.y/height) * newHeight;
    return(q);
}

//--------------------------------------------------------------
void testApp::draw(){

    switch(mode) {
        case 1: {
            ofPushMatrix();
            if (drawBlur)
                blur->begin();
            ofClear(0);
            float draw_x = 0, draw_y = 0;
            float draw_width = ofGetWidth();
            float draw_height = ofGetHeight();
            if (letterbox) {

                draw_width = renderFbo.getWidth();
                draw_height = max(renderFbo.getHeight(), (float)ofGetHeight());
                draw_width = draw_height;
                draw_x = (ofGetWidth() - draw_width) / 2.0;
                draw_y = 0;
            }
            renderFbo.draw(draw_x, draw_y, draw_width, draw_height);
//            renderFbo.draw(0, 0, ofGetWidth(), ofGetHeight());
            if (drawBlur) {
                blur->end();
                blur->draw();
            }

            ofPopMatrix();
            break;
        }
        case 2:
            ofPushMatrix();
            renderFbo.draw(0, 0, ofGetWidth()/2.0, ofGetHeight()/2.0);
            ofPopMatrix();
            
            ofPushMatrix();
            ofTranslate(ofGetWidth()/2.0, 0);
//            randtex.draw(0, 0, ofGetWidth()/2.0, ofGetHeight()/2.0);
            cf.draw(0, 0, ofGetWidth()/2.0, ofGetHeight()/2.0);
            ofPopMatrix();
            
            ofPushMatrix();
            ofTranslate(0, ofGetHeight()/2.0);
            dja.getTextureReference().draw(0, 0, ofGetWidth()/2.0, ofGetHeight()/2.0);
            ofPopMatrix();
            
            ofPushMatrix();
            ofTranslate(ofGetWidth()/2.0, ofGetHeight()/2.0);
            if (debugFboOn)
                debugFbo.draw(0, 0, ofGetWidth()/2.0, ofGetHeight()/2.0);
            else {
//                partsFbo.dst->draw(0, 0, ofGetWidth()/2.0, ofGetHeight()/2.0);
//                velFbo.dst->draw(0, 0, ofGetWidth()/2.0, ofGetHeight()/2.0);

//                colorFbo.dst->draw(0, 0, ofGetWidth()/2.0, ofGetHeight()/2.0);
#ifdef USE_KINECT
                float box_width = ofGetWidth() /2.0;
                float box_height = ofGetHeight() /2.0;
                ofSetColor(255, 0, 0);
                ofRect(0, 0, box_width, box_height);
                ofSetColor(0);
                ofRect(2, 2, box_width - 4, box_height - 4);
                ofSetColor(255, 255, 255);
                for (int i = 0; i < contourFinder.nBlobs; i++) {
                    ofSetColor(20, 20, 220);
                    ofRect(pointScale(contourFinder.blobs[i].centroid, contourFinder.getWidth(), contourFinder.getHeight(), box_width, box_height), 5, 5);
                    
                    ofSetColor(200, 20, 20);
                    for (int j = 0; j < contourFinder.blobs[i].nPts; j++) {
                        ofPoint p = contourFinder.blobs[i].pts.at(j);
                        p = pointScale(p, contourFinder.getWidth(), contourFinder.getHeight(), box_width, box_height);
                        ofRect(p, 2, 2);
                    }
                }
                
#endif                
            }

            ofPopMatrix();
            break;
        case 3:
#ifdef USE_KINECT
            kinect.drawDepth(10, 10, 400, 300);
            //		kinect.draw(420, 10, 400, 300);
            ofEnableAlphaBlending();
            dja.getTextureReference().draw(420, 10, 400, 300);
            ofDisableAlphaBlending();
            
            grayImage.draw(10, 320, 400, 300);
            contourFinder.draw(10, 320, 400, 300);
            
            ofPushMatrix();
            ofTranslate(420, 320);
            ofSetColor(255, 0, 0);
            ofRect(0, 0, 400, 300);
            ofSetColor(0);
            ofRect(2, 2, 396, 296);
            ofSetColor(255, 255, 255);
            for (int i = 0; i < contourFinder.nBlobs; i++) {
                ofSetColor(20, 20, 220);
                ofRect(pointScale(contourFinder.blobs[i].centroid, contourFinder.getWidth(), contourFinder.getHeight(), 400, 300), 5, 5);
                
                ofSetColor(200, 20, 20);
                for (int j = 0; j < contourFinder.blobs[i].nPts; j++) {
                    ofPoint p = contourFinder.blobs[i].pts.at(j);
                    p = pointScale(p, contourFinder.getWidth(), contourFinder.getHeight(), 400, 300);
                    ofRect(p, 2, 2);
                }
            }
            ofPopMatrix();
#endif
            break;

    }

    if (fps) {
        ofEnableAlphaBlending();
        ofSetColor(255);
        ofDrawBitmapString("FPS: " + ofToString((int)ofGetFrameRate()), 20,20);
        ofDisableAlphaBlending();        
    }

    if (drawTimimgs) {
        ofEnableAlphaBlending();
        int prec = 2;
        stringstream msg;
        
        msg << "dja: " << ofToString(djaTimer.diff(), prec) << "\n"
        << "curl: " << ofToString(curlTimer.diff(), prec) << "\n"
        << "vel: " << ofToString(velTimer.diff(), prec) << "\n"
        << "pos: " << ofToString(posTimer.diff(), prec) << "\n"
        << "color: " << ofToString(colorTimer.diff(), prec) << "\n"
        << "parts: " << ofToString(partsTimer.diff(), prec) << "\n"
        << "render: " << ofToString(renderTimer.diff(), prec) << "\n"
        << "update: " << ofToString(updateTimer.diff(), prec) << "\n"
        << "speed +/-: " << ofToString(speed, 1) << "\n"
        << "curl scale h/g: " << ofToString(curl_scale, 1) << "\n"
        << "zoom j/k: " << ofToString(zoom, 1) << "\n"
        << "dja blur q: " << (dja.isBlurOn() ? "on" : "off") << "\n"
        << "blur b: " << (drawBlur ? "on" : "off") << "\n"
        << "blur width: " << ofToString(blur_scale, 3) << "\n"
        << "letterbox l: " << (letterbox ? "on" : "off") << "\n"
#ifdef USE_KINECT
        << "blob eps y/u: " << ofToString(blob_eps, 1) << "\n"
        << "nearThresh <>: " << ofToString(nearThreshold) << "\n"
        << "farThresh 90: " << ofToString(farThreshold) << "\n"
#endif
        << endl;
        ofDrawBitmapString(msg.str(), 20, 33);
        ofDisableAlphaBlending();
    }
    
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

    if (key == ' ') {
        cout << "FPS: " << ofToString(ofGetFrameRate()) << endl;
    }
    if (key == 'p')
        pointSmoothing = !pointSmoothing;
    
    if (key == 'd')
        debugFboOn = !debugFboOn;
    if (key == 't')
        drawTimimgs = !drawTimimgs;
    if (key == 'h') {
        curl_scale += 0.5;
        cf.setScale(curl_scale);
        cf.build(dja.getTextureReference());
    }
    if (key == 'g') {
        curl_scale -= 0.5;
        cf.setScale(curl_scale);
        cf.build(dja.getTextureReference());
    }
    
    if (key == 'q')
        dja.blurToggle();
    
    if (key == 's') {
//        ofImage img;
        ofSaveScreen("butts1.png");
    }
    
    if (key == 'i')
        drawParts = !drawParts;
    if (key == 'o')
        drawOverlay = !drawOverlay;
    
    if (key == '1')
        mode = 1;
    if (key == '2')
        mode = 2;
    if (key == '3')
        mode = 3;
    if (key == 'f')
        fps = !fps;
    
    if (key == '+')
        speed += 0.5;
    if (key == '-')
        speed -= 0.5;
//        speed = max(0.5, speed - 0.5);
    
    if (key == 'j')
        zoom = max(0.1, zoom - 0.1), needDraw = true;
    if (key == 'k')
        zoom += 0.1, needDraw = true;
    
//    if (key == 't')
//        dja.thresh += 0.05;
//    if (key == 'r')
//        dja.thresh = max(0.0, dja.thresh - 0.05);
    if (key == 'b')
        drawBlur = !drawBlur;
    if (key == 'm') {
        blur_scale += 0.005;
        blur->setScale(blur_scale);
    }
    if (key == 'n') {
        blur_scale = max(0.0, blur_scale - 0.005);
        blur->setScale(blur_scale);
    }
    
    if (key == 'l') {
        letterbox = !letterbox;
    }
#ifdef USE_KINECT
    
    if (key == OF_KEY_UP) {
        angle++;
        if(angle>30) angle=30;
        kinect.setCameraTiltAngle(angle);
     
    }
    if (key == OF_KEY_DOWN) {
        angle--;
        if(angle<-30) angle=-30;
        kinect.setCameraTiltAngle(angle);
    }

    if (key == 'u') {
        blob_eps += 5.0;
    }
    if (key == 'y') {
        blob_eps = max(5.0, blob_eps - 5.0);
    }
    
    if (key == '<')
        nearThreshold = max(1, nearThreshold - 1);
    if (key == '>')
        nearThreshold = min(255, nearThreshold + 1);
    if (key == '9')
        farThreshold = max(1, farThreshold - 1);
    if (key == '0')
        farThreshold = min(255, farThreshold + 1);
#endif
    needDraw = true;
//    printf("thresh: %.2f\n", dja.thresh);
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
    
}

void testApp::exit()
{
#ifdef USE_KINECT
    kinect.close();
#endif
}
//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){
    setparm(x/(float)ofGetWidth(), y/(float)ofGetHeight());    
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
    setparm(x/(float)ofGetWidth(), y/(float)ofGetHeight());
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    printf("mouse %d %d, %.8f %.8f\n", x, y, x/(float)ofGetWidth(), y/(float)ofGetHeight());
    setparm(x/(float)ofGetWidth(), y/(float)ofGetHeight());
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

    
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
    if (blur->getTextureReference().getWidth() != ofGetWidth() || blur->getTextureReference().getHeight() != ofGetHeight()) {
        delete blur;
        blur = new ofxBlur();
        blur->setup(ofGetWidth(), ofGetHeight());
        blur->setScale(blur_scale);
    }

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){
    
}



