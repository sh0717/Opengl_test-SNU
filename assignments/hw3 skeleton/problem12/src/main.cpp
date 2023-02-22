#define GLM_ENABLE_EXPERIMENTAL
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))  
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))  
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "resource_utils.h"
#include <iostream>
#include <vector>
#include "camera.h"
#include "math_utils.h"

using namespace glm;

vector<vec3> pos = {vec3(-1,0.0,0.0 ),vec3(1,0.0,0.0),vec3(3,0.0,0.0),vec3(4.5f,-0.5f,0.0),vec3(7,0.0,0.0),vec3(9,0.0,0.0)};
bool outerSwitch = true;
bool triangleSwitch = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

bool isKeyboardDone[1024] = {0};

// setting
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;


// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


glm::mat4 BezMat= mat4(vec4(-1, 3, -3, 1), vec4(3, -6, 3, 0), vec4(-3, 3, 0, 0), vec4(1, 0, 0, 0));
glm::mat4 B_Mat =  mat4(vec4(-1/6.0f, 3/6.0f, -3/6.0f, 1/6.0f), vec4(3/6.0f, -6/6.0f, 0/6, 4/6.0f), vec4(-3/6.0f, 3/6.0f, 3/6.0f, 1/6.0f), vec4(1/6.0f, 0/6, 0, 0));
glm::mat4 CaR_Mat = mat4(vec4(-1/2.0f, 3/2.0f, -3/2.0f, 1/2.0f), vec4(2/2.0f, -5/2.0f, 4/2.0f, -1/2.0f), vec4(-1/2.0f, 0, 1/2.0f, 0), vec4(0, 2/2.0f, 0, 0));

int main()
{
    B_Mat = glm::transpose(B_Mat);
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
    
    Shader shader("../shaders/outer_line_shader.vs", "../shaders/outer_line_shader.fs");
    Shader splineShader("../shaders/splines/spline_shader.vs", "../shaders/splines/spline_shader.fs", "../shaders/splines/spline_shader.gs");

    // build and compile our shader program
    // ------------------------------------
    // TODO: define 3 shaders
    // (1) geometry shader for spline render.
    // (2) simple shader for spline's outer line render.
    // (optional) (3) tessellation shader for bezier surface.
    Shader tessShader("../shaders/bezier_surface/tess.vs", "../shaders/bezier_surface/tess.fs", "../shaders/bezier_surface/tess.gs", "../shaders/bezier_surface/TCS.glsl", "../shaders/bezier_surface/TES.glsl");
    Shader tessShader2("../shaders/bezier_surface/tess.vs", "../shaders/bezier_surface/tess.fs", "../shaders/bezier_surface/tess2.gs", "../shaders/bezier_surface/TCS.glsl", "../shaders/bezier_surface/TES.glsl");

    // TODO : load requied model and save data to VAO. 
    // Implement and use loadSplineControlPoints/loadBezierSurfaceControlPoints in resource_utils.h

    VAO* sp_simple_VAO = loadSplineControlPoints("../resources/spline_control_point_data/spline_simple.txt");
    VAO* sp_complex_VAO = loadSplineControlPoints("../resources/spline_control_point_data/spline_complex.txt");
    VAO* sp_U_VAO = loadSplineControlPoints("../resources/spline_control_point_data/spline_u.txt");

    VAO* teapot_VAO = loadBezierSurfaceControlPoints("../resources/bezier_surface_data/teapot.bpt");
    VAO* heart_VAO = loadBezierSurfaceControlPoints("../resources/bezier_surface_data/heart.bpt");
    VAO* sphere_VAO = loadBezierSurfaceControlPoints("../resources/bezier_surface_data/sphere.bpt");
    VAO* gumbo_VAO = loadBezierSurfaceControlPoints("../resources/bezier_surface_data/gumbo.bpt");


    // render loop
    // -----------
    float oldTime = 0;
    while (!glfwWindowShouldClose(window))
    {

     

        float currentTime = glfwGetTime();
        float dt = currentTime - oldTime;
        deltaTime = dt;

        oldTime = currentTime;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        shader.setMat4("projection", projection);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("view", view);
        glm::mat4 model;
        if (outerSwitch) {
           
          
            {
                model = glm::translate(glm::mat4(1.0f), pos[0]);
                shader.setMat4("model", model);
                glBindVertexArray(sp_simple_VAO->ID);
                glDrawArrays(GL_LINE_STRIP, 0, sp_simple_VAO->dataSize);

                model = glm::translate(glm::mat4(1.0f), pos[1]);
                shader.setMat4("model", model);
                glDrawArrays(GL_LINE_STRIP, 0, sp_simple_VAO->dataSize);

                model = glm::translate(glm::mat4(1.0f), pos[2]);
                shader.setMat4("model", model);
                glDrawArrays(GL_LINE_STRIP, 0, 4);
            }

            model = glm::translate(glm::mat4(1.0f), pos[4]);
            shader.setMat4("model", model);
            glBindVertexArray(sp_complex_VAO->ID);
            glDrawArrays(GL_LINE_STRIP, 0, sp_complex_VAO->dataSize);
            model = glm::translate(glm::mat4(1.0f), pos[5]);
            shader.setMat4("model", model);
            glDrawArrays(GL_LINE_STRIP, 0, sp_complex_VAO->dataSize);


            model = glm::translate(glm::mat4(1.0f), pos[3]);
            model = glm::scale(model, glm::vec3(0.2f, 0.2f, 1.0f));
            shader.setMat4("model", model);
            glBindVertexArray(sp_U_VAO->ID);
            glDrawArrays(GL_LINE_STRIP, 0, sp_U_VAO->dataSize);
            // TODO : render splines
            // (1) render simple spline with 4 control points for Bezier, Catmull-Rom and B-spline.
            // (2) render 'u' using Bezier spline
            // (3) render loop using Catmull-Rom spline and B-spline.
            // You have to also render outer line of control points!
        }
        splineShader.use();
        splineShader.setMat4("projection", projection);
        splineShader.setMat4("view", view);

        model = glm::translate(glm::mat4(1.0f),pos[0]);
        splineShader.setMat4("model", model);
        splineShader.setMat4("B", BezMat);
        glBindVertexArray(sp_simple_VAO->ID);
        glDrawArrays(GL_LINES_ADJACENCY, 0, 4);

        
        model = glm::translate(glm::mat4(1.0f), pos[1]);
        splineShader.setMat4("model", model);
        splineShader.setMat4("B", B_Mat);
        glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, 4);

        model = glm::translate(glm::mat4(1.0f), pos[2]);
        splineShader.setMat4("model", model);
        splineShader.setMat4("B", CaR_Mat);
        glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, 4);

        
        model = glm::translate(glm::mat4(1.0f), pos[4]);
        splineShader.setMat4("model", model);
        splineShader.setMat4("B", B_Mat);
        glBindVertexArray(sp_complex_VAO->ID);
        glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, sp_complex_VAO->dataSize);

        model = glm::translate(glm::mat4(1.0f), pos[5]);
        splineShader.setMat4("model", model);
        splineShader.setMat4("B", CaR_Mat);
        glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, sp_complex_VAO->dataSize);

        model = glm::translate(glm::mat4(1.0f), pos[3]);
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 1.0f));
        splineShader.setMat4("model", model);
        splineShader.setMat4("B", BezMat);
        glBindVertexArray(sp_U_VAO->ID);
        glDrawArrays(GL_LINES_ADJACENCY, 0, sp_U_VAO->dataSize);

        if (triangleSwitch) {
            // (Optional) TODO : render Bezier surfaces using tessellation shader.
            tessShader.use();
            tessShader.setVec3("cameraPosition", camera.Position);
            tessShader.setFloat("scale", 1.0f);
            tessShader.setMat4("projection", projection);
            tessShader.setMat4("view", view);

            tessShader.setFloat("uOuter02", 10);
            tessShader.setFloat("uOuter13", 10);
            /*    tessShader.setFloat("uInner0", 5);
                tessShader.setFloat("uInner1", 5);*/



            model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, -2.0f, 0.0f));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.5, 0.5, 0.4));
            tessShader.setMat4("model", model);
            glBindVertexArray(teapot_VAO->ID);

            glPatchParameteri(GL_PATCH_VERTICES, 16);

            glDrawArrays(GL_PATCHES, 0, teapot_VAO->dataSize);

            model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, 0.0f));

            model = glm::scale(model, glm::vec3(0.3, 0.3, 0.5f));
            tessShader.setMat4("model", model);
            glBindVertexArray(heart_VAO->ID);

            glDrawArrays(GL_PATCHES, 0, heart_VAO->dataSize);



            model = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, -2.0f, 0.0f));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.5, 0.5, 0.4));
            tessShader.setMat4("model", model);
            glBindVertexArray(sphere_VAO->ID);


            glDrawArrays(GL_PATCHES, 0, sphere_VAO->dataSize);



            model = glm::translate(glm::mat4(1.0f), glm::vec3(3.00f, -4.0f, 0.0f));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
            tessShader.setMat4("model", model);
            glBindVertexArray(gumbo_VAO->ID);

            glDrawArrays(GL_PATCHES, 0, gumbo_VAO->dataSize);
        }
        else {
            tessShader2.use();
            tessShader2.setVec3("cameraPosition", camera.Position);
            tessShader2.setFloat("scale", 1.0f);
            tessShader2.setMat4("projection", projection);
            tessShader2.setMat4("view", view);

            tessShader2.setFloat("uOuter02", 100);
            tessShader2.setFloat("uOuter13", 100);
            /*    tessShader.setFloat("uInner0", 5);
                tessShader.setFloat("uInner1", 5);*/



            model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, -2.0f, 0.0f));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.5, 0.5, 0.4));
            tessShader2.setMat4("model", model);
            glBindVertexArray(teapot_VAO->ID);

            glPatchParameteri(GL_PATCH_VERTICES, 16);

            glDrawArrays(GL_PATCHES, 0, teapot_VAO->dataSize);

            model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, 0.0f));

            model = glm::scale(model, glm::vec3(0.3, 0.3, 0.5f));
            tessShader2.setMat4("model", model);
            glBindVertexArray(heart_VAO->ID);

            glDrawArrays(GL_PATCHES, 0, heart_VAO->dataSize);



            model = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, -2.0f, 0.0f));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.5, 0.5, 0.4));
            tessShader2.setMat4("model", model);
            glBindVertexArray(sphere_VAO->ID);


            glDrawArrays(GL_PATCHES, 0, sphere_VAO->dataSize);



            model = glm::translate(glm::mat4(1.0f), glm::vec3(3.00f, -4.0f, 0.0f));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
            tessShader2.setMat4("model", model);
            glBindVertexArray(gumbo_VAO->ID);

            glDrawArrays(GL_PATCHES, 0, gumbo_VAO->dataSize);
        
        }

        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    //glDeleteVertexArrays(1,&VAO);
    //glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

bool pressed9;
bool pressed0;
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
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
    bool curent_pressed9 = glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS;

    if (curent_pressed9 && !pressed9) {
        outerSwitch = !outerSwitch;
    }
    pressed9 = curent_pressed9;

    bool curent_pressed0 = glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS;

    if (curent_pressed0 && !pressed0) {
        triangleSwitch = !triangleSwitch;
    }
    pressed0 = curent_pressed0;



    // TODO : 
    // (1) (for spline) if we press key 9, toggle whether to render outer line.
    // (2) (Optional, for Bezier surface )if we press key 0, toggle GL_FILL and GL_LINE.
    
    
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