#version 460 core
out vec4 FragColor;

struct Material {
    sampler2D diffuseSampler;
    sampler2D specularSampler;
    sampler2D normalSampler;
    float shininess;
}; 

uniform sampler2D depthMapSampler;
uniform sampler2DArray shadowMap;

uniform float farPlane;


struct Light {
    vec3 dir;
    vec3 color; // this is I_d (I_s = I_d, I_a = 0.3 * I_d)
};

//layout (std140, binding = 0) uniform LightSpaceMatrices
//{
//    mat4 lightSpaceMatrices[16];
//};
uniform mat4 lightSpaceMatrices[16];
uniform float cascadePlaneDistances[16];
uniform int cascadeCount;


uniform vec3 viewPos;
uniform Material material;
uniform Light light;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

in mat3 TBN;

in vec4 FragPosLightSpace;

uniform int shadowMode;
uniform float useNormalMap1;
uniform float useSpecularMap;
uniform float useShadow;
uniform float useLighting;
uniform mat4 view1;

////그림자 계산 
//포아송?

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

///////
float NormalCalc(vec4 FragPosLightSpace){
    vec3 projCoord = FragPosLightSpace.xyz/FragPosLightSpace.w;
    projCoord=projCoord*0.5+0.5;
    
    float closestDepth=texture(depthMapSampler,projCoord.xy).r;
        
        vec3 normal=normalize(Normal);
        vec3 lightDir=-light.dir;
      float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
     float currentDepth = projCoord.z;

    float shadow=0.0f;
    shadow = currentDepth-bias > closestDepth  ? 1.0 : 0.0;
     return shadow;

}

float ShadowCalc(vec4 FragPosLightSpace){
    vec3 projCoord = FragPosLightSpace.xyz/FragPosLightSpace.w;
    projCoord=projCoord*0.5+0.5;
    
    float closestDepth=texture(depthMapSampler,projCoord.xy).r;
        
        vec3 normal=normalize(Normal);
        vec3 lightDir=-light.dir;
      float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
     float currentDepth = projCoord.z;

     if(currentDepth>1.0f){
        return 0.0f;
     }
     //float shadow = currentDepth > closestDepth  ? 1.0 : 0.0; 기본 


     float shadow=0.0f;

     //vec2 texelSize=1.0/textureSize(depthMapSampler,0);
     //for(int x=-1;x<=1;++x){
     //   for(int y=-1;y<=1;++y){
     //      float pcfDepth = texture(depthMapSampler, projCoord.xy + vec2(x, y) * texelSize).r; 
     //                 shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;  
     //   }
     //}

     //shadow/=9.0f;
     //if( projCoord.z>1.0){
     //   shadow=0.0;
     //}

     //// 딱 그부분의 뎁스맵이랑 비교하는게 아니라 주위거랑 비교7해서 평균내버리기
     for(int i=0;i<4;i++){
        int index = int(16.0*random(floor(FragPos.xyz*1000.0), i))%16;
        float pcfDepth=texture(depthMapSampler,projCoord.xy + poissonDisk[index]/700.0).r;
       float tmp=currentDepth - bias > pcfDepth  ? 1.0 : 0.0;;
       shadow+=0.2f*tmp;
       
     }
     return shadow;
}

float CSMcalc(vec3 fragPosWorldSpace){
    ///먼저 어떤 레이어로 할지 결정부투 하고 
     vec4 fragPosViewSpace = view1 * vec4(fragPosWorldSpace, 1.0);
     float depthValue = abs(fragPosViewSpace.z);
     int layer = -1;
        for (int i = 0; i < cascadeCount; ++i)
        {
            if (depthValue < cascadePlaneDistances[i])
            {
                layer = i;
                break;
            }
        }
        if (layer == -1)
        {
            layer = cascadeCount;
        }
        ///레이어 정해졌으면  일반적으로 했던거랑 비슷하게 간다

       vec4 fragPosLightSpace=lightSpaceMatrices[layer]*vec4(FragPos,1.0f);
       vec3 projCoords=fragPosLightSpace.xyz/fragPosLightSpace.w;
       projCoords=projCoords*0.5+0.5;
       float currentDepth=projCoords.z;

       //if(currentDepth>1.0f){return 0.0f;}

       ///bias 를 레이어에 따라 다르게 한다 
       vec3 normal=normalize(Normal);
              vec3 lightDir=-light.dir;
            float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
        float biasModifier=0.5f;
        if(layer ==cascadeCount){
            bias= bias/(farPlane*biasModifier);
        }
        else{
            bias=bias/(cascadePlaneDistances[layer]*biasModifier);
        }


        float shadow=0.0f;
        for(int i=0;i<4;i++){
            int index = int(16.0*random(floor(FragPos.xyz*1000.0), i))%16;
            float pcfDepth=texture(shadowMap,vec3(projCoords.xy + poissonDisk[index]/700.0,layer)).r;
            float tmp=(currentDepth - bias) > pcfDepth  ? 1.0 : 0.0;;
            shadow+=0.2f*tmp;
           
         }
         return shadow;


         //float shadow = 0.0;
         //vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
         //for(int x = -1; x <= 1; ++x)
         //{
         //    for(int y = -1; y <= 1; ++y)
         //    {
         //        float pcfDepth = texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
         //        shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;        
         //    }    
         //}
         //shadow /= 9.0;
             
         //return shadow;
}





void main()
 
{   
   
	vec3 fcolor = texture(material.diffuseSampler, TexCoord).rgb;
    
   

    // on-off by key 3 (useLighting). 
    // if useLighting is 0, return diffuse value without considering any lighting.(DO NOT CHANGE)
	if (useLighting < 0.5f){

        FragColor = vec4(fcolor, 1.0); 
        return; 
    }
    

    vec3 Is=light.color;
    vec3 Id=light.color;
    vec3 Ia=0.3f*light.color;

    vec3 ambient=Ia*texture(material.diffuseSampler,TexCoord).rgb;//I_ambient 정하고
    vec3 diffuse;
    vec3 specular;
    
    vec3 Nvec;
    vec3 Rvec;

    vec3 Lvec=-light.dir;
    Lvec=normalize(Lvec);
    vec3 Vvec=normalize(viewPos-FragPos);
    //L,V 벡터 정하기


    if(useNormalMap1<0.5f){
        Nvec=normalize(Normal);
    }
    else{
        vec3 normal= texture(material.normalSampler,TexCoord).rgb;
        normal = normalize(normal * 2.0 - 1.0); 
        Nvec=normalize(TBN*normal);
    }
    //n vec 정하기 

    Rvec=reflect(-Lvec,Nvec);

    float N_L=max(0.0f,dot(Nvec,Lvec));
    float R_V=pow(max(dot(Vvec,Rvec),0.0f),material.shininess);
    diffuse=Id*texture(material.diffuseSampler,TexCoord).rgb*N_L;//I_diff 정하고
    float k_s;

    if(useSpecularMap<0.5f){
        k_s=0;
    }
    else{
        k_s=texture(material.specularSampler, TexCoord).r;
    }
    specular=k_s*Is*R_V;

    

    float shadow=0;



    if(useShadow>0.5f){
      if(shadowMode==1){
         shadow=NormalCalc(FragPosLightSpace);
      }
      if(shadowMode==2){
            shadow=ShadowCalc(FragPosLightSpace);
      }
      if(shadowMode==3){
        shadow=CSMcalc(FragPos);
      }
      
    }
  
    vec3 result = ambient + (1-shadow)*diffuse + (1-shadow)*specular ;

    FragColor=vec4(result,1.0f);

}