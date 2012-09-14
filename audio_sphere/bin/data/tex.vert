#version 120
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect u_OffMap;
uniform float u_fboRes;

varying vec4 Color;

void main()
{

    gl_TexCoord[0] = gl_MultiTexCoord0;
        
    vec2 base_loc = gl_TexCoord[0].st;
    
    vec2 loc = base_loc * u_fboRes; //scale to tex coords of fbo
    vec4 offvec = texture2DRect(u_OffMap, loc);
    float df = -offvec.y * 1.25; //the y coord is the displacement of the eq val
                //we scale by 1.25 arbitrarily. 2.0 is good for quieter songs
    
    vec4 newpos = vec4(gl_Normal * df * 100.0, 0.0) + gl_Vertex;

    //if the alph gets too low, will be discarded in frag shader
    Color = vec4(df+0.3, 0.0, 1.0 - df, df);
    gl_Position = gl_ModelViewProjectionMatrix * newpos;

//    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    

}

