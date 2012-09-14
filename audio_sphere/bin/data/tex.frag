#version 120

varying vec4 Color;

void main(void)
{
    if (Color.a < 0.03)
        discard;

    gl_FragColor = Color;
    
}