#ifndef DRIVER_H
#define DRIVER_H
#include "core.hpp"

class OpenGLWorker {
    private:
        static GLuint program;
        static GLuint mvpID, transformID, normalID;
        static glm::mat4 View;
        static glm::mat4 Proj;
        static glm::mat4 MVP;
        static glm::mat3 NM;
    public:
        static bool transformed;

        static void initOpenGL();
        static void renderTransformedTexture(GLuint text, GLuint vao, GLuint vbo, glm::mat4 t);
        static void renderTexture(GLuint text, GLuint vao, GLuint vbo);
        static void rotate(float x, float y, float z, float direction);
        static void preStage();
        static void generateVAOVBO(int, int, int, int, GLuint&, GLuint&);
};
#endif