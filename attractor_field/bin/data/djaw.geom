#version 120
#extension GL_EXT_geometry_shader4 : enable

uniform float a;
uniform float b;
uniform float c;
uniform float d;
uniform vec4 color;
uniform float zoom;
uniform int nverts;

void main ()
{
    float x = gl_PositionIn[0].x;
    float y = gl_PositionIn[0].y;

    for (int j = 0; j < nverts; j++)
    {
        float xn = sin(a * y) - cos(b * x);
        float yn = sin(c * x) - cos(d * y);
        gl_FrontColor = vec4(x, 0.0, 0.0, 1.0);
        x = xn;
        y = yn;
        gl_Position = vec4(x * zoom, y * zoom, 0, 1);
        EmitVertex();
    }
}

