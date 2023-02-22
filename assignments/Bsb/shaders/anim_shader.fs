#version 460 core
out vec4 FragColor;

in vec2 TexCoords;
uniform int setBlack;


uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height;
const float shininess=64.0f;
//
uniform sampler2D depthMapSampler;



struct Light{
vec3 dir;
vec3 color;

};


uniform vec3 viewPos;//
uniform Light light;//
uniform float useSpecularMap;

in vec3 Normal;//
in vec3 FragPos;//

in vec4 FragPosLightSpace;

vec2 poissonDisk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

float random(vec3 seed, int i){
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

float ShadowCalc(vec4 FragPosLightSpace){
    vec3 projCoord = FragPosLightSpace.xyz/FragPosLightSpace.w;
    projCoord=projCoord*0.5+0.5;
    
    float closestDepth=texture(depthMapSampler,projCoord.xy).r;
        
        vec3 normal=normalize(Normal);
        vec3 lightDir=-light.dir;
      float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.005);
     float currentDepth = projCoord.z;

     if(currentDepth>1.0f){
        return 0.0f;
     }
     float shadow=0.0f;
    for(int i=0;i<4;i++){
       int index = int(16.0*random(floor(FragPos.xyz*1000.0), i))%16;
       float pcfDepth=texture(depthMapSampler,projCoord.xy + poissonDisk[index]/700.0).r;
      float tmp=currentDepth - bias > pcfDepth  ? 1.0 : 0.0;;
      shadow+=0.2f*tmp;
      
    }
    return shadow;
}

float NormalCalc(vec4 FragPosLightSpace){
    vec3 projCoord = FragPosLightSpace.xyz/FragPosLightSpace.w;
    projCoord=projCoord*0.5+0.5;
    
    float closestDepth=texture(depthMapSampler,projCoord.xy).r;
        
     //   vec3 normal=normalize(Normal);
     //   vec3 lightDir=-light.dir;
     // float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.05);
     float currentDepth = projCoord.z;
     float bias=0.0f;
    float shadow=0.0f;
    shadow = currentDepth-bias > closestDepth  ? 1.0 : 0.0;
     return shadow;

}

void main()
{    
    if(setBlack==100){
        FragColor=vec4(1.0f,1.0f,0.3f,1.0f);
    }
    else{
    vec3 fcolor=texture(texture_diffuse1,TexCoords).rgb;

    vec3 Is=light.color;
    vec3 Id=light.color;
    vec3 Ia=0.3f*light.color;

    vec3 ambient= Ia*texture(texture_diffuse1,TexCoords).rgb;
    vec3 diffuse;
    vec3 specular;

    vec3 Nvec;
    vec3 Rvec;
    vec3 Lvec=-light.dir;
    Lvec=normalize(Lvec);
    vec3 Vvec=normalize(viewPos-FragPos);
    Nvec=normalize(Normal);
    Rvec=reflect(-Lvec,Nvec);

    float N_L=max(0.0f,dot(Nvec,Lvec));
    float R_V=pow(max(dot(Vvec,Rvec),0.0f),shininess);

    diffuse=Id*texture(texture_diffuse1,TexCoords).rgb*N_L;//I_diff Á¤ÇÏ°í
    float k_s;
    if(useSpecularMap<0.5f){
     k_s=0;
    }
    else{
     k_s=texture(texture_specular1, TexCoords).r;
    }
     specular=k_s*Is*R_V;

     float shadow=0;
      //shadow=NormalCalc(FragPosLightSpace);
  
     vec3 result= ambient+(1-shadow)*diffuse+(1-shadow)*specular;
        //vec3 result=vec3(shadow,shadow,shadow);

       FragColor=vec4(result,1.0f);
        //FragColor = texture(depthMapSampler,TexCoords);
    }
}