#version 330 core
// declare input and output

in  vec4 tmp_col;
out vec4 fragColor;

uniform vec4 Sex;


void main()
{
    fragColor=tmp_col;
}