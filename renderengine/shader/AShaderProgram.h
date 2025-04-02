#ifndef RENDERENGINE_ASHADERPROGRAM_H
#define RENDERENGINE_ASHADERPROGRAM_H

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

class AShaderProgram {
public:
    bool is_valid() const;

    void bind() const;

    static void unbind();

    void cleanUp() const;

    void createUniform(const std::string& uniformName);

    void setUniform(const std::string& uniformName, glm::mat4 value);

    void setUniform(const std::string& uniformName, int value);

    void setUniform(const std::string& uniformName, float value);

    void setUniform(const std::string& uniformName, glm::vec3 value);

    void setUniform(const std::string& uniformName, glm::ivec3 value);

    void setUniform(const std::string& uniformName, glm::vec4 value);

    int getUniformLocation(const std::string& uniformName);

protected:
    void create();

    int createShader(const std::string& shaderCode, int shaderType) const;

    int programId = 0;
    std::unordered_map<std::string, int> uniforms;

};

#endif