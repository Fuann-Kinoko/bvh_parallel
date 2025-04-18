#include "IOUtils.h"

#include <fstream>
#include <sstream>

void IOUtils::readAllLines(const std::string& path, std::string *buffer) {
    std::ifstream t(path);
    std::stringstream buf;
    buf << t.rdbuf();
    *buffer = buf.str();
}

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../lib/stb/stb_image_write.h"
#endif

void IOUtils::writeFramebufferToFile(const std::string &file, int width, int height) {
    char *data = (char*) calloc(4, width * height);

    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

    // stbi_flip_vertically_on_write(1);
    stbi_write_png(
        file.c_str(),
        width,
        height,
        4,
        data + (width * 4 * (height - 1)),
        -width * 4);

    free(data);
}