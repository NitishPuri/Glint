#pragma once

#include "glad/glad.h"
//
#include "GLFW/glfw3.h"
#include "Logger.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#ifndef ROOT
#define ROOT ""
#endif

#ifdef _DEBUG
#define ASSERT(x) \
  if (!(x)) __debugbreak();

#define GLCall(x) \
  GLClearError(); \
  x;              \
  ASSERT(GLLogCall(#x, __FILE__, __LINE__))
#else
#define ASSERT(x)
#define GLCall(x) x
#endif

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);

inline std::string getRootDir() { return std::string(ROOT); }

inline std::string getFilePath(const std::string& path) { return getRootDir() + path; }

using uint = unsigned int;
using uchar = unsigned char;

using std::make_unique;
using std::shared_ptr;
using std::unique_ptr;