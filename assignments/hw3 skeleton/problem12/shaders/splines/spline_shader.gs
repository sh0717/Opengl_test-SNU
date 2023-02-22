#version 330 core
#define N 20
layout (lines_adjacency) in;
layout (line_strip, max_vertices = N) out;

 in VS_OUT {
     vec3 color;
 } gs_in[];

out vec3 fColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 B;

void BezierLine(float t)
{
    vec4 T = vec4(t*t*t, t*t, t, 1);
   
    mat4x3 G;
    for(int i=0; i<4; i++){
        vec4 wp = model * gl_in[i].gl_Position;
        G[i][0] = wp.x;
        G[i][1] = wp.y;
        G[i][2] = wp.z;
    }
    fColor = vec3(t,1,1-t);//gs_in[0].color;
    vec4 worldV = vec4(G * B * T, 1.0f);
    gl_Position = projection * view * worldV;
    EmitVertex();
}

void main() {    
    for(int i=0; i<N; i++){
        BezierLine(float(i)/(N-1));
    }
    EndPrimitive();
}