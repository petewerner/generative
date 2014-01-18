/*
 *  ofxFboBlur.h
 *  emptyExample
 *
 *  Created by Oriol Ferrer Mesià on 26/03/12.
 *  Copyright 2012 uri.cat. All rights reserved.
 *
 * peterw - made some small changes, mostly getting sizes
 *  originally here https://github.com/armadillu/ofxFboBlur
 */

#include "ofMain.h"

#define STRINGIFY(A) #A

class ofxFboBlur{

public:
	
	void setup(ofFbo::Settings s, bool additive){

		backgroundColor = ofColor(0,0,0,0);

		string fragV = "#extension GL_ARB_texture_rectangle : enable\n" + (string)
		STRINGIFY(
			 uniform float blurLevel;
			 uniform sampler2DRect texture;

			 void main(void){

				 float blurSize = blurLevel ;
				 vec4 sum = vec4(0.0);

				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y - 4.0 * blurSize)) * 0.05;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y - 3.0 * blurSize)) * 0.09;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y - 2.0 * blurSize)) * 0.12;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y - blurSize)) * 0.15;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y)) * 0.2;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + blurSize)) * 0.15;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + 2.0 * blurSize)) * 0.12;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + 3.0 * blurSize)) * 0.09;
				 sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + 4.0 * blurSize)) * 0.05;
		);

		if(additive) fragV += STRINGIFY( if (sum.a > 0.0) sum.a = 1.0; );

		fragV += STRINGIFY(
				 gl_FragColor = sum;
			 }
		);

		string fragH = "#extension GL_ARB_texture_rectangle : enable\n" + (string)
		STRINGIFY(
				  uniform float blurLevel;
				  uniform sampler2DRect texture;

				  void main(void){

					  float blurSize = blurLevel;
					  vec4 sum = vec4(0.0);
					  vec2 st = gl_TexCoord[0].st;

					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x - 4.0 * blurSize, gl_TexCoord[0].y)) * 0.05;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x - 3.0 * blurSize, gl_TexCoord[0].y)) * 0.09;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x - 2.0 * blurSize, gl_TexCoord[0].y)) * 0.12;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x - blurSize, gl_TexCoord[0].y)) * 0.15;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y)) * 0.2;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x + blurSize, gl_TexCoord[0].y)) * 0.15;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x + 2.0 * blurSize, gl_TexCoord[0].y)) * 0.12;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x + 3.0 * blurSize, gl_TexCoord[0].y)) * 0.09;
					  sum += texture2DRect(texture, vec2(gl_TexCoord[0].x + 4.0 * blurSize, gl_TexCoord[0].y)) * 0.05;
					  
					);

		if(additive) fragH += STRINGIFY( if (sum.a > 0.0) sum.a = 1.0; );

		fragH += STRINGIFY(
					  gl_FragColor = sum;
				  }
		);

		string vertex =
		STRINGIFY(
			   void main() {
				   gl_TexCoord[0] = gl_MultiTexCoord0;
				   gl_Position = ftransform();
			   }
		);

		shaderV.setupShaderFromSource(GL_VERTEX_SHADER, vertex);
		shaderV.setupShaderFromSource(GL_FRAGMENT_SHADER, fragV);
		shaderV.linkProgram();
		
		shaderH.setupShaderFromSource(GL_VERTEX_SHADER, vertex);
		shaderH.setupShaderFromSource(GL_FRAGMENT_SHADER, fragH);
		shaderH.linkProgram();

		cleanImgFBO.allocate( s );
		blurOutputFBO.allocate( s );
		blurTempFBO.allocate( s );
		blurTempFBO2.allocate( s );

		blurOffset = 1.0;
		blurPasses = 2;
		numBlurOverlays = 1;
		blurOverlayGain = 255;
	}

                  float getWidth() {
                      return cleanImgFBO.getWidth();
                  }
                  float getHeight() {
                      return cleanImgFBO.getHeight();
                  }
                  
	void beginDrawScene(){
		cleanImgFBO.begin();
	}

	void endDrawScene(){
		cleanImgFBO.end();
	}

	void performBlur(){
		blur(&cleanImgFBO, &blurOutputFBO, &blurTempFBO, &blurTempFBO2, blurPasses, blurOffset);
	}

	void drawSceneFBO(){
#if (OF_VERSION_MINOR >= 8)
		cleanImgFBO.getTextureReference().draw(0, 0, cleanImgFBO.getWidth(), cleanImgFBO.getHeight());
#else
		cleanImgFBO.getTextureReference().draw(0, 0, cleanImgFBO.getWidth(), cleanImgFBO.getHeight());
#endif
	}

    void drawSceneFBO(float x, float y){
    #if (OF_VERSION_MINOR >= 8)
      cleanImgFBO.getTextureReference().draw(x, y, cleanImgFBO.getWidth(), cleanImgFBO.getHeight());
    #else
      cleanImgFBO.getTextureReference().draw(x, y, cleanImgFBO.getWidth(), cleanImgFBO.getHeight());
    #endif
    }

    void drawSceneFBO(float x, float y, float width, float height){
    #if (OF_VERSION_MINOR >= 8)
      cleanImgFBO.getTextureReference().draw(x, y, width, height);
    #else
      cleanImgFBO.getTextureReference().draw(x, y, width, height);
    #endif
    }
                  
	void drawBlurFbo(bool useCurrentColor = false){
		if(!useCurrentColor) ofSetColor(blurOverlayGain);
		for(int i = 0; i < numBlurOverlays; i++){
			#if (OF_VERSION_MINOR >= 8)
			blurOutputFBO.getTextureReference().draw(0, 0, blurOutputFBO.getWidth(), blurOutputFBO.getHeight());
			#else
			blurOutputFBO.getTextureReference().draw(0, blurOutputFBO.getHeight(), blurOutputFBO.getWidth(), -blurOutputFBO.getHeight());
			#endif
		}
	}

                  
                  
    void drawBlurFbo(float x, float y, bool useCurrentColor = false){
        if(!useCurrentColor)
            ofSetColor(blurOverlayGain);
        for(int i = 0; i < numBlurOverlays; i++){
        #if (OF_VERSION_MINOR >= 8)
            blurOutputFBO.getTextureReference().draw(x, y, blurOutputFBO.getWidth(), blurOutputFBO.getHeight());
        #else
            blurOutputFBO.getTextureReference().draw(x, y, blurOutputFBO.getWidth(), blurOutputFBO.getHeight());
        #endif
        }
    }

    void drawBlurFbo(float x, float y, float width, float height, bool useCurrentColor = false){
        if(!useCurrentColor)
            ofSetColor(blurOverlayGain);
        for (int i = 0; i < numBlurOverlays; i++){
        #if (OF_VERSION_MINOR >= 8)
            blurOutputFBO.getTextureReference().draw(x, y, width, height);
        #else
            blurOutputFBO.getTextureReference().draw(x, y, width, height);
        #endif
        }
    }
                  
	void setBackgroundColor(ofColor c){
		backgroundColor = c;
	}

	//access direclty please!
	float blurOffset;
	int blurPasses;
	int numBlurOverlays;	
	int blurOverlayGain;	//[0..255]

private:

	void blur( ofFbo * input, ofFbo * output, ofFbo * buffer, ofFbo * buffer2, int iterations, float blurOffset  ){

		if( iterations > 0 ){

			buffer->begin();
			ofClear(backgroundColor);
			buffer->end();

			buffer2->begin();
			ofClear(backgroundColor);
			buffer2->end();

			ofEnableAlphaBlending();
			for (int i = 0; i < iterations; i++) {

				buffer->begin();
				shaderV.begin();
				if (i == 0){ //for first pass, we use input as src; after that, we retro-feed the output of the 1st pass
					shaderV.setUniformTexture( "texture", input->getTextureReference(), 0 );
				}else{
					shaderV.setUniformTexture( "texture", buffer2->getTextureReference(), 1 );
				}
				shaderV.setUniform1f("blurLevel", blurOffset * (i + 1) );
				input->draw(0,0);
				shaderV.end();
				buffer->end();

				buffer2->begin();
				shaderH.begin();
				shaderH.setUniformTexture( "texture", buffer->getTextureReference(), 2 );
				shaderH.setUniform1f("blurLevel", blurOffset * (i + 1) );
				buffer->draw(0,0);
				shaderH.end();
				buffer2->end();
			}
			//draw back into original fbo

			output->begin();
			ofClear(backgroundColor);
			buffer2->draw(0, 0);
			output->end();

		}else{
			output->begin();
			ofClear(backgroundColor);
			input->draw(0, 0);
			output->end();
		}
	}

	ofFbo	cleanImgFBO;
	ofFbo	blurOutputFBO;
	ofFbo	blurTempFBO;
	ofFbo	blurTempFBO2;

	ofShader shaderV;
	ofShader shaderH;

	ofColor backgroundColor;
};