#version 120
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect u_prevDisp;  // recive the previus position texture
uniform sampler2DRect u_newDisp; //displacement data
uniform float u_decayRate;
uniform float u_axisloc; //location on axis to apply new displacement

void main(void){

    vec2 st = gl_TexCoord[0].st;
     
    if (floor(st.t) == u_axisloc) {
        vec4 col = texture2DRect(u_newDisp, st);
        col /= 100;
        gl_FragColor = vec4(col.x, col.y, col.z, 1.0);
    } else {
        vec4 prevcol = texture2DRect(u_prevDisp, st);
        gl_FragColor = prevcol * u_decayRate;
    }
    
}

