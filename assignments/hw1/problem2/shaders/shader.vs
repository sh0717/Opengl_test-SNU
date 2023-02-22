#version 330 core


layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aCol;

uniform mat4 SizeMat;
uniform mat4 TransMat;
uniform mat4 RotMat;
uniform vec4 GravVec;

out vec4 tmp_col;
void main()
{    
    tmp_col=vec4(aCol,1);

   vec4 tmp_pos=vec4(aPos,1.0f);

   tmp_pos=SizeMat*tmp_pos;
   tmp_pos-=GravVec;
   tmp_pos=RotMat*tmp_pos;
   tmp_pos+=GravVec;
   tmp_pos=TransMat*tmp_pos;
   
    gl_Position= tmp_pos;
}