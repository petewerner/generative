#include "testApp.h"


//press spacebar to toggle between fbos and the mesh with fbo texture
bool drawFbos = true;

ofEasyCam cam;

//--------------------------------------------------------------
void testApp::setup(){
        
    ofBackground(0);
	ofSetFrameRate(40);
	
    //fbo
	fbo1.allocate(ecran, ecran, GL_RGB);
    //we make fbo2 the same size as main fbo, and we will scale it when we draw it
    fbo2.allocate(ecran, ecran*2, GL_RGB);
    Mainfbo.allocate(ecran, ecran*2, GL_RGB);
    
    //the mesh defines a shape, in this case a rectangle
    //we need to use two triangles make the rectangle
    
    //the vertex coordinates are the coordinates that make the
    //size of the shape we want, in this case ofGetWidth()/2.0 and ofGetHeight()/2.0
    
    //the texture coordinates are the size of the texture we draw on to the mesh
    //so in this case it is the same size as Mainfbo
    
    //each vertex needs a texture coordinate
    
    //first triangle in the mesh
    mesh1.addTexCoord(ofVec2f(0,0));
    mesh1.addVertex(ofVec3f(0,0,0));
    
    mesh1.addTexCoord(ofVec2f(Mainfbo.getWidth(), 0));
    mesh1.addVertex(ofVec3f(ofGetWidth()/2.0, 0, 0));
    
    mesh1.addTexCoord(ofVec2f(0, Mainfbo.getHeight()));
    mesh1.addVertex(ofVec3f(0, ofGetHeight()/2.0, 0));

    //second triangle. together they make something the same size/shape as Mainfbo
    mesh1.addTexCoord(ofVec2f(Mainfbo.getWidth(), 0));
    mesh1.addVertex(ofVec3f(ofGetWidth()/2.0, 0, 0));
    
    mesh1.addTexCoord(ofVec2f(0, Mainfbo.getHeight()));
    mesh1.addVertex(ofVec3f(0, ofGetHeight()/2.0, 0));
    
    mesh1.addTexCoord(ofVec2f(Mainfbo.getWidth(), Mainfbo.getHeight()));
    mesh1.addVertex(ofVec3f(ofGetWidth()/2.0,ofGetHeight()/2.0, 0));

}

//--------------------------------------------------------------
void testApp::update(){
        
	//Dessin des fbo
    
    fbo1.begin();
    drawFbo();
    fbo1.end();
    
    fbo2.begin();
    drawFbo();
    fbo2.end();
    
    Mainfbo.begin();
    drawFbo();
    Mainfbo.end();
}
//-------------------------- FBO 1 -----------------------------
void testApp::drawFbo(){
    
    //Žcran clair
	ofClear(0,0,0, 0);
    
    ofPushMatrix();
    customDraw3d();
    ofPopMatrix();
    
}


//--------------------- ENVIRRONEMENT 3D -----------------------
void testApp::customDraw3d(){
    
    ofBox(30);

    ofSetColor(255,100,0);
    //do the same thing from the first example...
    ofMesh mesh;
    mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
    for(int i = 1; i < points.size(); i++){
        
        //find this point and the next point
        ofVec3f thisPoint = points[i-1];
        ofVec3f nextPoint = points[i];
        
        //get the direction from one to the next.
        //the ribbon should fan out from this direction
        ofVec3f direction = (nextPoint - thisPoint);
        
        //get the distance from one point to the next
        float distance = direction.length();
        
        //get the normalized direction. normalized vectors always have a length of one
        //and are really useful for representing directions as opposed to something with length
        ofVec3f unitDirection = direction.normalized();
        
        //find both directions to the left and to the right
        ofVec3f toTheLeft = unitDirection.getRotated(-90, ofVec3f(0,0,1));
        ofVec3f toTheRight = unitDirection.getRotated(90, ofVec3f(0,0,1));
        
        //use the map function to determine the distance.
        //the longer the distance, the narrower the line.
        //this makes it look a bit like brush strokes
        float thickness = ofMap(distance, 0, 30, 10, 1, true);
        
        //calculate the points to the left and to the right
        //by extending the current point in the direction of left/right by the length
        ofVec3f leftPoint = thisPoint+toTheLeft*thickness;
        ofVec3f rightPoint = thisPoint+toTheRight*thickness;
        
        //add these points to the triangle strip
        mesh.addVertex(ofVec3f(leftPoint.x, leftPoint.y, leftPoint.z));
        mesh.addVertex(ofVec3f(rightPoint.x, rightPoint.y, rightPoint.z));
    }
    
    //end the shape
    mesh.draw();

}


//--------------------------------------------------------------
void testApp::draw(){
    
   
    //draw the fbos directly
    if (drawFbos) {
        //draw fbo1 contents in the top left
        ofPushMatrix();
        ofTranslate(0, 0);
        fbo1.draw(0, 0);
        ofPopMatrix();

        //draw fbo2 contents in the underneath it
        ofPushMatrix();
        ofTranslate(0, ofGetHeight()/2.0);
        //here we pass the size arguments as well, so oF will scale it to fit
        fbo2.draw(0, 0, ofGetWidth()/2.0, ofGetHeight()/2.0);
        ofPopMatrix();

        //draw main fbo
        ofPushMatrix();
        ofTranslate(ofGetWidth()/2.0, 0);
        Mainfbo.draw(0,0);
        ofPopMatrix();
        
        //Pointeur souris
        ofFill();
        ofSetColor(255, 0, 0);
        ofSphere(ofGetMouseX(), ofGetMouseY(), 5);
        
        //draw some borders
        ofSetColor(255);
        ofSetLineWidth(3);
        ofLine(ofGetWidth()/2.0, 0, ofGetWidth()/2.0, ofGetHeight());
        ofLine(0, ofGetHeight()/2.0, ofGetWidth()/2.0, ofGetHeight()/2.0);
    } else {
        
        //or you can use getTextureReference() and bind the fbo contents
        //to use as a texture.
        //you will need texture coordinates set in your mesh for this to work

        ofClear(30);
        cam.begin();
        Mainfbo.getTextureReference().bind();
        mesh1.draw();
        Mainfbo.getTextureReference().unbind();
        cam.end();

    }
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    
    if (key == ' ')
        drawFbos = !drawFbos;
    
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){    

    
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

    ofVec3f mousePoint(x,y,0);
    points.push_back(mousePoint);
    
    
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