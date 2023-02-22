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
#include <algorithm>
using namespace std;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, DirectionalLight* sun);

bool isWindowed = true;
bool isKeyboardDone[1024] = { 0 };

// setting
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;
const float planeSize = 15.f;

// camera
Camera camera(glm::vec3(0.0f, 0.5f, 3.0f));
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


////

float cameraNearPlane = 0.1f;
float cameraFarPlane = 500.0f;


std::vector<float> shadowCascadeLevels{ cameraFarPlane / 50.0f, cameraFarPlane / 25.0f, cameraFarPlane / 10.0f, cameraFarPlane / 2.0f };

std::vector<glm::mat4> getLightSpaceMatrices();
glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane);
std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& projview);



/// CSM ���� 
DirectionalLight sun(30.0f, 30.0f, glm::vec3(0.8f));

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

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader program
    // ------------------------------------
    Shader lightingShader("../shaders/shader_lighting.vs", "../shaders/shader_lighting.fs"); // you can name your shader files however you like
    Shader shadowShader("../shaders/shadow.vs", "../shaders/shadow.fs");
    Shader skyboxShader("../shaders/shader_skybox.vs", "../shaders/shader_skybox.fs");
    Shader csmShader("../shaders/csm.vs", "../shaders/csm.fs", "../shaders/csm.gs");

    // define models
    // There can be three types 
    // (1) diffuse, specular, normal : brickCubeModel
    // (2) diffuse, normal only : boulderModel
    // (3) diffuse only : grassGroundModel
    Model brickCubeModel = Model("../resources/brickcube/brickcube.obj");
    brickCubeModel.diffuse = new Texture("../resources/brickcube/brickcube_d.png");
    brickCubeModel.specular = new Texture("../resources/brickcube/brickcube_s.png");
    brickCubeModel.normal = new Texture("../resources/brickcube/brickcube_n.png");

    Model boulderModel("../resources/boulder/boulder.obj");
    boulderModel.diffuse = new Texture("../resources/boulder/boulder_d.png");
    boulderModel.normal = new Texture("../resources/boulder/boulder_n.png");

    Model grassGroundModel = Model("../resources/plane.obj");
    //grassGroundModel.diffuse = new Texture("../resources/grass_ground.jpg");
    grassGroundModel.diffuse = new Texture("../resources/grass_ground.jpg");
    grassGroundModel.ignoreShadow = true;

    
    // TODO: Add more models (barrels, fire extinguisher) and YOUR own model
    Model barrelModel("../resources/barrel/barrel.obj");
    barrelModel.diffuse = new Texture("../resources/barrel/barrel_d.png");
    barrelModel.specular = new Texture("../resources/barrel/barrel_s.png");
    barrelModel.normal = new Texture("../resources/barrel/barrel_n.png");

    Model fireExtModel("../resources/FireExt/FireExt.obj");
    fireExtModel.diffuse = new Texture("../resources/FireExt/FireExt_d.jpg");
    fireExtModel.specular = new Texture("../resources/FireExt/FireExt_s.jpg");
    fireExtModel.normal = new Texture("../resources/FireExt/FireExt_n.jpg");

    Model TreeModel("../resources/tree/tree_base_obj.obj");
    TreeModel.diffuse = new Texture("../resources/tree/treebase_DefaultMaterial_BaseColor.png");
    TreeModel.normal = new Texture("../resources/tree/treebase_DefaultMaterial_Normal.png");
    TreeModel.specular = new Texture("../resources/tree/treebase_DefaultMaterial_Metallic.png");

    // Add entities to scene.
    // you can change the position/orientation.
    Scene scene;
    scene.addEntity(new Entity(&brickCubeModel, glm::mat4(1.0)));
    scene.addEntity(new Entity(&brickCubeModel, glm::translate(glm::vec3(-3.5f, 0.0f, -2.0f)) * glm::rotate(glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f))));
    scene.addEntity(new Entity(&brickCubeModel, glm::translate(glm::vec3(1.0f, 0.5f, -3.0f)) * glm::rotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f))));
    scene.addEntity(new Entity(&barrelModel, glm::vec3(2.5f, 0.0f, -2.0f), 0, 0, 0, 0.1f));

    glm::mat4 planeWorldTransform = glm::mat4(1.0f);
    planeWorldTransform = glm::scale(planeWorldTransform, glm::vec3(planeSize));
    planeWorldTransform = glm::translate(glm::vec3(0.0f, -0.5f, 0.0f)) * planeWorldTransform;
    scene.addEntity(new Entity(&grassGroundModel, planeWorldTransform));

    scene.addEntity(new Entity(&fireExtModel, glm::vec3(2,-1,0), 0.0f, 180.0f, 0.0f, 0.002f));
    scene.addEntity(new Entity(&boulderModel, glm::vec3(-5, 0, 2), 0.0f, 180.0f, 0.0f, 0.1));
    
    
    scene.addEntity(new Entity(&TreeModel, glm::vec3(2.5f, 0.0f, 4.0f), 0, 0, 0, 1.0f));
    
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

   /* glm::vec3 test = glm::vec3(0.8f);
    
    cout << test.x << endl;*/

    lightingShader.use();
    lightingShader.setInt("material.diffuseSampler", 0);
    lightingShader.setInt("material.specularSampler", 1);
    lightingShader.setInt("material.normalSampler", 2);
    lightingShader.setInt("depthMapSampler", 3);
    lightingShader.setInt("shadowMap", 4);
    lightingShader.setFloat("material.shininess", 64.f);    // set shininess to constant value.

    skyboxShader.use();
    skyboxShader.setInt("skyboxTexture1", 0);


  



    unsigned int matricesUBO;
    glGenBuffers(1, &matricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * 16, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, matricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);




    float oldTime = 0;
    while (!glfwWindowShouldClose(window))// render loop
    {
        float currentTime = glfwGetTime();
        float dt = currentTime - oldTime;
        deltaTime = dt;
        oldTime = currentTime;

        // input
        processInput(window, &sun);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 lightProjection = sun.getProjectionMatrix();
        glm::mat4 lightView = sun.getViewMatrix(camera.Position);
        glm::mat4 lightSpace = lightProjection * lightView;
        shadowShader.use();
        shadowShader.setMat4("lightMatrix", lightSpace);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depth.depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        

        for (auto it = scene.entities.begin(); it != scene.entities.end(); it++) {
            for (auto iter = it->second.begin(); iter != it->second.end(); iter++) {
                (*iter)->Draw(shadowShader);
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

        /// <summary>
        /// /csm �϶�?
        /// </summary>
        /// <returns></returns>
        /// 
        const auto lightMatrices = getLightSpaceMatrices();
        if (SHADOW_MODE == CSM) {

            
            glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);


            for (size_t i = 0; i < lightMatrices.size(); ++i)
            {
                glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &lightMatrices[i]);
            }
            glBindBuffer(GL_UNIFORM_BUFFER, 0);




            csmShader.use();
            glBindFramebuffer(GL_FRAMEBUFFER, CSMdepth.depthMapFBO);
            glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
            glClear(GL_DEPTH_BUFFER_BIT);

            for (auto it = scene.entities.begin(); it != scene.entities.end(); it++) {
                for (auto iter = it->second.begin(); iter != it->second.end(); iter++) {
                    (*iter)->Draw(csmShader);
                }
            }

            
        }





        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.use();
        lightingShader.setInt("shadowMode", SHADOW_MODE);
        //cout << SHADOW_MODE << endl;
        if (!useLight) {
            lightingShader.setFloat("useLighting", 0.0f);
          
        }
        else {
            lightingShader.setFloat("useLighting", 1.0f);
            
        }


        if (useNormalMap == false) {
            lightingShader.setFloat("useNormalMap", 0);
            lightingShader.setFloat("useNormalMap1", 0);
        }
        else {
            lightingShader.setFloat("useNormalMap", 1.0f);
            lightingShader.setFloat("useNormalMap1", 1.0f);
        }



        lightingShader.setVec3("viewPos", camera.Position);
       
        lightingShader.setVec3("light.dir", sun.lightDir);
        lightingShader.setVec3("light.color", sun.lightColor);

        lightingShader.setMat4("lightMatrix", lightSpace);


        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depth.ID);

       

        if (SHADOW_MODE == CSM) {
            lightingShader.setFloat("farPlane", cameraFarPlane);
            lightingShader.setInt("cascadeCount", shadowCascadeLevels.size());
            for (int i = 0; i < shadowCascadeLevels.size(); i++) {
                lightingShader.setFloat("cascadePlaneDistances[" + std::to_string(i) + "]", shadowCascadeLevels[i]);
            }
            for (int i = 0; i < shadowCascadeLevels.size(); i++) {
                lightingShader.setMat4("lightSpaceMatrices[" + std::to_string(i) + "]", lightMatrices[i]);
            }

            lightingShader.setMat4("view1", view);

            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D_ARRAY, CSMdepth.ID);
        }

        if (useShadow == true) {
            lightingShader.setFloat("useShadow", 1.0f);
        }
        else {
            lightingShader.setFloat("useShadow", 0.0f);
        }


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
       
        
        
        // use skybox Shader
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

bool pressed1; bool pressed2; bool pressed3;

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, DirectionalLight* sun)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);


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




////��Ȯ�ϰ� ����Ʈ ��Ʈ���� ���ϱ� 



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