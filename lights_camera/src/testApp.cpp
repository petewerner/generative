#include "testApp.h"

/*
 * A basic opengl lighting example by Pete Werner 2013
 *
 * Should be useful to see what the various parameters do and how they interact
 *
 * Spot light defaults to a red light with a blue specular highlight
 * Point light is similar to a spot light, defaults to green light with white spec highlight
 * Directional light defaults to blue light with red specular light.
 * Abmient light defaults to weak grey.
 *
 * use the W slider on specular color to adjust black <-> white
 * use the 'X Source' toggle to draw the light source
 *
 * You can also adjust the material properties, shine and the various colours.
 * Toggles at the bottom of the control panel let you turn lights on/off, and also face culling
 *
 * It uses ofEasyCam, left click + drag to move around
 * right click + drag to zoom
 *
 * This app depends on ofxUI and in turn ofXmlSettings
 */

float radius;
bool cull;
bool useSpot;
bool usePoint;
bool autoRotatePoint;
bool useAmb;
bool useDir;
bool showDirSource;
bool showSpotSource;
bool showPointSource;

bool doReset;

vector<ofxUISlider *> ambslider;
vector<ofxUISlider *> spotspecslider;
vector<ofxUISlider *> dirspecslider;
vector<ofxUISlider *> pointspecslider;

//--------------------------------------------------------------
void testApp::setup(){

    ofBackground(0);
    ofSetFrameRate(30);
    ofEnableAlphaBlending();
    ofEnableSmoothing();
    ofSetGlobalAmbientColor(ofColor(0, 0, 0));
    ofSetSmoothLighting(true);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    doReset = guiAlloc = false;
    ofSetSphereResolution(100);
    reset();
}

void testApp::reset()
{
    //sphere radius
    radius = 100;
    
    spot.setDiffuseColor(ofFloatColor(255.0, 0.0, 0.0f));
    spot.setSpecularColor(ofColor(0, 0, 255));
    spot.setSpotlight();
    spot.setSpotConcentration(5);
    spot.setSpotlightCutOff(10);
    spot_rot = ofVec3f(0, 0, 0);
    setLightOri(spot, spot_rot);

    spot.setPosition(0, 0, 300);
    
    point.setDiffuseColor(ofColor(0.0, 255.0, 0.0));
    
    point.setPointLight();
    
    dir.setDiffuseColor(ofColor(0.0f, 0.0f, 255.0f));
    dir.setSpecularColor(ofColor(255.0f, 0.0f, 0.0f));
    
    dir.setDirectional();
    dir_rot = ofVec3f(0, -95, 0);
    setLightOri(dir, dir_rot);
    
    amb.setAmbientColor(ofColor(20.0, 20.0, 20.0, 20.0));
    
    material.setShininess(120);

    material.setSpecularColor(ofColor(255, 255, 255, 255));
    material.setEmissiveColor(ofColor(0, 0, 0, 255));
    material.setDiffuseColor(ofColor(200, 200, 200, 255));
    material.setAmbientColor(ofColor(200, 200, 200, 255));
    
    cull = true;
    useSpot = true;
    usePoint = false;
    useAmb = true;
    useDir = true;
    autoRotatePoint = true;
    showDirSource = false;
    showSpotSource = true;
    showPointSource = true;
    
    cam.reset();
    setGUI();
    doReset = false;
    
    
}

//--------------------------------------------------------------
void testApp::update(){

    if (doReset) 
        reset();

    ofVec3f center = ofVec3f(0);
    if (autoRotatePoint) {
        point.setPosition(cos(ofGetElapsedTimef()*.6f) * radius * 2 + center.x,
						   sin(ofGetElapsedTimef()*.8f) * radius * 2 + center.y,
						   -cos(ofGetElapsedTimef()*.8f) * radius * 2 + center.z);
    }

}

//--------------------------------------------------------------
void testApp::draw(){

    if (cull)
        glEnable(GL_CULL_FACE);
    
    cam.begin();

    //draw x/y/z axis
    ofSetLineWidth(5);
    ofSetColor(255, 0, 0);
    ofLine(0, 0, 200, 0);
    ofSetColor(0, 255, 0);
    ofLine(0, 0, 0, 200);
    ofSetColor(0, 0, 255);
    ofLine(0, 0, 0, 0, 0, 200);

    ofEnableLighting();
    material.begin();
    
    if (useAmb)
        amb.enable();
    else
        amb.disable();
    
    if (useDir)
        dir.enable();
    else
        dir.disable();
    
    if (useSpot)
        spot.enable();
    else
        spot.disable();
    
    if (usePoint)
        point.enable();
    else
        point.disable();
    
    //white sphere
    ofSetColor(255);
    ofSphere(0,0,0, radius);

    amb.disable();
    dir.disable();
    spot.disable();
    point.disable();
    material.end();
    ofDisableLighting();
    
    if (useSpot && showSpotSource) {
        ofSetColor(spot.getDiffuseColor());
        spot.draw();
    }
    
    if (usePoint && showPointSource) {
        ofSetColor(point.getDiffuseColor());
        point.draw();
    }
    
    if (useDir && showDirSource) {
        ofSetColor(dir.getDiffuseColor());
        dir.draw();
    }
    
    cam.end();
    
    if (cull)
        glDisable(GL_CULL_FACE);
}

void testApp::guiEvent(ofxUIEventArgs &e)
{
	string name = e.widget->getName();
	int kind = e.widget->getKind();
   
	if (name == "Radius") {
        ofxUISlider *slider = (ofxUISlider *) e.widget;
        radius = slider->getScaledValue();
    } else if (name == "Spot Source") {
        ofxUIToggle *toggle = (ofxUIToggle *) e.widget;
        showSpotSource = toggle->getValue();
    } else if (name == "Cutoff") {
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        spot.setSpotlightCutOff(slider->getScaledValue());
    } else if (name == "Cons") {
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        spot.setSpotConcentration(slider->getScaledValue());
    } else if (name == "SX_POS") {
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        ofVec3f curpos = spot.getPosition();
        curpos.x = slider->getScaledValue();
        spot.setPosition(curpos);
    } else if (name == "SY_POS") {
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        ofVec3f curpos = spot.getPosition();
        curpos.y = slider->getScaledValue();
        spot.setPosition(curpos);
    } else if (name == "SZ_POS") {
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        ofVec3f curpos = spot.getPosition();
        curpos.z = slider->getScaledValue();
        spot.setPosition(curpos);
        
    /* spot orientation */
    } else if (name == "SX_ORI") {
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        spot_rot.x = slider->getScaledValue();
        setLightOri(spot, spot_rot);
    } else if (name == "SY_ORI") {
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        spot_rot.y = slider->getScaledValue();
        setLightOri(spot, spot_rot);
    } else if (name == "SZ_ORI") {
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        spot_rot.z = slider->getScaledValue();
        setLightOri(spot, spot_rot);
        
    /* spot diffuse colour */
    } else if (name == "SDR") {
        ofColor c = spot.getDiffuseColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.r = slider->getScaledValue();
        spot.setDiffuseColor(c);
    } else if (name == "SDG") {
        ofColor c = spot.getDiffuseColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.g = slider->getScaledValue();
        spot.setDiffuseColor(c);
    } else if (name == "SDB") {
        ofColor c = spot.getDiffuseColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.b = slider->getScaledValue();
        spot.setDiffuseColor(c);
    } else if (name == "SDA") {
        ofColor c = spot.getDiffuseColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.a = slider->getScaledValue();
        spot.setDiffuseColor(c);
        
        /* spot specular colour */
    } else if (name == "SSR") {
        ofColor c = spot.getSpecularColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.r = slider->getScaledValue();
        spot.setSpecularColor(c);
    } else if (name == "SSG") {
        ofColor c = spot.getSpecularColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.g = slider->getScaledValue();
        spot.setSpecularColor(c);
    } else if (name == "SSB") {
        ofColor c = spot.getSpecularColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.b = slider->getScaledValue();
        spot.setSpecularColor(c);
    } else if (name == "SSA") {
        ofColor c = spot.getSpecularColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.a = slider->getScaledValue();
        spot.setSpecularColor(c);
    } else if (name == "SSW") {
        ofColor c = spot.getSpecularColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.r = c.g = c.b = c.a = slider->getScaledValue();
        spot.setSpecularColor(c);
        
        for (int i = 0; i < spotspecslider.size(); i++)
            spotspecslider.at(i)->setValue(slider->getScaledValue());
        
    /* directional light parms */
    } else if (name == "Dir Source") {
        ofxUIToggle *toggle = (ofxUIToggle *) e.widget;
        showDirSource = toggle->getValue();
    } else if (name == "DIR_XORI") {
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        dir_rot.x = slider->getScaledValue();
        setLightOri(dir, dir_rot);
    } else if (name == "DIR_YORI") {
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        dir_rot.y = slider->getScaledValue();
        setLightOri(dir, dir_rot);
    } else if (name == "DIR_ZORI") {
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        dir_rot.z = slider->getScaledValue();
        setLightOri(dir, dir_rot);
        
        /* directional diffuse colour */
    } else if (name == "DDR") {
        ofColor c = dir.getDiffuseColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.r = slider->getScaledValue();
        dir.setDiffuseColor(c);
    } else if (name == "DDG") {
        ofColor c = dir.getDiffuseColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.g = slider->getScaledValue();
        dir.setDiffuseColor(c);
    } else if (name == "DDB") {
        ofColor c = dir.getDiffuseColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.b = slider->getScaledValue();
        dir.setDiffuseColor(c);
    } else if (name == "DDA") {
        ofColor c = dir.getDiffuseColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.a = slider->getScaledValue();
        dir.setDiffuseColor(c);
        
        
    /* directional specular colour */
    } else if (name == "DSR") {
        ofColor c = dir.getSpecularColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.r = slider->getScaledValue();
        dir.setSpecularColor(c);
    } else if (name == "DSG") {
        ofColor c = dir.getSpecularColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.g = slider->getScaledValue();
        dir.setSpecularColor(c);
    } else if (name == "DSB") {
        ofColor c = dir.getSpecularColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.b = slider->getScaledValue();
        dir.setSpecularColor(c);
    } else if (name == "DSA") {
        ofColor c = dir.getSpecularColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.a = slider->getScaledValue();
        dir.setSpecularColor(c);
    } else if (name == "DSW") {
        ofColor c = dir.getSpecularColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.r = c.g = c.b = c.a = slider->getScaledValue();
        dir.setSpecularColor(c);
        
        for (int i = 0; i < dirspecslider.size(); i++)
            dirspecslider.at(i)->setValue(slider->getScaledValue());
        
    /* point diffuse colour */
    } else if (name == "Point Source") {
        ofxUIToggle *toggle = (ofxUIToggle *) e.widget;
        showPointSource = toggle->getValue();
        
    } else if (name == "PDR") {
        ofColor c = point.getDiffuseColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.r = slider->getScaledValue();
        point.setDiffuseColor(c);
    } else if (name == "PDG") {
        ofColor c = point.getDiffuseColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.g = slider->getScaledValue();
        point.setDiffuseColor(c);
    } else if (name == "PDB") {
        ofColor c = point.getDiffuseColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.b = slider->getScaledValue();
        point.setDiffuseColor(c);
    } else if (name == "PDA") {
        ofColor c = point.getDiffuseColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.a = slider->getScaledValue();
        point.setDiffuseColor(c);
        
        
        /* point specular colour */
    } else if (name == "PSR") {
        ofColor c = point.getSpecularColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.r = slider->getScaledValue();
        point.setSpecularColor(c);
    } else if (name == "PSG") {
        ofColor c = point.getSpecularColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.g = slider->getScaledValue();
        point.setSpecularColor(c);
    } else if (name == "PSB") {
        ofColor c = point.getSpecularColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.b = slider->getScaledValue();
        point.setSpecularColor(c);
    } else if (name == "PSA") {
        ofColor c = point.getSpecularColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.a = slider->getScaledValue();
        point.setSpecularColor(c);
    } else if (name == "PSW") {
        ofColor c = point.getSpecularColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.r = c.g = c.b = c.a = slider->getScaledValue();
        point.setSpecularColor(c);
        
        for (int i = 0; i < pointspecslider.size(); i++)
            pointspecslider.at(i)->setValue(slider->getScaledValue());
        
    /* point auto move */
    } else if (name == "Auto Move") {
        ofxUIToggle *toggle = (ofxUIToggle *)e.widget;
        autoRotatePoint = toggle->getValue();

    /* misc toggles */
    } else if (name == "cull") {
        ofxUIToggle *toggle = (ofxUIToggle *)e.widget;
        cull = toggle->getValue();
    } else if (name == "point") {
        ofxUIToggle *toggle = (ofxUIToggle *)e.widget;
        usePoint = toggle->getValue();
    } else if (name == "spot") {
        ofxUIToggle *toggle = (ofxUIToggle *)e.widget;
        useSpot = toggle->getValue();
    } else if (name == "dir") {
        ofxUIToggle *toggle = (ofxUIToggle *)e.widget;
        useDir = toggle->getValue();
    
    /* material shininess */
    } else if (name == "MAT SHINE") {
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        material.setShininess(slider->getScaledValue());
        
    /* material diffuse/ambient colour */
    } else if (name == "MDR") {
        ofColor c = material.getDiffuseColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.r = slider->getScaledValue();
        material.setDiffuseColor(c);
        material.setAmbientColor(c);

    } else if (name == "MDG") {
        ofColor c = material.getDiffuseColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.g = slider->getScaledValue();
        material.setDiffuseColor(c);
        material.setAmbientColor(c);
    } else if (name == "MDB") {
        ofColor c = material.getDiffuseColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.b = slider->getScaledValue();
        material.setDiffuseColor(c);
        material.setAmbientColor(c);
    } else if (name == "MDA") {
        ofColor c = material.getDiffuseColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.a = slider->getScaledValue();
        material.setDiffuseColor(c);
        material.setAmbientColor(c);
        
    /* material emissive colour */
    } else if (name == "MER") {
        ofColor c = material.getEmissiveColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.r = slider->getScaledValue();
        material.setEmissiveColor(c);
    } else if (name == "MEG") {
        ofColor c = material.getEmissiveColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.g = slider->getScaledValue();
        material.setEmissiveColor(c);
    } else if (name == "MEB") {
        ofColor c = material.getEmissiveColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.b = slider->getScaledValue();
        material.setEmissiveColor(c);
    } else if (name == "MEA") {
        ofColor c = material.getEmissiveColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.a = slider->getScaledValue();
        material.setEmissiveColor(c);
        
    /* material specular colour */
    } else if (name == "MSR") {
        ofColor c = material.getSpecularColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.r = slider->getScaledValue();
        material.setSpecularColor(c);
    } else if (name == "MSG") {
        ofColor c = material.getSpecularColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.g = slider->getScaledValue();
        material.setSpecularColor(c);
    } else if (name == "MSB") {
        ofColor c = material.getSpecularColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.b = slider->getScaledValue();
        material.setSpecularColor(c);
    } else if (name == "MSA") {
        ofColor c = material.getSpecularColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.a = slider->getScaledValue();
        material.setSpecularColor(c);
        
    /* ambient light stuff */
    } else if (name == "ambient") {
        ofxUIToggle *toggle = (ofxUIToggle *)e.widget;
        useAmb = toggle->getValue();
    } else if (name == "AR") {
        ofColor c = amb.getAmbientColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.r = slider->getScaledValue();
        amb.setAmbientColor(c);
    } else if (name == "AG") {
        ofColor c = amb.getAmbientColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.g = slider->getScaledValue();
        amb.setAmbientColor(c);
    } else if (name == "AB") {
        ofColor c = amb.getAmbientColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.b = slider->getScaledValue();
        amb.setAmbientColor(c);
    } else if (name == "AA") {
        ofColor c = amb.getAmbientColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.a = slider->getScaledValue();
        amb.setAmbientColor(c);
    } else if (name == "AW") {
        ofColor c = amb.getAmbientColor();
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        c.r = c.g = c.b = c.a = slider->getScaledValue();
        amb.setAmbientColor(c);
        
        for (int i = 0; i < ambslider.size(); i++)
            ambslider.at(i)->setValue(slider->getScaledValue());
        
    } else if (name == "RESET") {
        doReset = true;
    }
    
}

void
testApp::setLightOri(ofLight &light, ofVec3f rot)
{
    ofVec3f xax(1, 0, 0);
    ofVec3f yax(0, 1, 0);
    ofVec3f zax(0, 0, 1);
    ofQuaternion q;
    q.makeRotate(rot.x, xax, rot.y, yax, rot.z, zax);
    light.setOrientation(q);
}

void
testApp::setGUI()
{
    
    float xInit = OFX_UI_GLOBAL_WIDGET_SPACING;
    float w = 300 - xInit * 2;
    float vertH = 40;
    float h = 8;

    if (guiAlloc) {
        ofRemoveListener(gui->newGUIEvent, this, &testApp::guiEvent);
        ambslider.clear();
        spotspecslider.clear();
        delete gui;
    }
    
    gui = new ofxUICanvas(0, 0, w + xInit * 2, ofGetHeight());
    gui->addWidgetDown(new ofxUILabel("LIGHT CONTROL", OFX_UI_FONT_MEDIUM));
    
    gui->addSlider("Radius", 5, 1000, radius, w, h);
    
    gui->addSpacer(w, 2);
    gui->addWidgetDown(new ofxUILabel("Spot Light Control", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUIToggle("Spot Source", showSpotSource, 10, 15));
    gui->addWidgetDown(new ofxUILabel("Spot Location", OFX_UI_FONT_SMALL));
    
    gui->addSlider("SX_POS", -500, 500, spot.getPosition().x, w, h);
    gui->addSlider("SY_POS", -500, 500, spot.getPosition().y, w, h);
    gui->addSlider("SZ_POS", -500, 500, spot.getPosition().z, w, h);

    gui->addWidgetDown(new ofxUILabel("Spot Orientation", OFX_UI_FONT_SMALL));
    gui->addSlider("SX_ORI", -180, 180, spot.getOrientationEuler().x, w, h);
    gui->addSlider("SY_ORI", -180, 180, spot.getOrientationEuler().y, w, h);
    gui->addSlider("SZ_ORI", -180, 180, spot.getOrientationEuler().z, w, h);

    gui->addWidgetDown(new ofxUILabel("Spot Param", OFX_UI_FONT_SMALL));    
    gui->addSlider("Cutoff", 0.0, 90.0, spot.getSpotlightCutOff(), w, h);
    gui->addSlider("Cons", 0.0, 128.0, spot.getSpotConcentration(), w, h);
    
    gui->addWidgetDown(new ofxUILabel("Spot Diffuse/Specular Color", OFX_UI_FONT_SMALL));
    ofColor c = spot.getDiffuseColor();
    gui->addSlider("SDR", 0, 255, c.r, h, vertH);
    gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui->addSlider("SDG", 0, 255, c.g, h, vertH);
    gui->addSlider("SDB", 0, 255, c.b, h, vertH);
//    gui->addSlider("SDA", 0, 255, c.a, h, vertH);
    
    gui->addSpacer(2, vertH+10);

    c = spot.getSpecularColor();
    spotspecslider.push_back(gui->addSlider("SSR", 0, 255, c.r, h, vertH));
    spotspecslider.push_back(gui->addSlider("SSG", 0, 255, c.g, h, vertH));
    spotspecslider.push_back(gui->addSlider("SSB", 0, 255, c.b, h, vertH));
//    spotspecslider.push_back(gui->addSlider("SSA", 0, 255, c.a, h, vertH));
    gui->addSpacer(2, vertH+10);
    gui->addSlider("SSW", 0, 255, c.r == c.b && c.r == c.g ? c.r : 0.0, h, vertH);
    gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    
    gui->addSpacer(w, 2);
    gui->addWidgetDown(new ofxUILabel("Directional Light Control", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUIToggle("Dir Source", showDirSource, 10, 15));

    gui->addWidgetDown(new ofxUILabel("Directional Orientation", OFX_UI_FONT_SMALL));

    gui->addSlider("DIR_XORI", -180, 180, dir.getOrientationEuler().x, w, h);
    gui->addSlider("DIR_YORI", -180, 180, dir.getOrientationEuler().y, w, h);
    gui->addSlider("DIR_ZORI", -180, 180, dir.getOrientationEuler().z, w, h);
    

    gui->addWidgetDown(new ofxUILabel("Directional Diffuse/Specular Color", OFX_UI_FONT_SMALL));
    c = dir.getDiffuseColor();
    gui->addSlider("DDR", 0, 255, c.r, h, vertH);
    gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui->addSlider("DDG", 0, 255, c.g, h, vertH);
    gui->addSlider("DDB", 0, 255, c.b, h, vertH);
    
    gui->addSpacer(2, vertH+10);
    c = dir.getSpecularColor();
    dirspecslider.push_back(gui->addSlider("DSR", 0, 255, c.r, h, vertH));
    dirspecslider.push_back(gui->addSlider("DSG", 0, 255, c.g, h, vertH));
    dirspecslider.push_back(gui->addSlider("DSB", 0, 255, c.b, h, vertH));
//    dirspecslider.push_back(gui->addSlider("DSA", 0, 255, c.a, h, vertH));
    gui->addSpacer(2, vertH+10);
    float ival = c.r == c.b && c.r == c.g ? c.r : 0.0;
    gui->addSlider("DSW", 0, 255, c.r == c.b && c.r == c.g ? c.r : 0.0, h, vertH);
    gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    
    gui->addSpacer(w, 2);
    gui->addWidgetDown(new ofxUILabel("Point Light Control", OFX_UI_FONT_MEDIUM));
    gui->addWidgetRight(new ofxUIToggle("Point Source", showPointSource, 10, 15));
    gui->addWidgetDown(new ofxUILabel("Point Diffuse/Specular Color", OFX_UI_FONT_SMALL));
    c = point.getDiffuseColor();
    gui->addSlider("PDR", 0, 255, c.r, h, vertH);
    gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui->addSlider("PDG", 0, 255, c.g, h, vertH);
    gui->addSlider("PDB", 0, 255, c.b, h, vertH);
//    gui->addSlider("PDA", 0, 255, c.a, h, vertH);
    
    gui->addSpacer(2, vertH+10);
    c = point.getSpecularColor();
    pointspecslider.push_back(gui->addSlider("PSR", 0, 255, c.r, h, vertH));
    pointspecslider.push_back(gui->addSlider("PSG", 0, 255, c.g, h, vertH));
    pointspecslider.push_back(gui->addSlider("PSB", 0, 255, c.b, h, vertH));
//    pointspecslider.push_back(gui->addSlider("PSA", 0, 255, c.a, h, vertH));
    gui->addSpacer(2, vertH+10);
    gui->addSlider("PSW", 0, 255, c.r == c.b && c.r == c.g ? c.r : 0.0, h, vertH);
    gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    
    gui->addToggle("Auto Move", autoRotatePoint);
    
    gui->addSpacer(w, 2);
    gui->addWidgetDown(new ofxUILabel("Ambient Light Control", OFX_UI_FONT_MEDIUM));
    gui->addWidgetDown(new ofxUILabel("Ambient Light Color (w for white)", OFX_UI_FONT_SMALL));

    c = amb.getAmbientColor();
    ambslider.push_back(gui->addSlider("AR", 0, 255.0, c.r, h, vertH));
    gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    ambslider.push_back(gui->addSlider("AG", 0, 255.0, c.g, h, vertH));
    ambslider.push_back(gui->addSlider("AB", 0, 255.0, c.b, h, vertH));
//    ambslider.push_back(gui->addSlider("AA", 0, 255, c.a, h, vertH));
    gui->addSpacer(2, vertH+10);
    gui->addSlider("AW", 0, 255.0, c.r, h, vertH);
    gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    
    //Material Control
    
    gui->addSpacer(w, 2);
    gui->addWidgetDown(new ofxUILabel("Material Control", OFX_UI_FONT_MEDIUM));
    gui->addSlider("MAT SHINE", 0, 128, material.getShininess(), w, h);
    gui->addWidgetDown(new ofxUILabel("Material Diffuse/Emissive/Specular Color", OFX_UI_FONT_SMALL));
    
    c = material.getDiffuseColor();
    gui->addSlider("MDR", 0, 255, c.r, h, vertH);
    gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui->addSlider("MDG", 0, 255, c.g, h, vertH);
    gui->addSlider("MDB", 0, 255, c.b, h, vertH);
//    gui->addSlider("MDA", 0, 255, c.a, h, vertH);
    
    gui->addSpacer(2, vertH+10);
    c = material.getEmissiveColor();
    gui->addSlider("MER", 0, 255, c.r, h, vertH);
    gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui->addSlider("MEG", 0, 255, c.g, h, vertH);
    gui->addSlider("MEB", 0, 255, c.b, h, vertH);
//    gui->addSlider("MDA", 0, 255, c.a, h, vertH);
    
    gui->addSpacer(2, vertH+10);
    c = material.getSpecularColor();
    gui->addSlider("MSR", 0, 255, c.r, h, vertH);
    gui->addSlider("MSG", 0, 255, c.g, h, vertH);
    gui->addSlider("MSB", 0, 255, c.b, h, vertH);
//    gui->addSlider("MSA", 0, 255, c.a, h, vertH);
    gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);
    
    gui->addSpacer(w, 2);
    gui->addToggle("cull", cull);
    gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_RIGHT);
    gui->addToggle("spot", useSpot);
    gui->addToggle("point", usePoint);
    gui->addToggle("dir", useDir);
    gui->addToggle("ambient", useAmb);
    gui->setWidgetPosition(OFX_UI_WIDGET_POSITION_DOWN);

    
    gui->addSpacer(w, 2);
    gui->addLabelButton("RESET", false);
    
    ofAddListener(gui->newGUIEvent, this, &testApp::guiEvent);
    guiAlloc = true;
    
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    switch (key) {
        case 'm':
            gui->toggleVisible();
            break;
        default:
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
    if (gui->isHit(x, y)) {
        cam.disableMouseInput();
    }
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