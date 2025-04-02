#ifndef RENDERENGINE_SHADERPROGRAM_H
#define RENDERENGINE_SHADERPROGRAM_H

#include <string>

#include "AShaderProgram.h"

class ShaderProgram : public AShaderProgram {
public:
    void init();

    void createVertexShader(const std::string &fileName);

    void createFragmentShader(const std::string &fileName);

    void link();

private:
    int vertexShaderId = 0;
    int fragmentShaderId = 0;
};

#endif