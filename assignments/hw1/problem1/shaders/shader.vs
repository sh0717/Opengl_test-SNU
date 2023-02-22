#version 330 core
layout (location = 0) in vec3 aPos;

   uniform vec3 move_vec;
   uniform mat3 rotate_mat;
   uniform vec3 grav_cent;

void main()
{
   
   vec3 tmp=aPos;
	tmp-=grav_cent;

   tmp=rotate_mat*tmp;
	tmp+=grav_cent;
	tmp+=move_vec;
   gl_Position=vec4(tmp,1.0f);

}
