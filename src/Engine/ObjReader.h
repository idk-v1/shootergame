#ifndef OBJREADER_H
#define OBJREADER_H

#include "OpenGL_helper.h"
#include <string>
#include <vector>

std::vector<GLH::Vertex> readObjFile(const std::string& name);

#endif