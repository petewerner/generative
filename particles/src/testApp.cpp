#include "testApp.h"

/*
 spread	speed	samp_coeff	circ_coeff
 10		1		0.035		0.1			//default
 3		1		0.015		0.24		//noice
 1		2.5		0.015		0.25		//smooth fall, jumpt to spread = 2 is cool also
*/

float *noise;
float spread = 1.0f;
float speed = 2.5f;
float samp_coeff = 0.015;// 0.035f;
float samp_var = 0.005f;
float rc = 0.01;
float circ_coeff = 0.01; //0.25;//0.1f;

#define MAX_PARTICLES	150000

float	pos[MAX_PARTICLES][3];
float	vel[MAX_PARTICLES][3];
float	col[MAX_PARTICLES][4];

int			numParticles		= 150000;
int			adder				= 10000;
float		currentRot			= 0;
bool		vsync				= false;
bool		vboInited			= false;
int			pointSize			= 1;

bool drawMap = false;
bool drawParts = true;
bool output = false;

enum DrawMode {
	DrawCos,
	DrawSin,
	DrawMap
};
DrawMode drawMode;

GLuint		vbo[2];

void build_noise();


void init_parts() 
{
	for(int i=0; i<MAX_PARTICLES; i++) {
		pos[i][0] = ofRandom(0, ofGetWidth());
		pos[i][1] = ofRandom(0, ofGetHeight());
		pos[i][2] = 0;
		
		vel[i][0] = ofRandom(-1, 1); 
		vel[i][1] = ofRandom(-1, 1); 
		vel[i][2] = ofRandom(-1, 1); 
		
		col[i][0] = 0.7f;//ofRandom(0, 1);
		col[i][1] = 0.1;//ofRandom(0, 1);
		col[i][2] = 0.1;//ofRandom(0, 1);
		col[i][3] = 0;//ofRandom(0, 1);
	}	
	
}

//--------------------------------------------------------------
void testApp::setup(){
	ofBackground(0);
	ofSetFrameRate(30);
	
	int w = ofGetWidth();
	int h = ofGetHeight();
	noise = new float[w * h];
	memset(noise, 0x00, w*h*sizeof(float));

	drawMode = DrawSin;

	build_noise();
	init_parts();
	vboInited = true;
		
	// initialize VBO
	glGenBuffersARB(2, vbo);
	
	// vbo for vertex positions
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[0]);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(pos), pos, GL_STREAM_DRAW_ARB);
	//	
	//	// vbo for vertex colors
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[1]);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(col), col, GL_STREAM_DRAW_ARB);


}

//--------------------------------------------------------------
void testApp::update(){
	

	int w = ofGetWidth();
	int h = ofGetHeight();

	if (drawParts) {
		
		for (int i = 0; i < numParticles; i++) {
			float x = pos[i][0];
			float y = pos[i][1];
				
			int idx_x = x;
			int idx_y = y;
			float idxNorm = i / (float)numParticles;
			float a = noise[idx_x + idx_y * h] + idxNorm * circ_coeff;
	
			float angle = ofLerp(0.0, spread*3.14159f, a);
			float cang = cos(angle) ;
			float sang = sin(angle) ;

			x += speed * cang;// * 0.5 + 0.4;
			y += speed * sang;// * 0.5 + 0.4;
			
//			float alph = 1 - ((cang * 0.5 + 0.5f) + (sang * 0.5 + 0.5))/2.0;
			
			if (x < 1 || x >= w || y < 1 || y >= h){
				pos[i][0] = ofRandom(0, w);
				pos[i][1] = ofRandom(0, h);
				
				float x2 = pos[i][0];
				float y2 = pos[i][1];
				
				int idx_x2 = x2;
				int idx_y2 = y2;
				float idxNorm = i / (float)numParticles;
				float a = noise[idx_x2 + idx_y2 * h] + idxNorm * circ_coeff;
				
				float angle = ofLerp(0.0, spread*3.14159f, a);
				float cang = cos(angle) ;
				float sang = sin(angle) ;
				
				float alph = 1 - ((cang * 0.5 + 0.5f) + (sang * 0.5 + 0.5))/2.0;				
				
				col[i][0] = max(0.7f, alph * 0.95f);
				col[i][2] = min((1 - alph) * 0.7f, 0.3f);
				col[i][3] = alph;
				
				//gone now we set the color based on the initial position, see cmt below
//				col[i][0] = 0.7f;
//				col[i][1] = 0.1;
//				col[i][2] = 0.1;
//				col[i][3] = 0.0f;
			} else {
				pos[i][0] = x;
				pos[i][1] = y;
			
				//used to change the color based on where it had moved, but 
				//setting the color once when we redraw the part looks a bit nicer
//				col[i][0] = max(0.7f, alph * 0.95f);
//				col[i][2] = min((1 - alph) * 0.7f, 0.3f);
//				col[i][3] = alph;
			}

		}
		
	}
	
	
}

//--------------------------------------------------------------
void testApp::draw(){
	int w = ofGetWidth();
	int h = ofGetHeight();
	
	if (drawMap) {
		glEnable(GL_DEPTH_TEST);
		glPointSize(1);
		ofEnableAlphaBlending();
		glBegin(GL_POINTS);	
		
		for (int x = 0; x < w; x++) {
			for (int y = 0; y < h; y++) {
	
				float idxNorm = (x + y * h) / (float)numParticles;
				float a = noise[x + y * h];// + idxNorm * circ_coeff;	

				float angle = ofLerp(0.0, spread*3.14159f, a);
				
				float v;
				if (drawMode == DrawCos) 
					v = ofMap(cos(angle), -1.0f, 1.0f, 0.0f, 1.0f);
				else if (drawMode == DrawSin) 
					v = ofMap(sin(angle), -1.0f, 1.0f, 0.0f, 1.0f);
				else 
					v = a;
				glColor4f(1.0, 1.0f, 1.0f, v);
				glVertex2f(x, y);
			}
		}
		output = true;
		glEnd();	
		glDisable(GL_DEPTH_TEST);		
		ofDisableAlphaBlending();
	}
	
	if (drawParts) {
		ofEnableAlphaBlending();
		glPushMatrix();
		
//		glTranslatef(ofGetWidth()/2, 0, 0);
//		glRotatef(currentRot, 0, 1, 0);
		
		ofSetColor(255);
		
		glEnable(GL_DEPTH_TEST);
		glPointSize(pointSize);
		
		glEnable(GL_POINT_SPRITE);
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[0]);
		glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, numParticles * 3 * sizeof(float), pos);
		glVertexPointer(3, GL_FLOAT, 0, 0);
		
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo[1]);
		glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, numParticles * 4 * sizeof(float), col);
		glColorPointer(4, GL_FLOAT, 0, 0);
		
		
		glDrawArrays(GL_POINTS, 0, numParticles);
		
		glDisableClientState(GL_VERTEX_ARRAY); 
		glDisableClientState(GL_COLOR_ARRAY);
		
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

		glDisable(GL_POINT_SPRITE);
		glDisable(GL_DEPTH_TEST);	
		glPopMatrix();
		ofDisableAlphaBlending();		
	
	}
	
}

float Cosine_Interpolate(float a, float b, float x)
{
	float ft = x * 3.1415927f;
	float f = (1.0f - cos(ft)) * 0.5f;
	
	return  a * ( 1 - f ) + b * f;
}

float Noise(int x, int y)
{
	int n = x + y * 57;
	n = ( n << 13 ) ^ n;
	return ( 1.f - ( ( n * ( n * n * 15731 + 789221 ) + 1376312589 ) & 0x7fffffff ) / 1073741824.0 );    
}

float SmoothedNoise1(float x, float y)
{
	float corners = ( Noise(x-1, y-1)+Noise(x+1, y-1)+Noise(x-1, y+1)+Noise(x+1, y+1) ) / 16.0f;
	float sides   = ( Noise(x-1, y)  +Noise(x+1, y)  +Noise(x, y-1)  +Noise(x, y+1) ) /  8.0f;
	float center  =  Noise(x, y) / 4.0f;
	return corners + sides + center;
}

float InterpolatedNoise1(float x, float y)
{
	int integer_X    = (int)floor(x);
	float fractional_X = fabs (x - integer_X);
	
	int integer_Y    = (int)floor(y);
	float fractional_Y = fabs(y - integer_Y);
	
	float v1 = SmoothedNoise1(integer_X,     integer_Y);
	float v2 = SmoothedNoise1(integer_X + 1, integer_Y);
	float v3 = SmoothedNoise1(integer_X,     integer_Y + 1);
	float v4 = SmoothedNoise1(integer_X + 1, integer_Y + 1);
	
	float i1 = Cosine_Interpolate(v1 , v2 , fractional_X);
	float i2 = Cosine_Interpolate(v3 , v4 , fractional_X);
	
	return Cosine_Interpolate(i1 , i2 , fractional_Y);
}

float GetPerlin( float x, float y)
{
	float total = 0.0f;
	float p = /*persistence*/0.5f;
	float freq = 2.0f;
	int n = /*Number_Of_Octaves*/8- 1;
	
	for( int i = 0; i < n; ++i )
	{
		float frequency = pow(freq, (float)i);
		float amplitude = pow(p, (float)i);
		
		total = total + InterpolatedNoise1(x * frequency, y * frequency) * amplitude;
	}
	
	return total;
}

void build_noise() {
	int w = ofGetWidth();
	int h = ofGetHeight();
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			//float n = ofSignedNoise(x * samp_coeff + ofRandom(-rc, rc), y * samp_coeff + ofRandom(-rc, rc)) * 0.5 + 0.5;
			float n = GetPerlin(x * samp_coeff + ofRandom(-rc, rc), y * samp_coeff + ofRandom(-rc, rc)) * 0.5 + 0.5;			
			noise[x + y * h] = n;
		}
	}	
	printf("sample_coeff %.5f\n", samp_coeff);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	
	string s = "fps: " + ofToString(ofGetFrameRate(), 2);
	cout << s << endl;
	
	switch (key) {
		case 'm':
			drawMap = !drawMap;
			break;
		case 'p':
			drawParts = !drawParts;
			break;
		case '+':
			spread += 1.0;
			printf("spread: %.2f\n", spread);
			break;
		case '-':
			if (spread >= 1.0) spread -= 1.0;
			printf("spread: %.2f\n", spread);
			break;
		case '[':
			if (circ_coeff >= 0.01) {
				circ_coeff -= 0.01;
				printf("circ coeff %.2f\n", circ_coeff);
			}
			break;
		case ']':
			circ_coeff += 0.01;
			printf("circ coeff %.2f\n", circ_coeff);
			break;
		case 'v':
			vsync = !vsync;
			ofSetVerticalSync(vsync);
			cout << "Vsync: " << ofToString(vsync) << endl;
			break;
		case 'c':
			drawMode = DrawCos;
			output = false;
			break;
		case 's':
			drawMode = DrawSin;
			output = false;
			break;
		case 'x':
			drawMode = DrawMap;
			break;
		case 'r':
			init_parts();
			break;
		case ',' :
			if (samp_coeff > samp_var) {
				samp_coeff -= samp_var;
				build_noise();
			}
			break;
		case '.':
			samp_coeff += samp_var;
			build_noise();
			break;
		case 'y':
			if (speed > 0.5f) speed -= 0.5f;
			printf("speed: %.1f\n", speed);
			break;
		case 'u':
			speed += 0.5f;
			printf("speed: %.1f\n", speed);
			break;
		case '1':
			spread = 1.0;
			break;
		case '2':
			spread = 2.0;
			break;
		case '3':
			spread = 3.0;
			break;
		case '8':
			speed = 1.5;
			spread = 8.0;
			break;
		case '0':
			spread = 10.0;
			break;
	}
}

void testApp::exit() {
    if(vboInited) glDeleteBuffersARB(2, vbo);
	
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

	float a = noise[x + y * ofGetHeight()];
//	a += ofRandom(-0.1, 0.1);
	float angle = ofLerp(0.0, spread*3.14159f, a);
//	angle = a;
	float new_x = x + speed * cos(angle);
	float new_y = y + speed * sin(angle);
	
	printf("%d, %d a %.4f, angle %.4f, cos %.4f, sin %.4f speed x/y %.2f/%.2f: %d,%d -> %.1f, %.1f\n", x, y, a, angle, cos(angle), sin(angle), speed * cos(angle), speed * sin(angle), x, y, new_x, new_y);
	
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