#ifndef VOLUMELOADER_H
#define VOLUMELOADER_H

#include <GLFW/glfw3.h>
#include <OpenGL/gl3.h>
#include <fstream>
#include <cstdlib>
#include <cstdio>

class VolumeLoader{
    private :
        const char *volume;
        const char *tff;
        unsigned int vol_tex_obj;//volume texture object
        unsigned int tff_tex_obj;

    public :
        VolumeLoader(const char *volume_path, const char *tff_path)
        {
            volume = volume_path;
            tff = tff_path;
        }
        ~VolumeLoader()
        {
            glDeleteTextures(1, &vol_tex_obj);
            glDeleteTextures(1, &tff_tex_obj);
        }

        void readVolumeToTexture(GLuint width, GLuint height, GLuint depth)
        {
            FILE *fp;

            if(!(fp = fopen(volume, "rb")))
            {
                std::cout << "ERROR : Failed to open " << volume << std::endl;
                exit(EXIT_FAILURE);
            }

            std::cout << "SUCCESS : Opened " << volume << std::endl;

            size_t size = width * height * depth;
            GLubyte *data = new GLubyte[size];
            std::cout << "volume size = " << size << std::endl;

            if(fread(data, sizeof(char), size, fp) != size)
            {
                std::cout << "ERROR : Failed to read " << volume << std::endl;
                exit(EXIT_FAILURE);
            }
            fclose(fp);

            glGenTextures(1, &vol_tex_obj);
            glBindTexture(GL_TEXTURE_3D, vol_tex_obj);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  
            std::cout << "Width, Height, Depth : " << width << ", " << height << ", " << depth << std::endl;
            glTexImage3D(GL_TEXTURE_3D,0 ,GL_R8, width, height, depth, 0, GL_RED, GL_UNSIGNED_BYTE, data);

            delete []data;
            std::cout << "Volume Loaded on Texture " << std::endl;
        }

        void readTFFToTexture()
        {
            std::ifstream inFile(tff, std::ifstream::in);

            if(!inFile)
            {
                std::cout << "Error : Can not open " << tff << std::endl;
                exit(EXIT_FAILURE);
            }

            const int max_cnt = 10000;

            GLubyte *data = (GLubyte *)calloc(max_cnt, sizeof(GLubyte));

            inFile.read(reinterpret_cast<char *>(data), max_cnt);

            if(inFile.eof())
            {
                size_t byte_cnt = inFile.gcount();
                *(data + byte_cnt) = '\0';

                std::cout << "byte_cnt : " << byte_cnt << std::endl;
            }else if(inFile.fail()){
                std::cout << "FAIL : Can not read " << tff << std::endl;
                exit(EXIT_FAILURE);
            }else{
                std::cout << "TFF file is too large" << std::endl;
            }

            glGenTextures(1, &tff_tex_obj);
            glBindTexture(GL_TEXTURE_1D, tff_tex_obj);
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            free(data);
        }

        unsigned int getVolTexObj(){
            return vol_tex_obj;
        }

        unsigned int getTFFTexObj(){
            return tff_tex_obj;
        }
};

#endif