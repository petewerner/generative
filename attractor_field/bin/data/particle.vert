#version 120

uniform sampler2DRect partloc;
uniform sampler2DRect colortex;
uniform vec2 screensize;
uniform vec2 colsz;
uniform float widthOffset;
varying float alpha;
varying vec2 ploc;



void main(void)
{
    //x y coords of part in particle location buffer
    vec2 loc = gl_Vertex.xy;
    vec4 pix = texture2DRect(partloc, loc);
    ploc = pix.xy;
    //colors are done seperately so strip them out
    vec4 final_vertloc = vec4(pix.x+widthOffset, pix.y, 0.0, 1.0);
    gl_Position = gl_ModelViewProjectionMatrix * final_vertloc;
    
    vec4 color = texture2DRect(colortex, loc);
    gl_FrontColor = vec4(color.r, color.g, color.b, color.a);
    
}


