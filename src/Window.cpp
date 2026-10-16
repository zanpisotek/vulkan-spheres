//
// Created by zanpisotek on 10/15/26.
//
#include "Window.h"

#include <stdexcept>


Window::Window(uint32_t width, uint32_t height, const std::string& titile) {
  glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
  int result = glfwInit();
  if (!result) {
    throw std::runtime_error("Failed to initialize GLFW");
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  m_Window = glfwCreateWindow(width, height, titile.c_str(), nullptr, nullptr);
  if (!m_Window) {
    throw std::runtime_error("Failed to create GLFW window");
  }
}
Window::~Window() {
  if (m_Window) {
    glfwDestroyWindow(m_Window);
  }
  glfwTerminate();
}

bool Window::IsOpen() { return !glfwWindowShouldClose(m_Window); }
void Window::PollEvents() { glfwPollEvents(); }
void Window::WaitEvents() { glfwWaitEvents(); }