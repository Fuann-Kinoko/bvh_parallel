#ifndef RENDERENGINE_IOUTILS_H
#define RENDERENGINE_IOUTILS_H

#include "glad/glad.h"
// #include "GL/glew.h"

#include <filesystem>

class IOUtils {
public:
    static void readAllLines(const std::string& path, std::string *buffer);

    static void writeFramebufferToFile(const std::string& file, int width, int height);
};

#endif