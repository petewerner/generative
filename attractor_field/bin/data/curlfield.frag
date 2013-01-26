#version 120

uniform sampler2DRect tex;
uniform float curl_scale;

void main(void)
{
    float dt = 1.0;
    vec2 loc = gl_TexCoord[0].st;
    float x0 = texture2DRect(tex, loc + vec2(0.0, dt)).r;
    float x1 = texture2DRect(tex, loc + vec2(0.0, -dt)).r;
    float x2 = texture2DRect(tex, loc + vec2(dt, 0.0)).r;
    float x3 = texture2DRect(tex, loc + vec2(-dt, 0.0)).r;
    
    float curl_x = (x0 - x1)/2.0 * curl_scale;
    float curl_y = (x2 - x3)/2.0 * curl_scale;
//    gl_FragColor = vec4(-curl_x, 0.0, curl_y, 1.0);
    gl_FragColor = vec4(-curl_x, curl_y, 0.0, 1.0);
//    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}

//works, try w/curl_scale = 0.5
void a_main(void)
{
    vec2 loc = gl_TexCoord[0].st;
    float x0 = texture2DRect(tex, loc).r;
    float x1 = texture2DRect(tex, loc + vec2(1, 1)).r;
    float x2 = texture2DRect(tex, loc + vec2(0, 1)).r;
    
    float curl_x = (x0 - x1)/2.0 * curl_scale;
    float curl_y = (x0 - x2)/2.0 * curl_scale;
    gl_FragColor = vec4(-curl_x, 0.0, curl_y, 1.0);
}