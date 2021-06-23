#include "common.h"

void check_error(const char *file, int line) noexcept {
	auto error_code = glGetError();
	switch (error_code) {
	case GL_INVALID_ENUM:
		std::cerr << file << ": " << line << "枚举参数不合法";
		break;
	case GL_INVALID_VALUE:
		std::cerr << file << ": " << line << " 值参数不合法";
		break;
	case GL_INVALID_OPERATION:
		std::cerr << file << ": " << line << " 一个指令的状态对指令的参数不合法";
		break;
	case GL_STACK_OVERFLOW:
		std::cerr << file << ": " << line << " 压栈操作造成栈上溢(Overflow)";
		break;
	case GL_STACK_UNDERFLOW:
		std::cerr << file << ": " << line << " 弹栈操作时栈在最低点(Underflow)";
		break;
	case GL_OUT_OF_MEMORY:
		std::cerr << file << ": " << line << " 内存调用操作无法调用（足够的）内存";
		break;
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		std::cerr << file << ": " << line << " 读取或写入一个不完整的帧缓冲";
		break;
	default:
		break;
	}
}

GLFWwindow *create_window(int width, int height, const std::string &title) {
	GLFWwindow *window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	if (window == nullptr) {
		std::cerr << "Failed create glfw window" << std::endl;
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return nullptr;
	}

	glfwSetWindowPos(window, 500, 200);
	glViewport(0, 0, width, height);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);			// 隐藏鼠标的显示,适合摄像机
	glfwSetFramebufferSizeCallback(window, Scene::framebuffer_size_callback);
	glfwSetKeyCallback(window, Scene::key_callback);
	glfwSetCursorPosCallback(window, Scene::mouse_callback);
	glfwSetScrollCallback(window, Scene::scroll_callback);
	return window;
}

constexpr int g_width = 500;
constexpr int g_height = 500;
constexpr float g_aspect = float(g_width) / float(g_height);
constexpr float g_near = 0.1f;
constexpr float g_far = 1000.f;
constexpr float g_fov = 45.f;


void blinn_phong() {
	GLFWwindow *window = create_window(g_width, g_height, static_cast<std::string>(project_name));
	glm::vec3 look_from(-200, 0, 0);
	glm::vec3 look_up(0, 1, 0);
	glm::vec3 look_at(0, 0, 0);
	std::shared_ptr<Camera> camera_ptr = std::make_shared<Camera>(look_from, look_up, g_fov, g_near, g_far, 0.1f, 50.f);
	camera_ptr->set_aspect(g_aspect);
	Scene scene(window, camera_ptr, g_width, g_height, g_near, g_far);
	scene.blinn_phong();
	glfwDestroyWindow(window);
}

