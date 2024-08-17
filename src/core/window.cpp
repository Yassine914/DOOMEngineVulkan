#include "window.h"

#include "logger.h"

i32 Window::width = DEF_WIDTH, Window::height = DEF_HEIGHT;
Joystick Window::mainJoystick(0);

void Window::ErrorCallback(i32 error, const char *description)
{
    LERROR(description << "\n");
}

void Window::OnWindowResize(GLFWwindow *window, i32 width, i32 height)
{
    // TODO: resize with vulkan
    // glViewport(0, 0, width, height);
    Window::SetWindowSize(width, height);
}

void Window::InitializeWindow()
{
    // initialize glfw.
    if(!glfwInit())
    {
        ErrorCallback(1, "failed to init GLFW.");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // TODO: handle vulkan viewport resizing.
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    glfwSetErrorCallback(ErrorCallback);

    Window::monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *screen = glfwGetVideoMode(monitor);
    Window::monitorWidth = screen->width;
    Window::monitorHeight = screen->width;

    // clang-format off
    if(fullscreen)
    {
        SetWindowSize(monitorWidth, monitorHeight);

        window = glfwCreateWindow(
            monitorWidth, monitorHeight,
            GetTitle().c_str(),
            monitor, nullptr);
    }
    else
    {
        window = glfwCreateWindow(
            GetWidth(), GetHeight(), 
            GetTitle().c_str(), 
            nullptr, nullptr);
    }
    // clang-format on

    u32 extentionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extentionCount, nullptr);
    LINFO(true, "VulkanInstance Extension Count: " << extentionCount << "\n");

    if(!window)
    {
        ErrorCallback(1, "window was not correctly initialized");
        glfwTerminate();
    }

    // TODO: setup vsync using vulkan.
    // if(VSyncEnabled())
    // glfwSwapInterval(1);

    // if(resizeable)
    // {
    //     glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    //     glfwSetFramebufferSizeCallback(window, OnWindowResize);
    // }

    // input callbacks
    glfwSetKeyCallback(window, Keyboard::KeyCallback);

    glfwSetCursorPosCallback(window, Mouse::CursorPosCallback);
    glfwSetMouseButtonCallback(window, Mouse::MouseButtonCallback);
    glfwSetScrollCallback(window, Mouse::MouseWheelCallback);

    // joystick initialization
    mainJoystick.Update();

    // if(mainJoystick.IsPresent())
    // Log(LOG_INFO) << "joystick " << mainJoystick.GetName() << " is present\n";
    // else
    // Log(LOG_INFO) << "no joystick found.\n";
}

Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}