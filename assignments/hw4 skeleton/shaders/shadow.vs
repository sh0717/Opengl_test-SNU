#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightMatrix;
uniform mat4 world;

void main()
{
	gl_Position=lightMatrix*world*vec4(aPos,1.0f);
}