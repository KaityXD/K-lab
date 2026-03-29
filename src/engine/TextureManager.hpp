#pragma once

#include <string>
#include <GL/glew.h>
#include <vector>

namespace klab {

std::string readFile(const std::string& path);
unsigned int loadTexture(const char* path);
unsigned int generateCheckerboardTexture();

} // namespace klab
