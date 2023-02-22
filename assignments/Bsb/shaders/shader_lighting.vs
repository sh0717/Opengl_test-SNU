#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

out mat3 TBN;
out vec4 FragPosLightSpace;


uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightMatrix;

uniform float useNormalMap;

void main()
{
	

	// on-off by key 1 (useNormalMap).
    // if model does not have a normal map, this should be always 0.
    // if useNormalMap is 0, we use a geometric normal as a surface normal.
    // if useNormalMap is 1, we use a geometric normal altered by normal map as a surface normal.





		TexCoord = aTexCoord;
		Normal = normalize(mat3(transpose(inverse(world))) * aNormal);
		FragPos = vec3(world * vec4(aPos, 1.0));
	if (useNormalMap > 0.5){
		vec3 N=Normal;
		vec3 T=normalize(mat3(transpose(inverse(world))) * aTangent);
		 T = normalize(T - dot(T, N) * N);
		 vec3 B=cross(N,T);
		 TBN=mat3(T,B,N);
	}

	FragPosLightSpace = lightMatrix * vec4(FragPos, 1.0);
	gl_Position = projection * view * world * vec4(aPos, 1.0f);
}
