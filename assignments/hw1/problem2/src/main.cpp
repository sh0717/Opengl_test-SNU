#define GLM_ENABLE_EXPERIMENTAL
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))  
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))  
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "gameobjects.h"
#include "geometry_primitives.h"
#include <iostream>
#include <vector>
#include "text_renderer.h"

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, Bar& bar, float dt);

// setting
const unsigned int SCR_WIDTH = 900;
const unsigned int SCR_HEIGHT = 600;

bool previousKeyState[1024];


float barXPosition =0.0f;


bool Check(float x, float y , float bar_left, float bar_right, float bar_down, float bar_up) {

    return bar_left < x && x < bar_right && bar_down < y && y < bar_up;

}

int score = 0;

int main()
{
    
    // glfw: initialize and configure
    // ------------------------------'
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "yoo_sh", NULL, NULL);
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


    /////////////////////////////////////////////////////
    // TODO : Define VAO and VBO for triangle and quad(bar).
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)* 24, quad_positions_colors, GL_STATIC_DRAW);
  
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);


    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) *6, (void*)(3*sizeof(float)) );
    
    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW);
   



    unsigned int TVAO, TVBO;
    glGenVertexArrays(1, &TVAO);
    glBindVertexArray(TVAO);
    glGenBuffers(1, &TVBO);
    glBindBuffer(GL_ARRAY_BUFFER, TVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*18, triangle_position_colors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(3 * sizeof(float)));







    /////////////////////////////////////////////////////



    std::vector<Entity> entities;
    

    TextRenderer  *Text;
    Text = new TextRenderer(SCR_WIDTH, SCR_HEIGHT);
    Text->Load("../fonts/OCRAEXT.TTF", 24);

    Bar bar{ 0,0,0 };

    // render loop
    // -----------
    float generationInterval = 1.0f;
    float dt = 0.0f;
    int score = 0;

    float barWidth = 0.3f;
    float barHeight = 0.05f;
    float barYPosition = -0.8f;
    bar.speed = 0.5;


    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int tri_cnt = 0;
    while (!glfwWindowShouldClose(window))
    {
        float temp = glfwGetTime();
        
        processInput(window, bar, dt);
        
        int additional = temp / generationInterval - tri_cnt;
        for (int i = 0; i < additional; i++) {
            entities.push_back(getRandomEntity());
        }
        tri_cnt += additional;
       

       

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ourShader.use();
        for (int i = 0; i < entities.size(); i++) {

            Entity& Ent = entities[i];

            Ent.position.y -= Ent.dropSpeed* dt;
            Ent.rotate += Ent.rotateSpeed * dt;
            float rot = Ent.rotate;

            glm::mat4 scale{ {Ent.scale,0,0,0},{0,Ent.scale,0,0},{0,0,Ent.scale ,0},{0,0,0,1} };
            glm::mat4 trans{ {1,0,0,0},{0,1,0,0},{0,0,1,0},{Ent.position.x,Ent.position.y,0,1} };
            glm::mat4 rotate{ {cos(rot),-sin(rot),0,0},{sin(rot),cos(rot),0,0},{0,0,1,0},{0,0,0,1} };
            glm::vec4 gravity(0,0,0,0);
            ourShader.setMat4("SizeMat", scale);
            ourShader.setMat4("TransMat", trans);
            ourShader.setMat4("RotMat", rotate);
            ourShader.setVec4("GravVec", gravity);

            glBindVertexArray(TVAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);

        }

    


        glm::mat4 transpose{ {1,0,0,0},{0,1,0,0},{0,0,1,0},{bar.xPosition,barYPosition,0,1} };
        glm::mat4 bar_size{ {barWidth,0,0,0},{0,barHeight,0,0},{0,0,1,0},{0,0,0,1} };
        glm::mat4 elemat{ {1,0,0,0} ,{0,1,0,0} ,{0,0,1,0} ,{0,0,0,1} };
        ourShader.setMat4("SizeMat", bar_size);
        ourShader.setMat4("TransMat", transpose);
        ourShader.setMat4("RotMat", elemat);
        ourShader.setVec4("GravVec", glm::vec4(0, 0, 0, 0));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


        for (int i = 0; i < entities.size(); i++) {
            Entity& Ent = entities[i];
            if (Check(Ent.position.x, Ent.position.y, bar.xPosition - barWidth * 0.5, bar.xPosition + barWidth * 0.5, barYPosition - barHeight * 0.5, barYPosition + barHeight * 0.5)) {
              /*  cout << bar.xPosition << " " << "(" << Ent.position.x << " " << Ent.position.y << ")" << Ent.scale<<endl;*/

                score++;
                
                entities.erase(entities.begin() + i);
                i--;
            }
            else if (Ent.position.y < -1) {
                

                entities.erase(entities.begin() + i);
                i--;
            }
        }

        for (int i = 0; i < score; i++) {
            score %= 20;
            float rot = 0;
            glm::mat4 scale{ {0.1,0,0,0},{0,0.1,0,0},{0,0,1 ,0},{0,0,0,1} };
            glm::mat4 trans{ {1,0,0,0},{0,1,0,0},{0,0,1,0},{-0.95+0.1*i,-0.9,0,1} };
            glm::mat4 rotate{ {cos(rot),-sin(rot),0,0},{sin(rot),cos(rot),0,0},{0,0,1,0},{0,0,0,1} };
            glm::vec4 gravity(0, 0, 0, 0);
            ourShader.setMat4("SizeMat", scale);
            ourShader.setMat4("TransMat", trans);
            ourShader.setMat4("RotMat", rotate);
            ourShader.setVec4("GravVec", gravity);

            glBindVertexArray(TVAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        

       


        stringstream sss;
        sss << "Score: " << score << "   " << bar.xPosition << "," << barYPosition;
        Text->RenderText(sss.str(), -0.9, 0.9, 1);

        dt = glfwGetTime() - temp;
        glfwSwapBuffers(window);
        glfwPollEvents();
      
    }


    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &TVAO);
    glDeleteBuffers(1, &TVBO);
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window, Bar& bar, float dt)
{   

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        bar.xPosition += dt*bar.speed;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        bar.xPosition -= dt*bar.speed;


}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
