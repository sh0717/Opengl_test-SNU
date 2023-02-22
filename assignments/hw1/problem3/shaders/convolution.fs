#version 330 core

out vec4 FragColor;
in vec2 textureCoords;

uniform sampler2D screenTexture;

const float offset_x=1.0f/9000.0f;
const float offset_y=1.0f/6000.0f;

vec2 offsets[9]=vec2[](
vec2(-offset_x,offset_y), vec2(0.0f,offset_y), vec2(offset_x,offset_y),
vec2(-offset_x,0.0f),vec2(0.0f,0.0f),vec2(offset_x,0.0f),
vec2(-offset_x,-offset_y),vec2(0.0f,-offset_y),vec2(offset_x,-offset_y)
);
float kernel[9]=float[](
-1,-1,-1,
-1,9,-1,
-1,-1,-1
);

void main()
{
  
    vec3 sampleTex[9];
   for(int i = 0; i < 9; i++)
   {
       sampleTex[i] = vec3(texture(screenTexture, textureCoords.st + offsets[i]));
   }
   vec3 col=vec3(0.0);
   for(int i = 0; i < 9; i++){
       col+=sampleTex[i]* kernel[i];
   }
   FragColor = vec4(col, 1.0f);
}