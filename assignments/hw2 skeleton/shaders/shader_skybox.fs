#version 330 core
// TODO: define in/out and uniform variables.

out vec4 FragColor;
in vec3 TexCoords;

uniform samplerCube skybox ;
uniform samplerCube nightbox;

uniform float time;

void main()
{   
    // mix two texture
    FragColor=texture(skybox,TexCoords)*(1-time)+texture(nightbox,TexCoords)*time;

}
