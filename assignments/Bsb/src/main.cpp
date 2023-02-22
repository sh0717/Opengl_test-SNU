#define GLM_ENABLE_EXPERIMENTAL
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "opengl_utils.h"
#include "geometry_primitives.h"
#include <iostream>
#include <vector>
#include "camera.h"
#include "texture.h"
#include "texture_cube.h"
#include "model.h"
#include "mesh.h"
#include "scene.h"
#include "math_utils.h"
#include "light.h"
#include <filesystem>
#include <algorithm>

#include "animator.h"
#include "Amodel.h"
#include "smodel.h"
#include "text_renderer.h"
using namespace std;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, DirectionalLight* sun);

bool isWindowed = true;
bool isKeyboardDone[1024] = { 0 };




// setting
const unsigned int SCR_WIDTH = 1600;

const unsigned int SCR_HEIGHT = 1200;
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;
const float planeSize = 150.0f;

// camera
Camera camera(glm::vec3(1.0f,1.8f, 0.5f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

bool useNormalMap = true;
bool useSpecular = false;
bool useShadow = true;
bool useLight = true;

int SHADOW_MODE = 3;
enum {
    NORMAL=1,
    PCF=2,
    CSM=3
};

enum {
    FASTBALL = 1,
    CURVE = 2,
    SLIDER=3,
    OFFSPEED=4
};



constexpr float Fast_Speed = 155.0f;
constexpr float Curve_Speed = 115.0f;
constexpr float Slider_Speed = 142.0f;
constexpr float Offspeed_Speed = 140.0f;
constexpr float Fast_Land = 17.4 * 3600 / Fast_Speed / 1000;
constexpr float Curve_Land = 17.4 * 3600 / Curve_Speed / 1000;
constexpr float Slider_Land = 17.4 * 3600 / Slider_Speed / 1000;
constexpr float Offspeed_Land = 17.4 * 3600 / Offspeed_Speed / 1000;
float RPM[] = { 0,2400,-2600,2700,2300 };
float LAND[] = { 0,Fast_Land, Curve_Land ,Slider_Land ,Offspeed_Land };
int Ball_Dist[] = { 1,1,1,1,1,2,2,3,4,3 };

int Ball_Species = FASTBALL;
bool PitchStartTrigger = true;
bool onPitch = false;
float BallT = 0.0f;
float deliverTime = 0.2f;

const glm::vec3 BallStartPos = glm::vec3(0.7f, 2.0f, 17.4f);
glm::vec3 BallTmp1;
glm::vec3 BallTmp2;
glm::vec3 BallEndPos=glm::vec3(-0.4f,0.2f,0.0f);

glm::vec3 GetBallPos(float t) {
    glm::mat4x3 tmpMat = glm::mat4x3(BallStartPos, BallTmp1, BallTmp2, BallEndPos);
    tmpMat = tmpMat * BezierMat;
    glm::vec4 TimeVec = glm::vec4(t * t * t, t * t, t, 1);
    return tmpMat * TimeVec;
}

void SetBallPos(int BallSpecies) {
    if (BallSpecies == FASTBALL) {
        BallTmp1 = (2 / 3.0f) * BallStartPos + (1 / 3.0f) * BallEndPos;
        BallTmp2 = (1 / 3.0f) * BallStartPos + (2 / 3.0f) * BallEndPos;
        BallTmp1 += glm::vec3(0.0f, 0.1, 0.0f);
        BallTmp2 += glm::vec3(0.0f, 0.05f, 0.0f);
        return;
    }
    if (BallSpecies == CURVE) {
        BallTmp1 = (2 / 3.0f) * BallStartPos + (1 / 3.0f) * BallEndPos;
        BallTmp2 = (1 / 3.0f) * BallStartPos + (2 / 3.0f) * BallEndPos;
        BallTmp1.y = BallStartPos.y -0.01f;
        BallTmp2.y = BallStartPos.y - 0.2f;
        return;
    }
    if (BallSpecies == SLIDER) {
        
        BallTmp1 = (2 / 3.0f) * BallStartPos + (1 / 3.0f) * BallEndPos;
        BallTmp2 = (1 / 3.0f) * BallStartPos + (2 / 3.0f) * BallEndPos;
        BallTmp1.x += 0.4f;
        BallTmp2.x+=0.8f;
        return;
    }
    if (BallSpecies == OFFSPEED) {
        BallTmp1 = (2 / 3.0f) * BallStartPos + (1 / 3.0f) * BallEndPos;
        BallTmp2 = (1 / 3.0f) * BallStartPos + (2 / 3.0f) * BallEndPos;
        BallTmp1.y += 0.3f;
        BallTmp1.x -= 0.1f;
        BallTmp2.y += 0.8;
        BallTmp2.y -= 0.7f;
    }

}

bool Hitted = false;
bool Swing = false;
bool HutSwing = false;

float HitTime = 0.0f;

float LaunchAngle;
float LaunchVel;
glm::vec3 LaunchStart;
glm::vec3 HitDir;


void MakeHit(float t) {
   
    LaunchStart = GetBallPos(t);
    LaunchAngle = getRandomValueBetween(10.0f, 50.0f);
    LaunchVel = getRandomValueBetween(100.0f, 170.0f);
    LaunchVel = LaunchVel * 1000 / 3600;
    float dir = (0.95 - t)/0.05*60.0f;
    HitDir = glm::vec3(sin(glm::radians(dir)), 0.0f, cos(glm::radians(dir)));
    camera.Yaw -= dir;
    camera.Pitch += LaunchAngle-10.0f;
    camera.updateCameraVectors();
}


glm::vec3 GetHitBallPos(float HitT) {
    float Vx = LaunchVel * cos(glm::radians(LaunchAngle));
    float xPos = Vx * HitT * HitDir.x;
    float zPos = Vx * HitT * HitDir.z;

    float x_l = Vx * HitT;
    float yPos = x_l * tan(glm::radians(LaunchAngle)) - 9.8 * x_l * x_l / 2 / (Vx) / Vx;

    return glm::vec3(xPos, yPos, zPos)+LaunchStart;
}

bool isStrike(glm::vec3 pos) {
       
    float x = pos.x;
    float y = pos.y;
    return -0.216 <= x && x <= 0.216 && 0.4 <= y && y <= 1.1;
}

int HITMODE = 1;

enum {HIT=1,STRIKE=2,BALL=3};





int action = 0;
float actionTime = 1.0f;
////
glm::vec3 CatcherPos = glm::vec3(0.0f, 0.8f, -0.5f);
const float CatcherRange = 0.2f;

int CatchEvent = 0;
float CatchTimer = 0.0f;

bool Catched() {
    float xpos1 = BallEndPos.x;
    float ypos1 = BallEndPos.y;
    float xpos2 = CatcherPos.x;
    float ypos2 = CatcherPos.y;
    return abs(xpos1 - xpos2) < CatcherRange && abs(ypos1 - ypos2) < CatcherRange;

};
////



std::vector<glm::vec3> Fielders = {glm::vec3(-14.0f,0.0f,25.0f),glm::vec3(14.0f,0.0f,25.0f),glm::vec3(-5.0f,0.0f,35.0f),glm::vec3(5.0f,0.0f,35.0f),glm::vec3(-0.0f,0.0f,75.0f),glm::vec3(-34.0f,0.0f,65.0f),glm::vec3(+34.0f,0.0f,65.0f) };




float cameraNearPlane = 0.1f;
float cameraFarPlane = 500.0f;


std::vector<float> shadowCascadeLevels{ cameraFarPlane / 50.0f, cameraFarPlane / 25.0f, cameraFarPlane / 10.0f, cameraFarPlane / 2.0f };

std::vector<glm::mat4> getLightSpaceMatrices();
glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane);
std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& projview);



/// CSM 변수 
DirectionalLight sun(30.0f, 90.0f, glm::vec3(1.0f));



float pitchTerm = 0.0f;


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    glEnable(GL_DEPTH_TEST);

    Shader animShader("../shaders/anim_shader.vs", "../shaders/anim_shader.fs");
   
    // ------------------------------------
    Shader lightingShader("../shaders/shader_lighting.vs", "../shaders/shader_lighting.fs"); // you can name your shader files however you like
    Shader shadowShader("../shaders/shadow.vs", "../shaders/shadow.fs");
    Shader skyboxShader("../shaders/shader_skybox.vs", "../shaders/shader_skybox.fs");
    Shader csmShader("../shaders/csm.vs", "../shaders/csm.fs", "../shaders/csm.gs");
    Shader simpleShader("../shaders/simple_shader.vs", "../shaders/simple_shader.fs");
    Shader picShader("../shaders/pic_shader.vs", "../shaders/pic_shader.fs");
    /// <summary>
    /// /모델칸
    /// </summary>
    /// <returns></returns>
    AModel alienModel("../resources/objects/vampire/dancing_vampire.dae");
    Animation alienAnimation("../resources/objects/vampire/dancing_vampire.dae",&alienModel);
    Animator animator(&alienAnimation);

    /*AModel alienModel2 ("../resources/objects/alien/Tree_frog.dae");
    Animation alienAnimation2("../resources/objects/alien/Tree_frog.dae", &alienModel2);
    Animator animator2(&alienAnimation2);*/

    AModel pitcherModel("../resources/objects/sexo/sexo.dae");
   
    Animation pitcherAnimation("../resources/objects/sexo/sexo.dae", &pitcherModel);
    Animator pitcherAnimator(&pitcherAnimation);
    
    AModel stadiumModel("../resources/stadium/stadium.obj");
    AModel gloveModel("../resources/glove/glove.obj");


   

   
    
    // TODO: Add more models (barrels, fire extinguisher) and YOUR own model
   

    Model BallModel("../resources/baseball/ball.obj");
    BallModel.diffuse = new Texture("../resources/baseball/ball_diffuse.jpg");
    
    Model BaseModel("../resources/base/base.obj");
    BaseModel.diffuse=new Texture("../resources/base/base.jpg");
   
    Model HomeBaseModel("../resources/homebase/homebase.obj");
    HomeBaseModel.diffuse = new Texture("../resources/homebase/homebase.jpg");





    // Add entities to scene.
    // you can change the position/orientation.
    Scene scene;
   /* scene.addEntity(new Entity(&brickCubeModel, glm::mat4(1.0)));
    scene.addEntity(new Entity(&brickCubeModel, glm::translate(glm::vec3(-3.5f, 0.0f, -2.0f)) * glm::rotate(glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f))));
    scene.addEntity(new Entity(&brickCubeModel, glm::translate(glm::vec3(1.0f, 0.5f, -3.0f)) * glm::rotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f))));
    scene.addEntity(new Entity(&barrelModel, glm::vec3(2.5f, 0.0f, -2.0f), 0, 0, 0, 0.1f));*/
    Entity* SampleBall = new Entity(&BallModel, glm::vec3(0.0, 0.0f, 10.3f), 0.0f, 0.0f, 0.0f, 0.007f);
    Entity* Ball = new Entity(&BallModel, glm::vec3(0.0, 2.0f, 2.0f), 0.0f, 0.0f, 0.0f, 0.007f);


    //scene.addEntity(new Entity(&grassGroundModel, planeWorldTransform));

   

   
    scene.addEntity(new Entity(&BaseModel, glm::vec3(19.395f, 0.0f, 19.395), -90.0f, 0.0f, 0.0f, 0.01f));
    scene.addEntity(new Entity(&BaseModel, glm::vec3(-19.395f, 0.0f, 19.395), -90.0f, 0.0f, 0.0f, 0.01f));
    scene.addEntity(new Entity(&BaseModel, glm::vec3(0.0f, 0.0f, 38.79), -90.0f, 0.0f, 0.0f, 0.01f));
    scene.addEntity(new Entity(&HomeBaseModel, glm::vec3(0.0f, 0.0f, -1.0f), -90.0f, 0.0f, 0.0f, 0.03f));

   // scene.addEntity(new Entity(&boulderModel, glm::vec3(-5, 0, 2), 0.0f, 180.0f, 0.0f, 0.1));
    
    
    //scene.addEntity(new Entity(&TreeModel, glm::vec3(2.5f, 0.0f, 4.0f), 0, 0, 0, 1.0f));
    
    // add your model's entity here!
    



    // define depth texture
    DepthMapTexture depth = DepthMapTexture(SHADOW_WIDTH, SHADOW_HEIGHT);
    DepthTexture CSMdepth = DepthTexture(SHADOW_WIDTH, SHADOW_HEIGHT);

    // skybox
    std::vector<std::string> faces
    {
        "../resources/skybox/right.jpg",
        "../resources/skybox/left.jpg",
        "../resources/skybox/top.jpg",
        "../resources/skybox/bottom.jpg",
        "../resources/skybox/front.jpg",
        "../resources/skybox/back.jpg"
    };
    CubemapTexture skyboxTexture = CubemapTexture(faces);
    unsigned int VAOskybox, VBOskybox;
    getPositionVAO(skybox_positions, sizeof(skybox_positions), VAOskybox, VBOskybox);

 

    lightingShader.use();
    lightingShader.setInt("material.diffuseSampler", 0);
    lightingShader.setInt("material.specularSampler", 1);
    lightingShader.setInt("material.normalSampler", 2);
    lightingShader.setInt("depthMapSampler", 3);
    lightingShader.setInt("shadowMap", 4);
    lightingShader.setFloat("material.shininess", 64.f);    // set shininess to constant value.

    animShader.setInt("depthMapSampler", 4);


    skyboxShader.use();
    skyboxShader.setInt("skyboxTexture1", 0);


  



    unsigned int matricesUBO;
    glGenBuffers(1, &matricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * 16, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, matricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    ///스트라이크 존
    float lines[] = {
    -0.216f,0.4f, 0.0f,
     -0.216f, 1.1f, 0.0f,
     0.216f,  1.1f, 0.0f,
     0.216f,0.4f,0.0f
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lines), lines, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);






    float lines2[] = {
        0,0.01f,0.0f,
        19.395f,0.01f,19.395f,
        0.0f,0.01f,38.79,
        -19.395f,0.01f,19.395f
    };
    
    unsigned int VBO2, VAO2;
    glGenVertexArrays(1, &VAO2);
    glGenBuffers(1, &VBO2);
    glBindVertexArray(VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lines2), lines2, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float vertices[] = {
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left 
    };
    unsigned int indices[] = {  // note that we start from 0!
       0, 1, 3,  // first Triangle
       1, 2, 3   // second Triangle
    };
    unsigned int VBO3, VAO3, EBO3;
    glGenVertexArrays(1, &VAO3);
    glGenBuffers(1, &VBO3);
    glGenBuffers(1, &EBO3);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO3);

    glBindBuffer(GL_ARRAY_BUFFER, VBO3);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO3);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    TextRenderer* Text;
    //Text = new TextRenderer(SCR_WIDTH, SCR_HEIGHT);
    ///Text->Load("../fonts/OCRAEXT.TTF", 24);

    float oldTime = 0;
    while (!glfwWindowShouldClose(window))// render loop
    {

        float currentTime = glfwGetTime();
        float dt = currentTime - oldTime;
        deltaTime = dt;
        oldTime = currentTime;
        
        // input
        processInput(window, &sun);

        glClearColor(1.0f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 200.0f);
        glm::mat4 view = camera.GetViewMatrix(); 


        lightingShader.use();
        lightingShader.setInt("shadowMode", SHADOW_MODE);
        ////cout << SHADOW_MODE << endl;
        /*if (!useLight) {
          lightingShader.setFloat("useLighting", 0.0f);
        }*/
       
         lightingShader.setFloat("useLighting", 1.0f);  
         lightingShader.setFloat("useNormalMap", 0.0f);
         lightingShader.setFloat("useNormalMap1", 1.0f);
         lightingShader.setFloat("useShadow", 0.0f);
         lightingShader.setVec3("viewPos", camera.Position);
         lightingShader.setMat4("projection", projection);
         lightingShader.setMat4("view", view);

        //if (useNormalMap == false) {
        //    lightingShader.setFloat("useNormalMap", 0);
        //    lightingShader.setFloat("useNormalMap1", 0);
        //}
        //else {
        //    lightingShader.setFloat("useNormalMap", 1.0f);
        //    lightingShader.setFloat("useNormalMap1", 1.0f);
        //}



        
       
        lightingShader.setVec3("light.dir", sun.lightDir);
        lightingShader.setVec3("light.color", sun.lightColor);

       //lightingShader.setMat4("lightMatrix", lightSpace);


       

       /* glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depth.ID);*/

       

        //if (SHADOW_MODE == CSM) {
        //    lightingShader.setFloat("farPlane", cameraFarPlane);
        //    lightingShader.setInt("cascadeCount", shadowCascadeLevels.size());
        //    for (int i = 0; i < shadowCascadeLevels.size(); i++) {
        //        lightingShader.setFloat("cascadePlaneDistances[" + std::to_string(i) + "]", shadowCascadeLevels[i]);
        //    }
        //    for (int i = 0; i < shadowCascadeLevels.size()+1; i++) {
        //        lightingShader.setMat4("lightSpaceMatrices[" + std::to_string(i) + "]", lightMatrices[i]);
        //    }

        //    lightingShader.setMat4("view1", view);

        //    glActiveTexture(GL_TEXTURE4);
        //    glBindTexture(GL_TEXTURE_2D_ARRAY, CSMdepth.ID);
        //}

        //if (useShadow == true) {
        //    lightingShader.setFloat("useShadow", 1.0f);
        //}
        //else {
        //    lightingShader.setFloat("useShadow", 0.0f);
        //}

       
    

        ///투구 렌더링
        if (PitchStartTrigger) {
            PitchStartTrigger = false;
            onPitch = true;
            BallT = 0.0f;
            deliverTime = 0.5f;
            pitchTerm = 2.0f;
            SetBallPos(Ball_Species);
        }
        if (deliverTime > 0.0f) {
            deliverTime -= deltaTime;
        }
        /*else if (deliverTime < 0.0f) {
            deliverTime = 0.0f;
        }*/





        //// 애니메이션 가능할까?




        if (Swing&&Hitted==false) {
            if ( BallT > 0.9f&&isStrike(BallEndPos)) {
                Hitted = true;
                MakeHit(BallT);
                HitTime = 0.0f;

                action = HIT;
                actionTime = 1.0f;
            }
            else {
                HutSwing = true;
            }

            Swing = false;
        }

        if (onPitch&&deliverTime<=0) {
            if (Hitted == false) {
                BallT += deltaTime / LAND[Ball_Species];

                glm::vec3 BallPos = GetBallPos(BallT);
                //glm::vec3 BallPos = BallStartPos;
                float angle = 0.01 * RPM[Ball_Species] / 60.0f * BallT * LAND[Ball_Species] * 360.0f;

                glm::mat4 tmp = glm::translate(glm::mat4(1.0f), BallPos);
                tmp = glm::rotate(tmp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                tmp = glm::rotate(tmp, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
                tmp = glm::scale(tmp, glm::vec3(0.01f));
                SampleBall->setModelMatrix(tmp);
                SampleBall->Draw(lightingShader);
            }
            else {
                HitTime += deltaTime;
                glm::vec3 BallPos=GetHitBallPos(HitTime);
             
                glm::mat4 tmp = glm::translate(glm::mat4(1.0f), BallPos);
                tmp = glm::scale(tmp, glm::vec3(0.01f));
                SampleBall->setModelMatrix(tmp);
                SampleBall->Draw(lightingShader);
            }
        }


        //공이 지나갔을 때
        if (Hitted==false&&BallT > 1.0f) {
            PitchStartTrigger = false;
            onPitch = false;
            BallT = 0.0f;
            if (isStrike(BallEndPos)) {
                action = STRIKE;
            }
            else if(HutSwing) {
                action = STRIKE;
            }
            else {
                action = BALL;
            }
            actionTime = 1.0f;
            HutSwing = false;
            if (CatchEvent == 0) {
                bool flag = Catched();
                if (flag) {
                    CatchEvent = 1;
                }
                else {
                    CatchEvent = 2;
                }
                CatchTimer = 0.4f;

                CatcherPos= glm::vec3(0.0f, 0.5f, -0.5f);
            }
            
        }
        ///히트가 될 때
        if (Hitted == true && GetHitBallPos(HitTime).y<0.0f) {
            Hitted = false;
            HitTime = 0.0f;
            PitchStartTrigger = false;
            onPitch = false;
            BallT = 0.0f;

            if (HITMODE == 1) {
                camera.SetFirst();
            }
            else {
                camera.SetFirst_2();
            }

            CatcherPos = glm::vec3(0.0f, 0.5f, -0.5f);
        }


        actionTime -= 3*deltaTime;
        if (actionTime < 0) {
            actionTime = 0.0f;
        }

        CatchTimer -= deltaTime;
        if (CatchTimer < 0.0f) {
            CatchEvent = 0;
            
        }





       /* stringstream sss1;
        if (actionTime > 0.0f) {
            if (action == STRIKE) {
                sss1 << "STRIKE";
            }
            else if (action == BALL) {
                sss1 << "BALL";
            }
            else if (action == HIT) {
                sss1 << "HIT";
            }
        }
        Text->RenderText(sss1.str(), 1.0f, 10.0f, 0.8f, glm::vec3(0.9, 0.9f, 0.9f));*/


       /* glm::mat4 tmp = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f,0.5f,0.0f));
        tmp = glm::rotate(tmp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        tmp = glm::rotate(tmp, glm::radians(40.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        tmp = glm::scale(tmp, glm::vec3(0.01f));
        SampleBall->setModelMatrix(tmp);
        SampleBall->Draw(lightingShader);*/



        for (auto it = scene.entities.begin(); it != scene.entities.end(); it++) {
        
            for (auto iter = it->second.begin(); iter != it->second.end(); iter++) {

                if ((*iter)->model->specular == nullptr) {
                    lightingShader.setFloat("useSpecularMap", 0.0f);
                }
                else{
                    lightingShader.setFloat("useSpecularMap", 1.0f);
                }



                if ((*iter)->model->normal == nullptr && (useNormalMap == true)) {
                    lightingShader.setFloat("useNormalMap", 0);
                    lightingShader.setFloat("useNormalMap1", 0);
                    (*iter)->Draw(lightingShader);
                    lightingShader.setFloat("useNormalMap", 1.0f);
                    lightingShader.setFloat("useNormalMap1", 1.0f);
                }

                else {
                    (*iter)->Draw(lightingShader);
                }
               
            }
        }

        /// <summary>
        /// //
        /// </summary>
        /// <returns></returns>
        animator.UpdateAnimation(deltaTime);
        //animator2.UpdateAnimation(deltaTime * 3.0f);


        if (onPitch && pitchTerm >= 0) {
            pitcherAnimator.UpdateAnimation(2.5f * deltaTime);
            pitchTerm -= deltaTime;
        }

        else {
            pitcherAnimator.m_CurrentTime = 0.0f;
        }///애니메이션 돌리기
        /// <summary>
        /// /
        /// </summary>
        /// <returns></returns>

        glm::mat4 lightProjection = sun.getProjectionMatrix();
        glm::mat4 lightView = sun.getViewMatrix(camera.Position);
        glm::mat4 lightSpace = lightProjection * lightView;
        shadowShader.use();
        shadowShader.setMat4("lightMatrix", lightSpace);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depth.depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        //그림자 세팅 구간

        shadowShader.setInt("AniMode", 1);
        auto transforms = animator.GetFinalBoneMatrices();


        for (int i = 0; i < transforms.size(); i++) {
            shadowShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
        }

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -5.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        shadowShader.setMat4("model", model);
        alienModel.Draw(shadowShader);


        for (int i = 0; i < 7; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, Fielders[i]);
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            shadowShader.setMat4("model", model);
            alienModel.Draw(shadowShader);
        }


        //transforms = animator2.GetFinalBoneMatrices();
        //for (int i = 0; i < transforms.size(); i++) {
        //    shadowShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
        //}
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(10.0f, 0.1f, 10.0f));
        model = glm::scale(model, glm::vec3(0.1, 0.1f, 0.1f));
        shadowShader.setMat4("model", model);
        //alienModel2.Draw(shadowShader);


        transforms = pitcherAnimator.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); i++) {
            shadowShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
        }
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 18.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
        shadowShader.setMat4("model", model);
        pitcherModel.Draw(shadowShader);


        shadowShader.setInt("AniMode", 2);


        model = glm::mat4(1.0f);

        //model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(-16.9f, 98.0f, 337.0f));
        model = glm::scale(model, glm::vec3(158.0f, 100.0f, 158.0f));
        shadowShader.setMat4("model", model);

        stadiumModel.Draw(shadowShader);


        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);


        //for (auto it = scene.entities.begin(); it != scene.entities.end(); it++) {
        //    for (auto iter = it->second.begin(); iter != it->second.end(); iter++) {
        //        (*iter)->Draw(shadowShader);
        //    }
        //}

        animShader.use();

        

        animShader.setMat4("lightMatrix", lightSpace);
        animShader.setVec3("viewPos", camera.Position);
        animShader.setVec3("light.dir", sun.lightDir);
        animShader.setVec3("light.color", sun.lightColor);

        animShader.setInt("AniMode", 1);
        animShader.setMat4("projection", projection);
        animShader.setMat4("view", view);
       
        /*glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, depth.ID);
        glActiveTexture(GL_TEXTURE0);*/

        transforms = animator.GetFinalBoneMatrices();


        for (int i = 0; i < transforms.size(); i++) {
            animShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
        }
        
        animShader.setFloat("useSpecularMap", 1.0f);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -5.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        animShader.setMat4("model", model);
        alienModel.Draw(animShader);

        for (int i = 0; i < 7; i++) {
            model = glm::mat4(1.0f);
            model=glm::translate(model, Fielders[i]);
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            animShader.setMat4("model", model);
            alienModel.Draw(animShader);
        }
       


      // transforms = animator2.GetFinalBoneMatrices();
       // for (int i = 0; i < transforms.size(); i++) {
       //     animShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
       // }
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(10.0f, 0.1f, 10.0f));
        model = glm::scale(model, glm::vec3(0.1, 0.1f, 0.1f));
      
        animShader.setMat4("model", model);
        //alienModel2.Draw(animShader);

        animShader.setFloat("useSpecularMap", 0.0f);


        //animShader.setInt("setBlack", 100);
        transforms = pitcherAnimator.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); i++) {
            animShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
        }
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 18.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
        animShader.setMat4("model", model);
        pitcherModel.Draw(animShader);

        animShader.setInt("setBlack", 1);





        animShader.setInt("AniMode", 2);
    /////이제부터 정적인 걸로

       /*for (int i = 0; i <100; i++) {
            animShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", glm::mat4(1.0f));
        }*/
        model = glm::mat4(1.0f);
   
        //model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(-16.9f, 98.0f, 337.0f));
        model = glm::scale(model, glm::vec3(158.0f, 100.0f, 158.0f));
        animShader.setMat4("model", model);

        stadiumModel.Draw(animShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, CatcherPos);
        model = glm::translate(model, glm::vec3(0.07f,0.0f,0.0f));

        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        animShader.setMat4("model", model);

        gloveModel.Draw(animShader);



        simpleShader.use();
        simpleShader.setMat4("projection", projection);
        simpleShader.setMat4("view", view);
        glm::mat4 lineModel = glm::mat4(1.0f);
        simpleShader.setMat4("model", lineModel);
        glBindVertexArray(VAO);
        glDrawArrays(GL_LINE_LOOP, 0, 4);

        
        

        simpleShader.setMat4("model", glm::mat4(1.0f));
        glBindVertexArray(VAO2);
        glDrawArrays(GL_LINE_LOOP, 0, 4);

        
        picShader.use();



        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.7f, 0.7f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
       
        picShader.setMat4("model", model);
        picShader.setInt("Catch", CatchEvent);
        glBindVertexArray(VAO3);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);



        // use skybox Shader
        // 
        // 

        skyboxShader.use();
        glDepthFunc(GL_LEQUAL);
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);

        // render a skybox
        glBindVertexArray(VAOskybox);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture.textureID);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

bool pressed1; bool pressed2; bool pressed3; bool pressedL;

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, DirectionalLight* sun)
{   
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime*5);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime*5);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime*5);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime*5);

    ////포수놀이 
    float Catchvel = deltaTime * 0.6;
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
        CatcherPos += (glm::vec3(0.0f, 1.0f, 0.0f)* Catchvel);
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
        CatcherPos += (glm::vec3(1.0f, 0.0f, 0.0f)* Catchvel);
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        CatcherPos -= (glm::vec3(0.0f, 1.0f, 0.0f)* Catchvel);
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        CatcherPos -= (glm::vec3(1.0f, 0.0f, 0.0f)* Catchvel);



    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        if (onPitch) {
            
        }
        else {
            PitchStartTrigger = true;
            int a = getRandomValueBetween(1.0, 10.0);
            Ball_Species = Ball_Dist[a];
            
            float num1 = getRandomValueBetween(-0.3f, 0.3f);
            float num2 = getRandomValueBetween(0.0f, 1.3f);
            BallEndPos = glm::vec3(num1, num2, 0.0f);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        if (Hitted == false&&onPitch) {
            Swing = true;
        }
    }
    bool curent_pressedL = glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;
    if (curent_pressedL && !pressedL) {
        if (HITMODE == 1) {
            HITMODE = 2;
        }
        else {
            HITMODE = 1;
        }
    }
    pressedL = curent_pressedL;

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        camera.SetFirst();
    }
 

    float t = 20.0f * deltaTime;
    
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        sun->processKeyboard(-3*t, 0);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        sun->processKeyboard(3*t, 0);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        sun->processKeyboard(0, t);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        sun->processKeyboard(0, -t);
    }

    bool curent_pressed1 = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
    if (curent_pressed1 && !pressed1) {
        useNormalMap = !useNormalMap;
    }
    pressed1 = curent_pressed1;


    bool curent_pressed2 = glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;
    if (curent_pressed2 && !pressed2) {
        useShadow=!useShadow;
    }
    pressed2 = curent_pressed2;

    bool curent_pressed3 = glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS;
    if (curent_pressed3 && !pressed3) {
        useLight = !useLight;
    }
    pressed3 = curent_pressed3;

    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
        SHADOW_MODE = NORMAL;
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
        SHADOW_MODE = PCF;
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
        SHADOW_MODE = 3;
        
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}



std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& projview)
{
    const auto inv = glm::inverse(projview);

    std::vector<glm::vec4> frustumCorners;
    for (unsigned int x = 0; x < 2; ++x)
    {
        for (unsigned int y = 0; y < 2; ++y)
        {
            for (unsigned int z = 0; z < 2; ++z)
            {
                const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }

    return frustumCorners;
}




////정확하게 라이트 매트릭스 구하기 



std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
    return getFrustumCornersWorldSpace(proj * view);
}



glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane)
{
    const auto proj = glm::perspective(
        glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, nearPlane,
        farPlane);
    const auto corners = getFrustumCornersWorldSpace(proj, camera.GetViewMatrix());

    glm::vec3 center = glm::vec3(0, 0, 0);
    for (const auto& v : corners)
    {
        center += glm::vec3(v);
    }
    center /= corners.size();

    const auto lightView = glm::lookAt(center - sun.lightDir, center, glm::vec3(0.0f, 1.0f, 0.0f));

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::min();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::min();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::min();
    for (const auto& v : corners)
    {
        const auto trf = lightView * v;
        minX = std::min(minX, trf.x);
        maxX = std::max(maxX, trf.x);
        minY = std::min(minY, trf.y);
        maxY = std::max(maxY, trf.y);
        minZ = std::min(minZ, trf.z);
        maxZ = std::max(maxZ, trf.z);
    }

    // Tune this parameter according to the scene
    constexpr float zMult = 10.0f;
    if (minZ < 0)
    {
        minZ *= zMult;
    }
    else
    {
        minZ /= zMult;
    }
    if (maxZ < 0)
    {
        maxZ /= zMult;
    }
    else
    {
        maxZ *= zMult;
    }

    const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);

    return lightProjection * lightView;
}

std::vector<glm::mat4> getLightSpaceMatrices()
{
    std::vector<glm::mat4> ret;
    for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
    {
        if (i == 0)
        {
            ret.push_back(getLightSpaceMatrix(cameraNearPlane, shadowCascadeLevels[i]));
        }
        else if (i < shadowCascadeLevels.size())
        {
            ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], shadowCascadeLevels[i]));
        }
        else
        {
            ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], cameraFarPlane));
        }
    }
    return ret;
}