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
#include "text_renderer.h"
using namespace std;
using namespace glm;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, float dt);

// setting
const unsigned int SCR_WIDTH = 900;
const unsigned int SCR_HEIGHT = 600;
unsigned int SCR_WIDTH_SSAA = SCR_WIDTH * 2;
unsigned int SCR_HEIGHT_SSAA = SCR_HEIGHT * 2;


bool previousKeyState[1024];

const int NO_ANTIALIASING = 1;
const int SSAA = 2;
const int MSAA = 3;
const int CONVOLUTION = 4;
const int FXAA = 5;
int antiAliasingMode = NO_ANTIALIASING;

int main()
{
    
    // glfw: initialize and configure
    // ------------------------------'
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);




    // TODO1 : Add glfwWindowHint for Multisampling

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
    
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // build and compile our shader program
    // ------------------------------------
    Shader ourShader("../shaders/shader.vs", "../shaders/shader.fs"); // you can name your shader files however you like
    Shader convolutionShader("../shaders/convolution.vs", "../shaders/convolution.fs");
    Shader FXAAShader("../shaders/fxaa.vs", "../shaders/fxaa.fs");


    // screen quad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    



    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH_SSAA, SCR_HEIGHT_SSAA, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ////////SSAA

    unsigned int mFbo;
    glGenFramebuffers(1, &mFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
    unsigned int mTex;
    glGenTextures(1, &mTex);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mTex);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, mTex, 0);
    //////MSAA

    unsigned int cfbo;
    glGenFramebuffers(1, &cfbo);
    glBindFramebuffer(GL_FRAMEBUFFER, cfbo);
    unsigned int ctexture;
    glGenTextures(1, &ctexture);
    glBindTexture(GL_TEXTURE_2D, ctexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ctexture, 0);







    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    unsigned int VAO, VBO;


    getPositionColorVAOEBO(triangle_position_colors, sizeof(triangle_position_colors), triangle_indices, sizeof(triangle_indices), VAO, VBO);

    TextRenderer  *Text;
    Text = new TextRenderer(SCR_WIDTH, SCR_HEIGHT);
    Text->Load("../fonts/OCRAEXT.TTF", 24);
    
    // render loop
    // -----------
    float oldTime = 0;

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    int frameCount = 0;
    float fpsTime = 0.0f;
    float fps = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        float dt = (float)glfwGetTime() - oldTime;
        oldTime = glfwGetTime();

        frameCount += 1;
        fpsTime += dt;
        if (fpsTime > 1.0f) {
            fps = frameCount / fpsTime;
            fpsTime = 0.0f;
            frameCount = 0;
        }

        processInput(window, dt);

        if (antiAliasingMode == SSAA) {
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glViewport(0, 0, SCR_WIDTH_SSAA, SCR_HEIGHT_SSAA);
        }
        else if (antiAliasingMode == MSAA) {
            glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        }
        else if (antiAliasingMode == CONVOLUTION||antiAliasingMode==FXAA) {
            glBindFramebuffer(GL_FRAMEBUFFER, cfbo);
            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        }
     
        // TODO2 : bind framebuffer

        

        // render
        // ------
  

            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

        
        glm::mat4 projectionMatrix = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, 0.1f, 100.0f);

        // render the triangle
        ourShader.use();
        glBindVertexArray(VAO);
        unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");

        ourShader.setBool("isStripped", true);
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(0.5f, 0.0f, 0));
        transform = glm::scale(transform, glm::vec3(1.0f));
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
        ourShader.setBool("isStripped", false);

        for (int i = 0; i < 4; i++) {
            glm::mat4 transform = glm::mat4(1.0f);
            transform = glm::translate(transform, glm::vec3(-1 + (i / 2) * 0.5f + 0.25f, (i % 2) * 0.5f - 0.3f, 0));
            transform = glm::scale(transform, glm::vec3(0.4f));
            transform = glm::rotate(transform, i * 28.0f + 5.0f, glm::vec3(0.0f, 0.0f, 1.0f));
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
        }

      
        // TODO2 : copy to main framebuffer
      

        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

        if (antiAliasingMode == SSAA) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glClearColor(0.5f, 0.5f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glBlitFramebuffer(0, 0, SCR_WIDTH_SSAA, SCR_HEIGHT_SSAA, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
           
        }
        else if (antiAliasingMode == MSAA) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, mFbo);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glClearColor(0.5f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }
        else if (antiAliasingMode == CONVOLUTION) {
            

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClearColor(0.7f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            convolutionShader.use();
            glUniform1i(glGetUniformLocation(convolutionShader.ID, "screenTexture"), 0);

            glBindVertexArray(quadVAO);
            glBindTexture(GL_TEXTURE_2D, ctexture);
            glDrawArrays(GL_TRIANGLES, 0, 6);

        }
        else if(antiAliasingMode==FXAA) {
        

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClearColor(0.7f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            FXAAShader.use();
            glUniform1i(glGetUniformLocation(FXAAShader.ID, "uInpTex"), 0);

            glBindVertexArray(quadVAO);
            glBindTexture(GL_TEXTURE_2D, ctexture);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        
        }



        Text->RenderText("FPS : " + std::to_string(fps), 5.0f, 5.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
        std::string antiAliasingModeName;
        switch (antiAliasingMode) {
        case NO_ANTIALIASING: {antiAliasingModeName = "no antialiasing"; break; }
        case MSAA: {antiAliasingModeName = "MSAA"; break; }
        case SSAA: {antiAliasingModeName = "SSAA"; break; }
        case CONVOLUTION: {antiAliasingModeName = "Simple Convolution"; break; }
        case FXAA: {antiAliasingModeName = "FXAA"; break; }
        }

        Text->RenderText("MODE : " + antiAliasingModeName, 5.0f, 35.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    }


    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window, float dt)
{
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !previousKeyState[GLFW_KEY_1]) {
        previousKeyState[GLFW_KEY_1] = true;
        antiAliasingMode = NO_ANTIALIASING;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !previousKeyState[GLFW_KEY_2]) {
        previousKeyState[GLFW_KEY_2] = true;
        antiAliasingMode = SSAA;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && !previousKeyState[GLFW_KEY_3]) {
        previousKeyState[GLFW_KEY_3] = true;
        antiAliasingMode = MSAA;
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && !previousKeyState[GLFW_KEY_4]) {
        previousKeyState[GLFW_KEY_4] = true;
        antiAliasingMode = CONVOLUTION;
    }
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS && !previousKeyState[GLFW_KEY_5]) {
        previousKeyState[GLFW_KEY_5] = true;
        antiAliasingMode = FXAA;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE) {
        previousKeyState[GLFW_KEY_1] = false;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE) {
        previousKeyState[GLFW_KEY_2] = false;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE) {
        previousKeyState[GLFW_KEY_3] = false;
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE) {
        previousKeyState[GLFW_KEY_4] = false;
    }
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE) {
        previousKeyState[GLFW_KEY_5] = false;
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
