#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "cube.h"
#include "shader.h"
#include "camera_quat.h"
#include "volumeloader.h"
using namespace std;

//string _name = "Engine_1_256.raw";
//glm::vec3 volume_size = glm::vec3(256,256,256);

Camera camera(glm::vec3(0.0, 0.0, 3.0));

double screen_width = 400.0;
double screen_height = 400.0;
double lastX = screen_width/2.0;
double lastY = screen_height/2.0;
bool first_mouse = true;

glm::vec4 bg_color = glm::vec4(0.5, 0.5, 0.5, 1.0);

void init();
void mouseCallback(GLFWwindow *window, double x_pos, double y_pos);
void checkFramebufferStatus();
void processInput(GLFWwindow *window, float delta_time);
void RCSetUniforms(Shader shader, glm::mat4 MVP, unsigned int tff, unsigned int bf, unsigned int vol);

unsigned int genBackFaceTextureBuffer(int tex_width, int tex_height);
unsigned int gen2DFramebuffer(unsigned int tex_obj, int tex_width, int tex_height);


int main(int argc, char** argv)
{   
    string file_name = "./data/";
    glm::vec3 volume_size;

    if(argc != 5){
        cout << "Error :: Invalid inputs. " << endl;
        cout << "Arguments -> volume_name volume_size_x, volume_size_y, volume_size_z" <<endl;
        cout << "Example : ./main Engine_1_256.raw 256 256 256" << endl;
        exit(EXIT_FAILURE);
    }else{
        file_name = file_name + argv[1];
        volume_size = glm::vec3(stoi(argv[2]), stoi(argv[3]), stoi(argv[4]));
    }

    init();
    GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, "Volume Rendering", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
    //glfwSetScrollCallback(window, scrollCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LESS);
    glViewport(0,0,screen_width, screen_height);

    Shader eep_shader("./shaders/cube.vert", "./shaders/cube.frag");
    Shader rc_shader("./shaders/raycasting.vert", "./shaders/raycasting.frag");
    Cube eep_cube(eep_shader);
    Cube rc_cube(rc_shader);
    VolumeLoader vol_loader(file_name.c_str(), "./data/tff.dat");
    vol_loader.readVolumeToTexture(volume_size.x, volume_size.y, volume_size.z);
    vol_loader.readTFFToTexture();

    // 볼륨 렌더링에 필요한 텍스쳐 설정
    unsigned int vol_tex_obj = vol_loader.getVolTexObj(); // 렌더패스 2에서 사용할 볼륨 데이터를 3D 텍스쳐로 만들어 사용
    unsigned int tff_tex_obj = vol_loader.getTFFTexObj(); // 렌더패스 2에서 볼륨 데이터의 인텐시티 값에 매칭될 색상값을 저장한 데이터 파일, 1D 텍스쳐에 저장
    unsigned int bf_tex_obj = genBackFaceTextureBuffer(screen_width, screen_height); // 렌더패스 1에서 결과 이미지(화면상에 렌더링 될 프래그먼트 컬러값을 프래그먼트의 월드좌표상 좌표로 저장)
    unsigned int back_buffer = gen2DFramebuffer(bf_tex_obj, screen_width, screen_height); // 백버퍼

    float last_frame = 0.0f;
    float current_frame =0.0f;

    while(!glfwWindowShouldClose(window)){
        float time = glfwGetTime();
        current_frame = glfwGetTime();
        processInput(window, current_frame - last_frame);
        last_frame = current_frame;
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(180.0f) , glm::vec3(0.0, 1.0, 0.0));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0, 0.0 ,0.0));

        model = glm::translate(model, glm::vec3(-0.5, -0.5, -0.5));
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screen_width/(float)screen_height, 0.1f, 1000.0f);
        glm::mat4 MVP = projection * view * model;

        // 렌더 패스 1 - 페이스 전면을 제거하고 파이프라인을 통과한 최종 이미지를 back_buffer에 바인딩된 2D 텍스쳐에 그린다.
        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, back_buffer);
        glViewport(0,0, screen_width, screen_height);
        glClearColor(bg_color[0], bg_color[1], bg_color[2] ,bg_color[3]);
        glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
        eep_shader.use();
        eep_shader.setMat4("MVP", MVP);
        eep_cube.draw(GL_FRONT);
        glUseProgram(0);

        // 렌더패스 2 - 페이스 후면을 제거하고 볼륨, tff 텍스쳐 이미지를 레이캐스트 쉐이더에 선언한
        // 유니폼 변수들에 전달하고, 렌더패스 1에서 만든 2D 텍스쳐 이미지 역시 같이 유니폼 변수로 전달한다.
        // 2D 텍스쳐의 각 픽셀은 렌더 패스1에서 픽셀에 매칭되는 프래그먼트들의 월드좌표계에서 좌표값을 색상값으로 갖는다.
        // 앞면을 표기해야하므로 glCullFace(GL_BACK);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0,0, screen_width, screen_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(bg_color[0], bg_color[1], bg_color[2] ,bg_color[3]);

        RCSetUniforms(rc_shader, MVP, tff_tex_obj, bf_tex_obj , vol_tex_obj);
        rc_cube.draw(GL_BACK);
        glUseProgram(0);
        GL_ERROR();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteTextures(1, &bf_tex_obj);
    glDeleteTextures(1, &tff_tex_obj);
    glDeleteTextures(1, &vol_tex_obj);
    glDeleteFramebuffers(1,&back_buffer);

    return EXIT_SUCCESS;
}

void checkFramebufferStatus()
{
    GLenum var = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(var != GL_FRAMEBUFFER_COMPLETE){
        std::cout << "Framebuffer is not complete." << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Framebuffer is complete." <<std::endl;
}

unsigned int genBackFaceTextureBuffer(int tex_width, int tex_height)
{
    unsigned int bf_tex;

    glGenTextures(1, &bf_tex);
    glBindTexture(GL_TEXTURE_2D,bf_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, tex_width, tex_height, 0, GL_RGBA, GL_FLOAT, NULL);

    return bf_tex;
}

unsigned int gen2DFramebuffer(unsigned int tex_obj, int tex_width, int tex_height)
{
    unsigned int depth_buffer;

    glGenRenderbuffers(1, &depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, tex_width, tex_height);

    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_obj, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

    checkFramebufferStatus();

    glEnable(GL_DEPTH_TEST);

    return fbo;
}

void RCSetUniforms(Shader shader, glm::mat4 MVP, unsigned int tff, unsigned int bf, unsigned int vol)
{
    shader.use();
    shader.setMat4("MVP", MVP);

    shader.setVec2("Resolution", glm::vec2(screen_width, screen_height));
    shader.setFloat("Step_Size", 0.5/256.0);
    shader.setVec4("Bg_Color", bg_color);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, tff);
    shader.setInt("TFF", 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bf);
    shader.setInt("ExitPoints", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, vol);
    shader.setInt("Volume", 2);
}

void processInput(GLFWwindow *window, float delta_time)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, delta_time);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, delta_time);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, delta_time);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, delta_time);
}

void mouseCallback(GLFWwindow* window, double x_pos, double y_pos)
{
    if(first_mouse  && GLFW_MOUSE_BUTTON_RIGHT){
        lastX = x_pos;
        lastY = y_pos;
        first_mouse = false;
    }

    float x_offset = x_pos - lastX;
    float y_offset = lastY - y_pos;

    lastX = x_pos;
    lastY = y_pos;

    camera.ProcessMouseMove(x_offset, y_offset);
}

void init()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
    #endif
}
