#version 330
out vec4 FragColor;


in vec2 TexCoords;

float INF=2000000000;
// You can change the code whatever you want

#define M_PI       3.14159265358979323846
#define M_PI_2     1.57079632679489661923   // pi/2
#define M_PI_4     0.785398163397448309616  // pi/4
#define M_1_PI     0.318309886183790671538  // 1/pi
#define M_2_PI     0.636619772367581343076  // 2/pi

const int MAX_DEPTH = 4; // maximum bounce
const float Kc=1.0f;
const float Kl=0.0f;
const float Kq=0.02f;


float clamp(float a, float b, float c){
    float x=a;
    if(x<b){
    x=b;
    }
    else if(x>c){
    x=c;
    }
    return x;
}

float GetAttenu(float distance){

    float a=1.0f/(1+distance*Kl+distance*distance*Kq);
    return a;
}

float GetDist(vec3 dist){
float x=dist.x;
float y=dist.y;
float z= dist.z;
float distance=sqrt(x*x+y*y+z*z);
return distance;
}

uniform samplerCube environmentMap;


struct Ray {
    vec3 origin;
    vec3 direction;
};

struct Material {
    // phong shading coefficients
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
    float shininess;

    // reflect / refract
    vec3 R0; // Schlick approximation
    float ior; // index of refration

    // for refractive material
    vec3 extinction_constant;
    float shadow_attenuation_constant;

    

    // 0 : phong
    // 1 : refractive dielectric
    // add more
    int material_type;
};

const int material_type_phong = 0;
const int material_type_refractive = 1;

// Just consider point light
struct Light{
    vec3 position;
    vec3 color;
    bool castShadow;
};

const float LightSample=5;

struct AreaLight{
       vec3 hor;
       vec3 ver;
       vec3 start;
       float length;
};

AreaLight alight=AreaLight(vec3(1.0f,0.0f,0.0f),vec3(0.0f,0.0f,1.0f),vec3(1.0f,5.0f,1.0f),4.0f);




uniform vec3 ambientLightColor;

// hit information
struct HitRecord{
    float t;        // distance to hit point
    vec3 p;         // hit point
    vec3 normal;    // hit point normal
    Material mat;   // hit point material

};

// Geometry
struct Sphere {
    vec3 center;
    float radius;
    Material mat;
};

struct Plane {
    vec3 normal;
    vec3 p0;
    Material mat;
};

struct Box {
    vec3 box_min;
    vec3 box_max;
    Material mat;
};

struct Triangle {
    vec3 v0;
    vec3 v1;
    vec3 v2;
    // we didn't add material to triangle because it requires so many uniform memory when we use mesh...
};


const int mat_phong = 0;
const int mat_refractive = 1;



uniform int ShadowVersion;

uniform Material material_ground;
uniform Material material_box;
uniform Material material_gold;
uniform Material material_dielectric_glass;
uniform Material material_mirror;
uniform Material material_lambert;
uniform Material material_mesh;


Sphere spheres[] = Sphere[](
  Sphere(vec3(1,0.5,-1), 0.5, material_gold),
  Sphere(vec3(-1,0.5,-1), 0.5, material_gold),
  Sphere(vec3(0,0.5,1), 0.5, material_dielectric_glass),
  Sphere(vec3(1,0.5,0), 0.5, material_dielectric_glass)
 
);

Box boxes[] = Box[](
  //Box(vec3(0,0,0), vec3(0.5,1,0.5), dielectric),
  Box(vec3(2,0,-3), vec3(3,1,-2), material_box)
);

Plane groundPlane = Plane(vec3(0,1,0), vec3(0,0,0), material_ground);
Triangle mirrorTriangle = Triangle( vec3(-3,0,0), vec3(0,0,-4), vec3(-1, 4, -2));

Light lights[] = Light[](
    Light(vec3(3,5,3), vec3(1,1,1), true),
    Light(vec3(-3,5,3), vec3(0.2f,0.2f,0.2f), false),
    Light(vec3(-3,5,-3), vec3(0.2f,0.2f,0.2f), false),
    Light(vec3(3,5,-3), vec3(0.0,0.0,0.2f), false)
);

// use this for mesh
/*
layout (std140) uniform mesh_vertices_ubo
{
    vec3 mesh_vertices[500];
};

layout (std140) uniform mesh_tri_indices_ubo
{
    ivec3 mesh_tri_indices[500];
};

uniform int meshTriangleNumber;*/

// Math functions
/* returns a varying number between 0 and 1 */
float rand(vec2 co) {
  return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

float max3 (vec3 v) {
  return max (max (v.x, v.y), v.z);
}

float min3 (vec3 v) {
  return min (min (v.x, v.y), v.z);
}







uniform vec3 cameraPosition;
uniform mat3 cameraToWorldRotMatrix;
uniform float fovY; //set to 45
uniform float H;
uniform float W;






Ray getRay(vec2 uv){
    // TODO:
  vec3 origin=cameraPosition;
  vec2 st=uv*2.0f-1.0f; 
  float scale= tan(fovY*0.5f);
  float imageRatio=W/H;
  float xpos= scale*imageRatio;
  float ypos= scale;
  vec3 dir=vec3(xpos*st.x,ypos*st.y,-1);
  
  dir=cameraToWorldRotMatrix*dir;

  dir=normalize(dir);
  return Ray(origin,dir); 

}


vec3 PointWithT(Ray r, float t){
    return r.origin+t*r.direction;
}

const float bias = 0.0001; // to prevent point too close to surface.



//// intersect는 9장의 geometry query 를 참조한다. 
bool sphereIntersect(Sphere sp, Ray r, inout HitRecord hit){
    // TODO:
    vec3 oc=r.origin-sp.center;
    float a= dot(r.direction,r.direction);
    float b= dot(oc,r.direction);
    float c=dot(oc,oc)-sp.radius*sp.radius;
    
    float deter=b*b-a*c;
    //determinant 가 0 이상이면 만난다 ,0이면 접한다 0밑이면 안만난다. 
    if(deter>0){

        float temp= (-b - sqrt(b*b-a*c)) /a;
        if(bias<temp){
            hit.t=temp;
            hit.p=PointWithT(r,temp);
            hit.normal= (hit.p-sp.center)/sp.radius;
            hit.mat=sp.mat;
            return true;
        }
        temp= (-b + sqrt(b*b-a*c)) /a;
        if(bias<temp){
            hit.t=temp;
            hit.p=PointWithT(r,temp);
            hit.normal= (hit.p-sp.center)/sp.radius;
            hit.mat=sp.mat;
            return true;
       }
    }
    return false;
}

///plane 은 Nt(x-p0)=0;
bool planeIntersect(Plane p, Ray r, inout HitRecord hit){
   vec3 N=normalize(p.normal);
   float c=dot(p.normal,p.p0);
   float tmp = (c-dot(N,r.origin))/(dot(N,r.direction));
   
    if(tmp<bias){
        return false;
    }
    hit.t=tmp;
    hit.p=PointWithT(r,tmp);
    hit.mat=p.mat;
    hit.normal=N;
    return true;
}

vec3 boxnormal(float t, vec3 t0, vec3 t1){
    vec3 neg=vec3(t==t0.x?1:0, t==t0.y?1:0, t==t0.z?1:0);
    vec3 pos= vec3(t==t1.x?1:0, t==t1.y?1:0, t==t1.z?1:0);
    return normalize(pos-neg);
}


bool boxIntersect(Box b, Ray r, inout HitRecord hit){
    // TODO:
    vec3 bmin= b.box_min;
    vec3 bmax= b.box_max;

    float txmin=(bmin.x-r.origin.x)/r.direction.x;
    float txmax=(bmax.x-r.origin.x)/r.direction.x;
    float tymin=(bmin.y-r.origin.y)/r.direction.y;
    float tymax=(bmax.y-r.origin.y)/r.direction.y;
    float tzmin=(bmin.z-r.origin.z)/r.direction.z;
    float tzmax=(bmax.z-r.origin.z)/r.direction.z;
    vec3 t0=vec3(txmin,tymin,tzmin);
    vec3 t1=vec3(txmax,tymax,tzmax);


    vec3 near=vec3(min(txmin,txmax),min(tymin,tymax),min(tzmin,tzmax));
    vec3 far=vec3(max(txmin,txmax),max(tymin,tymax),max(tzmin,tzmax));

    float tmin=max(max(near.x,near.y),near.z);
    float tmax=min(min(far.x,far.y),far.z);

    if(tmin<=tmax){
        float tmp=tmin;
        if(tmp>bias){
            hit.t=tmp;
            hit.p=PointWithT(r,tmp);
            hit.mat=b.mat;
            hit.normal=boxnormal(tmp,t0,t1);
            return true;
        }
        tmp=tmax;
        if(tmp>bias){
            hit.t=tmp;
            hit.p=PointWithT(r,tmp);
            hit.mat=b.mat;
            hit.normal=boxnormal(tmp,t0,t1);
            return true;
        }
    }
    return false;
}

bool triangleIntersect(Triangle tri, Ray r, inout HitRecord hit){
    // TODO:
    vec3 p0=tri.v0;
    vec3 p1=tri.v1;
    vec3 p2=tri.v2;

    mat3 M=mat3(p1-p0,p2-p0,-r.direction);
    vec3 uvt= inverse(M)*(r.origin-p0);
    float u=uvt.x;
    float v=uvt.y;
    float tmp=uvt.z;
    if(0<u&&u<1&&0<v&&v<1&&u+v<1){
           if(tmp>bias){
                hit.t=tmp;
                hit.p=PointWithT(r,tmp);
                hit.normal=normalize(cross(p1-p0,p2-p0));
                hit.mat=material_mirror;
                return true;
               
           }
    }
    return false;
}

float schlick(float cosine, float ref_idx) {
    float r0 = (1 - ref_idx) / (1 + ref_idx);
  r0 = r0 * r0;
  return r0 + (1 - r0) * pow((1 - cosine), 5);
}

vec3 schlick(float cosine, vec3 r0) {
    

    vec3 rtheta=r0+(vec3(1.0f)-r0)*pow((1-cosine),5);
    return rtheta;
   
}




bool Refract(in vec3 v,in  vec3 n, in float nint, out vec3 refracted ){
    vec3 uv= normalize(v);
    float dt=dot(uv,n);
    float discriminant = 1.0 - nint*nint * (1 - dt * dt);
  if (discriminant > 0) {
    refracted = nint * (uv - n * dt) - n * sqrt(discriminant);
    refracted=normalize(refracted);
    return true;
  } else {
    return false;
  }
}


float DecideRefract(in Ray r, in HitRecord hit, out vec3 attenuation ,out Ray scattered){
    Material mt=hit.mat;
    
    vec3 outward_normal;
    vec3 reflected =reflect(r.direction,hit.normal);
    float nint;
    attenuation=vec3(1.0f);
    vec3 refracted;
    float reflect_prob;
    float cosine;

    if(dot(r.direction,hit.normal)>0){
        outward_normal=-hit.normal;
        nint=mt.ior;
        cosine=mt.ior*dot(r.direction,hit.normal);
    }
    else{
        outward_normal=hit.normal;
        nint=1.0f/mt.ior;
        cosine=-dot(r.direction,hit.normal);
    }

    if(Refract(r.direction,outward_normal,nint,refracted)){
        reflect_prob=schlick(cosine,mt.ior);
    }
    else{
        reflect_prob=1.0f;
    }
    
    if(rand(r.direction.xy)<reflect_prob){
        scattered=Ray(hit.p,normalize(reflected));
    }
    else{
        scattered=Ray(hit.p,normalize(refracted));
    }
    return reflect_prob;
}



vec3 GetEnv(Ray r){

    float x=r.direction.x;
    float y=r.direction.y;
    float z=r.direction.z;

    float maxnum=-10;
    maxnum=max(abs(x),maxnum);
    maxnum=max(abs(y),maxnum);
    maxnum=max(abs(z),maxnum);

    x=x/maxnum;
    y=y/maxnum;
    z=z/maxnum;

return texture(environmentMap,vec3(x,y,z)).rgb;

}




bool trace(Ray r, out HitRecord hit){
    // TODO: trace single ray.
    HitRecord tmpHit;
    bool Hit_any=false;
    float closest=1987654321;
    for(int i=0;i<spheres.length();i++){
        if(sphereIntersect(spheres[i],r,tmpHit)){
            if(tmpHit.t<closest){
                Hit_any=true;
                hit=tmpHit;
                
                closest=tmpHit.t;
            }
        }
    }

    for(int i=0;i<boxes.length();i++){
           if(boxIntersect(boxes[i],r,tmpHit)){
             if(tmpHit.t<closest){
               Hit_any=true;
               hit=tmpHit;
               closest=tmpHit.t;
             }
           }
    }
    /// 구,box들 체크 
       
    if(planeIntersect(groundPlane,r,tmpHit)){
       if(tmpHit.t<closest){
            Hit_any=true;
            hit=tmpHit;
            closest=tmpHit.t;
       }
    }

    
    if(triangleIntersect(mirrorTriangle,r,tmpHit)){
        if(tmpHit.t<closest){
               Hit_any=true;
               hit=tmpHit;
               closest=tmpHit.t;
        }
    }
    return Hit_any;
}


vec3 bounceRay(in Ray ray){

    HitRecord hit;
    vec3 col=vec3(0.0f);
    vec3 ambcol=vec3(0.0f);
    vec3 total_col=vec3(0.0f);//반환할 색
    if(trace(ray,hit)){

              if(hit.mat.material_type==1){
                 return vec3(0.0f);
              }
               ambcol=hit.mat.Ka*ambientLightColor;
               total_col+=ambcol;

               vec3 hitP=hit.p;/// hit point 
               vec3 hitN=normalize(hit.normal);
               float shadow=0;/// shadow* color;

               if(ShadowVersion==0){
                   for(int i=0;i<lights.length();i++){
                        Light Li=lights[i];
                        vec3 distvec=Li.position-hit.p;
                        float dist=GetDist(distvec);
                        float attenu=GetAttenu(dist);

                       vec3 lightdir=normalize(Li.position-hitP);
                       float NL=dot(lightdir,hitN);
                       if(NL>0){
                           col+=hit.mat.Kd*NL*Li.color;
                       }
                       vec3 reflectdir=reflect(-lightdir,hitN);
                       vec3 viewdir=normalize(-ray.direction);
                       float RV=dot(reflectdir,viewdir);
                       if(RV>0){
                              col+=hit.mat.Ks*pow(RV,hit.mat.shininess)*Li.color;
                       }

                       col=col*attenu;

                        if(Li.castShadow){
                            vec3 origin= hitP;
                            vec3 direction= normalize(Li.position-hitP);
                            Ray objray= Ray(origin,direction);     
                            HitRecord objhit;

                            if(trace(objray,objhit)){
                                 if(objhit.mat.material_type==1){
                                     Ray rex;
                                     vec3 tmpo;
                                       float tmp=DecideRefract(objray,objhit,tmpo,rex);
                                       float a= clamp(tmp,1-objhit.mat.shadow_attenuation_constant,1);
                                       shadow=1-a;
                                 }
                                 else{
                                    shadow=0;
                                 }
                             }
                            else{
                                shadow=1;
                            }
                            total_col+=(shadow)*col;

                        }
                        else{
                               total_col+=col;
                        }

                   }
              }
              ////여기서부터는 area shadow
               else if(ShadowVersion==1){
                       vec3 diffuse=vec3(0.0f);
                       vec3 specular=vec3(0.0f);

                    vec3 corner=alight.start;
                    vec3 uvec=alight.hor;
                    vec3 vvec=alight.ver;
                    float len=alight.length;
                    for(int i=0;i<6;i++){
                        for(int j=0;j<6;j++){
                                vec3 tmpdif=vec3(0.0f);
                                vec3 tmpspe=vec3(0.0f);
                                vec3 now_corner=corner+i*uvec*len/6+j*vvec*len/6;
                                float x=rand(vec2(i+5/j,j*j/5));
                                float y=rand(vec2(j+7/i,i*j/5));
                                vec3 LPoint=now_corner+uvec*len*x/6+vvec*len*y/6;//샘플링 된 라이트 포인트 
                                vec3 distvec=LPoint-hit.p;
                                float dist=GetDist(distvec);
                                float attenu=GetAttenu(dist);

                                vec3 lightdir=normalize(LPoint-hitP);
                                float NL=dot(lightdir,hitN);
                                if(NL>0){
                                    tmpdif=hit.mat.Kd*NL*attenu;
                                }
                                vec3 reflectdir=reflect(-lightdir,hitN);
                                vec3 viewdir=normalize(-ray.direction);
                                float RV=dot(reflectdir,viewdir);
                               if(RV>0){
                                  tmpspe=hit.mat.Ks*pow(RV,hit.mat.shininess)*attenu;
                                }

                                vec3 origin= hitP;
                                vec3 direction= normalize(LPoint-hitP);
                                Ray objray= Ray(origin,direction);     
                                HitRecord objhit;

                               if(trace(objray,objhit)){
                                    if(objhit.mat.material_type==1){
                                        Ray rex;
                                         vec3 tmpo;
                                        float tmp=DecideRefract(objray,objhit,tmpo,rex);
                                       float a= clamp(tmp,1-objhit.mat.shadow_attenuation_constant,1);
                                       shadow=1-a;
                                     }
                                     else{
                                         shadow=0;
                                     }
                                }
                                else{
                                    shadow=1;
                                }
                                                        
                                    tmpdif*=shadow;
                                    tmpspe*=shadow;
                                    diffuse+=tmpdif;
                                    specular+=tmpspe;
                          }
                           
                    
                     }
                   
                    diffuse/=36;
                    specular/=36;
                    total_col+=(diffuse+specular)*vec3(1.0f);

                   for(int i=0;i<lights.length();i++){

                       Light Li=lights[i];
                       if(Li.castShadow==true)
                         continue;
                      
                       ///하얀빛은 넘어가고
                       vec3 distvec=Li.position-hit.p;
                       float dist=GetDist(distvec);
                       float attenu=GetAttenu(dist);

                      vec3 lightdir=normalize(Li.position-hitP);
                      float NL=dot(lightdir,hitN);
                      if(NL>0){
                          col+=hit.mat.Kd*NL*Li.color;
                      }
                      vec3 reflectdir=reflect(-lightdir,hitN);
                      vec3 viewdir=normalize(-ray.direction);
                      float RV=dot(reflectdir,viewdir);
                      if(RV>0){
                             col+=hit.mat.Ks*pow(RV,hit.mat.shininess)*Li.color;
                      }

                    
                     total_col+=attenu*col;
                  }

               }
    }   



    
   else{
       total_col=GetEnv(ray);
     }
     return total_col;
}






vec3 castRay(in Ray ray){
    // TODO: trace ray in iterative way.

    HitRecord hit;
    vec3 col=vec3(0.0f);
    vec3 ambcol=vec3(0.0f);

    vec3 total_col=vec3(0.0f);

    /// 카메라에서 나온 레이가 히트하면 그 히트 포인트를 분석 
       if(trace(ray,hit)){
              
              
             col=bounceRay(ray);
             //col=vec3(0.1f);
             total_col+=col;


            vec3 total_attenu=vec3(1.0f);

            Ray reflectray=ray;
            HitRecord reflecthit=hit;

            vec3 addition_col=vec3(0.0f);
            


              for(int bounce=0;bounce<MAX_DEPTH;bounce++){
                        float cosine=dot(-reflectray.direction,reflecthit.normal);
                   ///cosine for schlick;
                   if(reflecthit.mat.material_type==0){
                       /// 반사만 고려
                       
                       vec3 new_dir=reflect(reflectray.direction,reflecthit.normal);
                       vec3 new_origin=reflecthit.p;
                       reflectray=Ray(new_origin,new_dir);

                   
                       vec3 Rnum=schlick(cosine,reflecthit.mat.R0);
                       total_attenu*=Rnum;

                      if(trace(reflectray,reflecthit)){
                           vec3 add_col;
                           add_col=bounceRay(reflectray);
                           addition_col+=total_attenu*add_col;
                      }
                      else{
                        addition_col+=total_attenu*GetEnv(reflectray);
                        break;
                      }
                   }
                   else if(reflecthit.mat.material_type==1){
                        // 반사와 굴절 동시에 고려 
                        Ray scattered;
                        vec3 attenu;
                        DecideRefract(reflectray,reflecthit,attenu,scattered);
                        reflectray=scattered;
                        total_attenu*=attenu;

                      if(trace(reflectray,reflecthit)){
                           if(reflecthit.mat.material_type==1){
                               float dist=GetDist(reflectray.origin-reflecthit.p);
                                vec3 e=reflecthit.mat.extinction_constant;
                                float x=pow(10,-1*dist*e.x);
                                float y=pow(10,-1*dist*e.y);
                                float z=pow(10,-1*dist*e.z);
                            total_attenu*=vec3(x,y,z);
                           }
                           vec3 add_col;
                           add_col=bounceRay(reflectray);
                           //addition_col+=total_attenu*add_col;
                           addition_col+=(total_attenu*add_col);

                           
                      }
                      else{
                        addition_col+=total_attenu*GetEnv(reflectray);
                            break;
                      }
                   }
             }
             total_col+=addition_col;

       }
       else{
            total_col=GetEnv(ray);
       }

    return total_col;
}






void main()
{
    // TODO:
    
   const float nsamples =16;
   
   float x=TexCoords.x;
   float y=TexCoords.y;

   vec3 color=vec3(0.0f);

    //Ray r=getRay(vec2(x,y));
    //color= color+castRay(r);
    Ray r=getRay(vec2(x,y));

    float u,v;
   for(int i=0;i<nsamples;++i){
        
      u=x+rand(color.xy+i)/W;
      v=y+rand(color.xz+i)/H;
      r=getRay(vec2(u,v));
      color+=castRay(r);

   }


   FragColor = vec4(color.x/nsamples,color.y/nsamples,color.z/nsamples, 1.0);

 
}
