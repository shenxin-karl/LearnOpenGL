#include "common.h"
#include <Windows.h>

void init() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	std::string curr_path = std::filesystem::current_path().string();
	if (auto pos = curr_path.find(project_name); pos != std::string::npos) {
		std::string new_path = curr_path.substr(0, pos + project_name.size() + 1);
		SetCurrentDirectory(new_path.c_str());
	}
}

void destroy() {
	Loader::destroy();
	// imgui Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
}


int main(void) {
	init();

	GLFWwindow *window = create_window(1200, 800, static_cast<std::string>(project_name));
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	const char *glsl_version = "#version 130";
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	std::function<void(GLFWwindow *window)> callback;
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("select demo");
		{
			if (ImGui::Button("blinn-phong"))
				callback = blinn_phong;
			if (ImGui::Button("normal_mapping"))
				callback = normal_mapping;
			if (ImGui::Button("parallax_mapping"))
				callback = parallax_mapping;
			if (ImGui::Button("shadow_mapping"))
				callback = shadow_mapping;
			if (ImGui::Button("point_shadow"))
				callback = point_shadow;
			if (ImGui::Button("pbr"))
				callback = pbr;
			if (ImGui::Button("AK47"))
				callback = AK47;
			if (ImGui::Button("planet"))
				callback = planet;
			if (ImGui::Button("deferred_shading"))
				callback = deferred_shading;
		}
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);

		if (callback != nullptr) {
			callback(window);
			break;
		}
	}
	destroy();
}