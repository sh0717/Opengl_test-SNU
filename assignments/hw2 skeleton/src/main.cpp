#define GLM_ENABLE_EXPERIMENTAL
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "geometry_primitives.h"
#include <iostream>
#include <sstream>
#include <vector>
#include "camera.h"
#include "texture.h"
#include "texture_cube.h"
#include "math_utils.h"
#include <string>
#include "text_renderer.h"
#include <filesystem>
using namespace std;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

bool isWindowed = true;
bool isKeyboardProcessed[1024] = {0};

// setting
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(1.0f, 2.0f, 3.0f));


float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

bool Rflag;
bool Eflag;
int Ecnt=100;

char last_press = 'E';
// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastTime = 0.0f;

float realTime = 12.0f;

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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "yoosoheon_hw2", NULL, NULL);
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

    // define normal shader and skybox shader.
    Shader shader("../shaders/shader.vs", "../shaders/shader.fs"); // you can name your shader files however you like
    Shader skyboxShader("../shaders/shader_skybox.vs","../shaders/shader_skybox.fs");





    // TODO : define required VAOs(textured cube, skybox, quad)
    // data are defined in geometry_primitives.h

    unsigned int boxVAO, boxVBO;
    glGenVertexArrays(1, &boxVAO);
    glGenBuffers(1, &boxVBO);
    glBindVertexArray(boxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_positions_textures), cube_positions_textures, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    

    unsigned int QVAO, QVBO;
    unsigned int QEBO;
    glGenVertexArrays(1, &QVAO);
    glGenBuffers(1, &QVBO);
    glGenBuffers(1, &QEBO);
    glBindVertexArray(QVAO);
    glBindBuffer(GL_ARRAY_BUFFER, QVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_positions_textures), quad_positions_textures, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, QEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);



    unsigned int skyVAO, skyVBO;
    glGenVertexArrays(1, &skyVAO);
    glGenBuffers(1, &skyVBO);
    glBindVertexArray(skyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_positions), &skybox_positions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


    TextRenderer* Text;
    Text = new TextRenderer(SCR_WIDTH, SCR_HEIGHT);
    Text->Load("../fonts/OCRAEXT.TTF", 18);


    // world space positions of our cubes
    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  1.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f,  2.2f, -2.5f),
        glm::vec3(-3.8f,  2.0f, -12.3f),
        glm::vec3( 2.4f,  1.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  1.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    const int n_grass = 1000;
    float grassGroundSize = 20;
    glm::vec3 grassPositions[n_grass];

    // positions of our grasses
    for(int i=0; i<n_grass; i++){
        float s = grassGroundSize/2;
        float x = getRandomValueBetween(-s, s);
        float z = getRandomValueBetween(-s, s);
        grassPositions[i].x = x;
        grassPositions[i].y = 0.5f;
        grassPositions[i].z = z;
    }

    

    Texture boxtxt("../resources/container.jpg");
    Texture grasstxt("../resources/grass.png");
    Texture grass_groundtxt("../resources/grass_ground.jpg");
    
    vector<std::string> txts
    {"../resources/Sky Textures/right1.jpg",
        "../resources/Sky Textures/left.jpg",
        "../resources/Sky Textures/bottom.jpg",
        "../resources/Sky Textures/top.jpg",
        "../resources/Sky Textures/front.jpg" ,
        "../resources/Sky Textures/back1.jpg"
    };
   

    CubemapTexture cubetxt(txts);

    vector<string> nighttxts= { "../resources/night sky textures/nightleft.png","../resources/night sky textures/nightright.png","../resources/night sky textures/nightbottom.png","../resources/night sky textures/nighttop.png",
        "../resources/night sky textures/nightfront.png" ,"../resources/night sky textures/nightback.png" };

    CubemapTexture nightcubetxt(nighttxts);


    shader.use();
    shader.setInt("texture0", 0);
    shader.setInt("texture1", 1);

    skyboxShader.use();
    skyboxShader.setInt("skybox",0);
    skyboxShader.setInt("nightbox",1);


    // TODO : set texture & skybox texture uniform value (initialization)
    // e.g) shader.use(), shader.setInt("texture", 0);



    // render loop
    // -----------
    // camera
    //stringstream sss;
   
    //Text->RenderText(sss.str(), 0.9, 0.0, 1);

    while (!glfwWindowShouldClose(window)){

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastTime;
        lastTime = currentFrame;
        // input
        // -----
        processInput(window);

        if (Eflag) {
            if (Ecnt!=0) {
                camera.Echange();
                Ecnt--;
            }
            else {
                Eflag = false;
                Ecnt = 100;
            }
        }
        if (Rflag) {
            if (Ecnt != 0) {
                camera.Rchange((Ecnt-1)*0.01f);
                Ecnt--;
            }
            else {
                Rflag = false;
                Ecnt = 100;
                camera.goback();
            }
        }



         



        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        stringstream sss1;
        sss1 << "Last pressed key: ";
        if (last_press == 'E') {
            sss1 << "(E//yaw/pitch linear interpolation, right y should remain zero)";
        }
        else if (last_press == 'R') {
            sss1 <<    "R//spherical interpolation, right.y should not remain zero)";
        }
        else if (last_press == 'Q') {
            sss1 << "Q";
        }
        stringstream sss2;


        sss2 << "Saved Camera pos : vec3(" << camera.LastPosition.x << ", " << camera.LastPosition.y << "," <<
            camera.LastPosition.z << ")" ;

        stringstream sss3;
        sss3 << "Current Camera pos: vec3(" << camera.Position.x << ", " << camera.Position.y << "," <<
            camera.Position.z << ")";
        stringstream sss4;
        sss4 << "Current Camera right: vec3(" << camera.Right.x << ", " << camera.Right.y << "," <<
            camera.Right.z << ")";
        stringstream sss5;
        sss5 << "Current Camera Yaw: " << camera.Yaw << ", Pitch: "<<camera.Pitch<<", Roll: "<<camera.Roll;

        Text->RenderText(sss1.str(), 1.0f, 10.0f, 0.8f, glm::vec3(0.9, 0.9f,0.9f));
        Text->RenderText(sss2.str(),1.0f,30.0f ,1.0f,glm::vec3(0.5, 0.8f, 0.5f));
        Text->RenderText(sss3.str(),1.0f,50.0f ,1.0f,glm::vec3(0.5, 0.8f, 0.5f));
        Text->RenderText(sss4.str(),1.0f,70.0f ,1.0f,glm::vec3(0.5, 0.8f, 0.5f));
        Text->RenderText(sss5.str(),1.0f,90.0f ,1.0f,glm::vec3(0.5, 0.8f, 0.5f));


        shader.use();
        
        

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        shader.setMat4("projection", projection);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("view", view);

        glBindVertexArray(boxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, boxtxt.ID);
        for (unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            shader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

       
       

        glBindVertexArray(QVAO);
        glBindTexture(GL_TEXTURE_2D, grasstxt.ID);
        for (int i = 0; i < n_grass; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            glm::mat4 tmp;
            if (Rflag) {
                glm::mat4 rot = glm::mat4_cast(camera.orientation);
                tmp = rot;
                tmp[1] = { 0,1,0,0 };
            }
            else {
                tmp = glm::rotate(model, glm::radians(camera.Yaw), camera.WorldUp);
            }

            model = glm::translate(model, grassPositions[i]);
            model =model*tmp;
            shader.setMat4("model", model);
            
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        
        glBindTexture(GL_TEXTURE_2D, grass_groundtxt.ID);
        {
            glm::mat4 model = glm::mat4(1.0f);
            
            float angle = 90.0f;
           
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(grassGroundSize, grassGroundSize, 1.0f));
          

            shader.setMat4("model", model);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        glBindVertexArray(0);


   
     
            glDepthFunc(GL_LEQUAL);
           
            skyboxShader.use();
          
             view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
            
            skyboxShader.setMat4("view", view);

                projection= glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            skyboxShader.setMat4("projection", projection);


            glBindVertexArray(skyVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubetxt.textureID);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, nightcubetxt.textureID);
            skyboxShader.setFloat("time", realTime / 24.0f);

            glDrawArrays(GL_TRIANGLES, 0, 36);

            glBindVertexArray(0);
            glBindTexture(GL_TEXTURE_2D, 0);
            glDepthFunc(GL_LESS);

           
         
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &boxVAO);
    glDeleteVertexArrays(1, &QVAO);
    glDeleteVertexArrays(1, &skyVAO);
    glDeleteBuffers(1, &boxVBO);
    glDeleteBuffers(1, &skyVBO);
    glDeleteBuffers(1, &QVBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // TODO : make camera movable (WASD) & increase or decrease dayFactor(press O: increase, press P: decrease)
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.ProcessKeyboard(FORWARD ,  deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.ProcessKeyboard(LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        realTime -= deltaTime*5.0f;
        if (realTime <= 0.0f) {
            realTime = 0.0f;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        realTime += deltaTime*5.0f;
        if (realTime >= 24.0f) {
            realTime = 24.0f;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        camera.Qclick();
        last_press = 'Q';
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        if (Eflag == false) {
            camera.EClick();
            last_press = 'E';
            Eflag = true;
        }
       
        
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        if (Rflag == false) {
            camera.RClick();
            Rflag = true;
            last_press = 'R';
        }
    }

   
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
    float x_pos = static_cast<float>(xpos);
    float y_pos = static_cast<float>(ypos);

    if (firstMouse)
    {
        lastX = x_pos;
        lastY = y_pos;
        firstMouse = false;
    }
    float xoffset = -x_pos + lastX;
    float yoffset = +lastY - y_pos; 
    lastX = x_pos;
    lastY = y_pos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}
