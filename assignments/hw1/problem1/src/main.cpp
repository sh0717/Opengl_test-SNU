#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

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

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // positions
         0.5f, -0.25f, 0.0f, // bottom right
        -0.5f, -0.25f, 0.0f, // bottom left
         0.0f,  0.5f, 0.0f  // top
    };

    unsigned int VBO, VAO;
    /**********Fill in the blank*********/
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    /*************************************/


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);
        
        ourShader.setVec3("grav_cent", glm::vec3(0.0f, 0.0f, 0.0f));


        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // render the triangle
        ourShader.use();
        
        float timeValue = glfwGetTime();

        /**********Fill in the blank*********/
        float blueVal = static_cast<float>(sin(timeValue) / 2.0f + 0.5f);
        int vColorLocation = glGetUniformLocation(ourShader.getID(), "ourColor");
        glUniform4f(vColorLocation, 0.0f, 0.0f, blueVal, 1.0f);

        /*************************************/

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

float up_count = 0;
float ri_count = 0;
float theta = 0;
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    Shader ourShader("../shaders/shader.vs", "../shaders/shader.fs");

    /**********Fill in the blank*********/


    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
       up_count++;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        up_count--;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        ri_count++;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        ri_count--;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        theta +=0.01;
    }
    glm::mat3 ro_ma({ cos(theta),sin(theta),0 }, {- sin(theta),cos(theta),0 }, { 0,0,1 });
    ourShader.setVec3("move_vec", 0.01f*ri_count, 0.01f * up_count, 0.0f);
    ourShader.setMat3("rotate_mat", ro_ma);
    /*************************************/
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
