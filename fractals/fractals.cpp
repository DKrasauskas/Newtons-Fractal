#define uint unsigned int 
#include <iostream>
#include <iostream>
#include <string>
#include "include/glad/glad.h"
#include "include/GLFW/glfw3.h"
#include "Shader.h"
#include "Buffer.h"
#include "grid.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 1200;

#define DOMAIN 1200

float zoom = 1.0f;
int depth = 1;
float valx = -0.5628f;
float valy = -0.5628f;


int n = 8; // polynomial degree
//complex roots
float rx[5] = {
    -1.3247f, 0.0f, 0.0f, 0.667f, 0.667f
};
float ry[5] = {
    0.0f, -1.0f, 1.0f, 0.5628f, -0.5628f
};

int state = -1;
int click = -1;

int check_collision(float x, float y, float* vx, float* vy) {
    float min_d = 0xFFFFFF;
    for (int i = 0; i < n; i++) {
        if ((vx[i] - x) * (vx[i] - x) + (vy[i] - y) * (vy[i] - y) < 0.1f) {
            return i;
        }
    }
    return -1;
}
int main()
{
    float* vx, * vy;
    vx = (float*)malloc(sizeof(float) * n);
    vy = (float*)malloc(sizeof(float) * n);
    for (int i = 0; i < n; i++) {
        vx[i] = .5f * sin((float)i / n * 6.28);
        vy[i] = .5f * cos((float)i / n * 6.28);
    }
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_SAMPLES, 8);
    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Fractals", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    Shader vertex("vertex.glsl", "fragment.glsl");
    Cshader compute("compute.glsl");
    Grid gd = grid(2);
    Buffer buff((void*)gd.vertices, (void*)gd.indices, gd.v_size, gd.i_size);
    GLuint vxb, vyb;
    unsigned int texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, DOMAIN, DOMAIN, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    int cnt = 0;
    glGenBuffers(1, &vxb);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vxb);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vxb);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * n, vx, GL_STATIC_DRAW); //sizeof(data) only works for statically sized C/C++ arrays.
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vxb);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind   
    glGenBuffers(1, &vyb);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, vyb);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vyb);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * n, vy, GL_STATIC_DRAW); //sizeof(data) only works for statically sized C/C++ arrays.
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, vyb);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    float a = 1.0f;
    click = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    while (!glfwWindowShouldClose(window))
    { 
        glUseProgram(compute.ID);
        glUniform1f(glGetUniformLocation(compute.ID, "max_p"), zoom);
        glUniform1i(glGetUniformLocation(compute.ID, "min_p"), depth);       
        glDispatchCompute(DOMAIN / 20, DOMAIN / 20, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(vertex.ID);
        glBindVertexArray(buff.VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawElements(GL_TRIANGLES, 6 * gd.v_size / sizeof(float), GL_UNSIGNED_INT, (void*)0);
        glfwSwapBuffers(window);
        glfwPollEvents();            
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vxb);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, vxb);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * n, vx, GL_STATIC_DRAW); //sizeof(data) only works for statically sized C/C++ arrays.
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vxb);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind            
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, vyb);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, vyb);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * n, vy, GL_STATIC_DRAW); //sizeof(data) only works for statically sized C/C++ arrays.
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, vyb);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {           
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            x = zoom * (float)2 / 1200 *  (x - 600);
            y = -zoom * (float)2 / 1200 * (y - 600);
            cout << x << " " << y << "\n";
            int id = check_collision(x, y, vx, vy);
            if (id != -1 && state == -1) {
                state = id;
            }               
        }      
        if (state != -1) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            x = zoom*(float)2 / 1200 * (x - 600);
            y = zoom * (float)2 / 1200 * (y - 600);
            vx[state] = x;
            vy[state] = -y;    
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
            state = -1;
        }                
    }
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        zoom += zoom * 0.01f;;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        zoom -= zoom * 0.01f;;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        depth += 1;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        depth -= 1;
   
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
