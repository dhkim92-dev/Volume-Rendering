#ifndef CUBE_H
#define CUBE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <OpenGL/gl3.h>
#include <GLFW/glfw3.h>
#include "shader.h"


#define GL_ERROR() checkForOpenGLError(__FILE__, __LINE__)
int checkForOpenGLError(const char* file, int line)
{
    // return 1 if an OpenGL error occured, 0 otherwise.
    GLenum glErr;
    int retCode = 0;

    glErr = glGetError();
    while(glErr != GL_NO_ERROR)
    {
        std::cout << "glError in file " << file << "@line " << line << " Error num : " << glErr  << " " << std::endl;
        retCode = 1;
        exit(EXIT_FAILURE);
    }
    return retCode;
}

class Cube{
    private :
        Shader shader;
        unsigned int vao, vbo, ebo;
        float scale;
        glm::vec3 pos;

    public :
        Cube(const Shader s) : shader(s)
        {   
            pos.x = 0.0;
            pos.y = 0.0;
            pos.z = 0.0;
            scale = 1.0;
            float vertices[24] = {
                    0.0, 0.0, 0.0,
                    0.0, 0.0, 1.0,
                    0.0, 1.0, 0.0,
                    0.0, 1.0, 1.0,
                    1.0, 0.0, 0.0,
                    1.0, 0.0, 1.0,
                    1.0, 1.0, 0.0,
                    1.0, 1.0, 1.0
                    };

            unsigned int indices[36] = {
                    1,5,7,
                    7,3,1,
                    0,2,6,
                    6,4,0,
                    0,1,3,
                    3,2,0,
                    7,5,4,
                    4,6,7,
                    2,3,7,
                    7,6,2,
                    1,0,4,
                    4,5,1
            };

            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glBindVertexArray(vao);
            glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE, 3*sizeof(float),(void*)0);
            glVertexAttribPointer(1,3,GL_FLOAT, GL_FALSE, 3*sizeof(float),(void*)0);
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glGenBuffers(1,&ebo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        }
        
        ~Cube(){
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vbo);
            glDeleteBuffers(1, &ebo);
        }

        void setPos(const float x, const float y,const float z){
            pos[0] = x;
            pos[1] = y;
            pos[2] = z;
        }

        void setScale(const float scale){
            if(scale <= 0){
                std::cout << "scale can not be setted - value or 0" << std::endl;
                return ;
            }
            this->scale = scale;
        }

        void draw(GLenum face)
        {
            glBindVertexArray(vao);
            glEnable(GL_CULL_FACE);
            glCullFace(face);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);
            glDisable(GL_CULL_FACE);
        }

        void setMVP(glm::mat4 MVP){
            shader.setMat4("MVP",MVP);
        }

        Shader getShader()
        {
            return shader;
        }
};

#endif