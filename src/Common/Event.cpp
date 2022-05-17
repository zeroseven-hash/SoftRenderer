#include"Event.h"

#include<GLFW/glfw3.h>


namespace Input
{
	static GLFWwindow* s_context = nullptr;
	static EventCallBackFunc s_event_func;
	void Init(GLFWwindow* window, const EventCallBackFunc& func)
	{
		s_event_func = func;
		glfwSetWindowUserPointer(window, &s_event_func);
		s_context = window;
		glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x, double y) {

			EventCallBackFunc& function= *(EventCallBackFunc*)glfwGetWindowUserPointer(window);
			MouseMoveEvent e((float)x,(float)y);
			function((const Event*)&e);
		});


		glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
			const std::function<void(const Event*)>& func = *((const std::function<void(const Event*)>*)glfwGetWindowUserPointer(window));

			WindowResizeEvent e((uint32_t)width, (uint32_t)height);
			func((const Event*)&e);
		});

		glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
			const std::function<void(const Event*)>& func = *((const std::function<void(const Event*)>*)glfwGetWindowUserPointer(window));

			MouseScrollEvent e((float)yoffset);
			func((const Event*)&e);
		});
	}
	

	bool IsKeyPress(Key keycode)
	{
		return glfwGetKey(s_context, (int)keycode) == GLFW_PRESS;
	}

	bool IsMouseButton(MouseButton code)
	{
		return glfwGetMouseButton(s_context, (int)code) == GLFW_PRESS;
	}

}