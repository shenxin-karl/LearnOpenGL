#include "common.h"

Scene::Scene(GLFWwindow *_window, std::shared_ptr<Camera> _camera_ptr, int _widht, int _hegith, float _near, float _far)
: window(_window), camera_ptr(_camera_ptr), width(_widht), height(_hegith), near(_near), far(_far) {
	scene_mapping.insert(std::make_pair(window, this));
}

Scene::~Scene() {
	scene_mapping.erase(window);
}

void Scene::process_key_callback(int key, int scancode, int action, int mode) {
	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;

	if (keys[GLFW_KEY_ESCAPE])
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	
	if (on_key_callback != nullptr)
		on_key_callback(key, scancode, action, mode);
}

void Scene::process_mouse_callback(double xpos, double ypos) {
	camera_ptr->mouse_callback(xpos, ypos);
	if (on_mouse_callback != nullptr)
		on_mouse_callback(xpos, ypos);
}

void Scene::process_scroll_callback(double xoffset, double yoffset) {
	camera_ptr->scroll_callback(yoffset);
	if (on_scroll_callback != nullptr)
		on_scroll_callback(xoffset, yoffset);
}

void Scene::process_framebuffer_size_callback(int width, int height) {
	this->width = width;
	this->height = height;
	camera_ptr->framebuff_callback(width, height);
	if (on_frambuffer_size_callback != nullptr)
		on_frambuffer_size_callback(width, height);
}

void Scene::poll_event() {
	glfwPollEvents();

	float curr_time = static_cast<float>(glfwGetTime());
	float delta_time = curr_time - last_time;
	last_time = curr_time;
	static std::unordered_map<int, CameraKey> camera_key_map = {
		{ GLFW_KEY_A, CameraKey::Left },
		{ GLFW_KEY_D, CameraKey::Right },
		{ GLFW_KEY_W, CameraKey::Forward },
		{ GLFW_KEY_S, CameraKey::BackWard },
	};
	
	for (auto &&[key, camera_key] : camera_key_map) {
		if (keys[key])
			camera_ptr->key_callback(camera_key, delta_time);
	}

	++fps;
	if (curr_time > next_time) {
		std::string new_title = std::string(project_name) + ": " + std::to_string(fps);
		glfwSetWindowTitle(window, new_title.c_str());
		next_time = static_cast<int>(curr_time) + 1;
		fps = 0;
	}
}

void Scene::key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
	if (auto iter = scene_mapping.find(window); iter != scene_mapping.end())
		iter->second->process_key_callback(key, scancode, action, mode);
}

void Scene::mouse_callback(GLFWwindow *window, double xpos, double ypos) {
	if (auto iter = scene_mapping.find(window); iter != scene_mapping.end())
		iter->second->process_mouse_callback(xpos, ypos);
}

void Scene::scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
	if (auto iter = scene_mapping.find(window); iter != scene_mapping.end())
		iter->second->process_scroll_callback(xoffset, yoffset);
}

void Scene::framebuffer_size_callback(GLFWwindow *window, int width, int height) {
	glViewport(0, 0, width, height);
	if (auto iter = scene_mapping.find(window); iter != scene_mapping.end())
		iter->second->process_framebuffer_size_callback(width, height);
}

void Scene::blinn_phong() {
	std::unique_ptr<Model> model_ptr = Loader::load_model("resources/alod/dino obj.obj");
	GLuint diffuse_map = Loader::load_texture2d("resources/alod/dino.jpg");

	Shader blinn_phong_shader("shader/blinn_phong/blinn_phong.vert", "shader/blinn_phong/blinn_phong.frag");
	if (!blinn_phong_shader) {
		std::cerr << "Failed initialize blinn_phong_shader" << std::endl;
		return;
	}

	glm::vec3 light_dir = glm::normalize(glm::vec3(0.5f, 0.5f, 0.f));
	blinn_phong_shader.use();
	glBindTexture(GL_TEXTURE_2D, diffuse_map);
	blinn_phong_shader.set_uniform("diffuse_map1", diffuse_map);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	while (!glfwWindowShouldClose(window)) {
		poll_event();
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		blinn_phong_shader.use();
		blinn_phong_shader.set_uniform("model", glm::mat4(1.f));
		blinn_phong_shader.set_uniform("view", camera_ptr->get_view());
		blinn_phong_shader.set_uniform("projection", camera_ptr->get_projection());
		blinn_phong_shader.set_uniform("light_dir", light_dir);
		blinn_phong_shader.set_uniform("eye_pos", camera_ptr->get_look_from());
		model_ptr->draw(blinn_phong_shader);
		glfwSwapBuffers(window);
	}
}

