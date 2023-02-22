#version 410
#extension GL_ARB_tessellation_shader : enable
#define saturate(x) clamp(x, 0, 1)

const vec3 diffuseColor = vec3(0.64, 0.64, 0.64);
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const vec3 lightDirection = vec3(1,2,1);

out vec4 FragColor;
in vec3 gsViewNormal;
in vec3 gsColor;

void main()
{   
     vec3 diffuse = gsColor;
     vec3 normal = gsViewNormal;
     float diffuseFactor = saturate(dot(normalize(lightDirection), normal));
     FragColor = vec4(diffuseFactor * diffuse,1.0f);
    //FragColor = vec4(0.1f,0.5f,0.5f, 1.0);
}