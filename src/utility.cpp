#include "common.h"

void _check_error(const char *file, int line) noexcept {
	auto error_code = glGetError();
	switch (error_code) {
	case GL_INVALID_ENUM:
		std::cerr << file << ": " << line << "ö�ٲ������Ϸ�" << std::endl;
		break;
	case GL_INVALID_VALUE:
		std::cerr << file << ": " << line << " ֵ�������Ϸ�" << std::endl;
		break;
	case GL_INVALID_OPERATION:
		std::cerr << file << ": " << line << " һ��ָ���״̬��ָ��Ĳ������Ϸ�" << std::endl;
		break;
	case GL_STACK_OVERFLOW:
		std::cerr << file << ": " << line << " ѹջ�������ջ����(Overflow)" << std::endl;
		break;
	case GL_STACK_UNDERFLOW:
		std::cerr << file << ": " << line << " ��ջ����ʱջ����͵�(Underflow)" << std::endl;
		break;
	case GL_OUT_OF_MEMORY:
		std::cerr << file << ": " << line << " �ڴ���ò����޷����ã��㹻�ģ��ڴ�" << std::endl;
		break;
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		std::cerr << file << ": " << line << " ��ȡ��д��һ����������֡����" << std::endl;
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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);			// ����������ʾ,�ʺ������
	glfwSetFramebufferSizeCallback(window, Scene::framebuffer_size_callback);
	glfwSetKeyCallback(window, Scene::key_callback);
	glfwSetCursorPosCallback(window, Scene::mouse_callback);
	glfwSetScrollCallback(window, Scene::scroll_callback);
	return window;
}

constexpr int g_width = 600;
constexpr int g_height = 600;
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

void normal_mapping() {
	GLFWwindow *window = create_window(g_width, g_height, static_cast<std::string>(project_name));
	glm::vec3 look_from(-5, 0, 0);
	glm::vec3 look_up(0, 1, 0);
	glm::vec3 look_at(0, 0, 0);
	std::shared_ptr<Camera> camera_ptr = std::make_shared<Camera>(look_from, look_up, g_fov, g_near, g_far, 0.1f, 5.f);
	camera_ptr->set_aspect(g_aspect);
	Scene scene(window, camera_ptr, g_width, g_height, g_near, g_far);
	scene.normal_mapping();
	glfwDestroyWindow(window);
}

void parallax_mapping() {
	GLFWwindow *window = create_window(g_width, g_height, static_cast<std::string>(project_name));
	glm::vec3 look_from(-5, 0, 0);
	glm::vec3 look_up(0, 1, 0);
	glm::vec3 look_at(0, 0, 0);
	std::shared_ptr<Camera> camera_ptr = std::make_shared<Camera>(look_from, look_up, g_fov, g_near, g_far, 0.1f, 5.f);
	camera_ptr->set_aspect(g_aspect);
	Scene scene(window, camera_ptr, g_width, g_height, g_near, g_far);
	scene.parallax_mapping();
	glfwDestroyWindow(window);
}

void shadow_mapping() {
	GLFWwindow *window = create_window(g_width, g_height, static_cast<std::string>(project_name));
	glm::vec3 look_from(-5.f, 2.f, 0);
	glm::vec3 look_up(0, 1, 0);
	glm::vec3 look_at(0, -10, 0);
	std::shared_ptr<Camera> camera_ptr = std::make_shared<Camera>(look_from, look_up, g_fov, g_near, g_far, 0.1f, 5.f);
	camera_ptr->set_aspect(g_aspect);
	Scene scene(window, camera_ptr, g_width, g_height, g_near, g_far);
	scene.shadow_mapping();
	glfwDestroyWindow(window);
}

