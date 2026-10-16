//
// Created by zanpisotek on 10/15/26.
//

#pragma once
#include <GLFW/glfw3.h>
#include <string>

class Window {
public:
  Window(uint32_t width, uint32_t height, const std::string& title);
  ~Window();
  bool IsOpen();
  void PollEvents();
  void WaitEvents();

  GLFWwindow* GetHandle()const { return m_Window;}
private:
  GLFWwindow* m_Window = nullptr;
};

