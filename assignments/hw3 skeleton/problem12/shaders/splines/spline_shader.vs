#version 330 core
layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec3 aColor;

out VS_OUT {
     vec3 color;
 } vs_out;

void main()
{
	//vs_out.color = aColor;
	//gl_Position = projection * view * model * vec4(aPos, 1.0f);
	//gl_Position = vec4(aPos, 1.0f);
	vs_out.color=vec3(1.0f);
	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
