#include "common.h"
#include <Windows.h>


int main(void) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	std::string curr_path = std::filesystem::current_path().string();
	if (auto pos = curr_path.find(project_name); pos != std::string::npos) {
		std::string new_path = curr_path.substr(0, pos + project_name.size() + 1);
		SetCurrentDirectory(new_path.c_str());
	}

	//blinn_phong();
	//normal_mapping();
	parallax_mapping();
}
