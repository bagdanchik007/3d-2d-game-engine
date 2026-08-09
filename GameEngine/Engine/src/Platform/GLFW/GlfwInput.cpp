#include "Engine/Core/Input.h"

#include "Engine/Core/Application.h"

#include <GLFW/glfw3.h>

namespace Engine
{
    bool Input::IsKeyPressed(KeyCode key)
    {
        auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        const int state = glfwGetKey(window, static_cast<int>(key));
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::IsMouseButtonPressed(MouseCode button)
    {
        auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        const int state = glfwGetMouseButton(window, static_cast<int>(button));
        return state == GLFW_PRESS;
    }

    std::pair<float, float> Input::GetMousePosition()
    {
        auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        double x = 0.0;
        double y = 0.0;
        glfwGetCursorPos(window, &x, &y);
        return {static_cast<float>(x), static_cast<float>(y)};
    }

    float Input::GetMouseX()
    {
        return GetMousePosition().first;
    }

    float Input::GetMouseY()
    {
        return GetMousePosition().second;
    }

} // namespace Engine
