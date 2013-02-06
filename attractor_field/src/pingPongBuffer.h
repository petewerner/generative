//
//  pingPongBuffer.h
//  attractor_field9
//
//  Created by Peter Werner on 10/01/13.
//
//

#ifndef attractor_field9_pingPongBuffer_h
#define attractor_field9_pingPongBuffer_h

struct pingPongBuffer {
public:
    void allocate( int _width, int _height, int _internalformat = GL_RGBA, float _dissipation = 1.0f){
        // Allocate
        for(int i = 0; i < 2; i++){
            FBOs[i].allocate(_width,_height, _internalformat );
        }
        
        // Clean
        clear();
        
        // Set everything to 0
        flag = 0;
        swap();
        flag = 0;
    }
    
    void swap(){
        src = &(FBOs[(flag)%2]);
        dst = &(FBOs[++(flag)%2]);
    }
    
    void clear(){
        for(int i = 0; i < 2; i++){
            FBOs[i].begin();
            ofClear(0,255);
            FBOs[i].end();
        }
    }
    
    ofFbo& operator[]( int n ){ return FBOs[n];}
    
    ofFbo   *src;       // Source       ->  Ping
    ofFbo   *dst;       // Destination  ->  Pong
private:
    ofFbo   FBOs[2];    // Real addresses of ping/pong FBO´s
    int     flag;       // Integer for making a quick swap
};



#endif
