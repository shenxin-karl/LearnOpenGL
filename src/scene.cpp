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
	blinn_phong_shader.set_uniform("diffuse_map1", 0);

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

void Scene::normal_mapping() {
	std::unique_ptr<Model> plane_ptr = Loader::create_test_plane();
	GLuint diffuse_map = Loader::load_texture2d("resources/test_plane/brickwall.jpg");
	GLuint normal_map = Loader::load_texture2d("resources/test_plane/brickwall_normal.jpg");
	Shader normal_mapping_shader("shader/normal_mapping/normal_mapping.vert", "shader/normal_mapping/normal_mapping.frag");
	if (!normal_mapping_shader) {
		std::cerr << "Failed initialize normal_mapping_shader" << std::endl;
		return;
	}

	glm::vec3 light_dir = glm::normalize(glm::vec3(0.5f, 0.5f, 0.f));
	normal_mapping_shader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuse_map);
	normal_mapping_shader.set_uniform("diffuse_map1", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normal_map);
	normal_mapping_shader.set_uniform("normal_map1", 1);

	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window)) {
		poll_event();
		glClearColor(0.1f, 0.1f, 0.1f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		normal_mapping_shader.use();
		normal_mapping_shader.set_uniform("model", glm::rotate(glm::mat4(1.f), glm::radians(-70.f), glm::vec3(1, 0, 0)));
		normal_mapping_shader.set_uniform("view", camera_ptr->get_view());
		normal_mapping_shader.set_uniform("projection", camera_ptr->get_projection());
		normal_mapping_shader.set_uniform("eye_pos", camera_ptr->get_look_from());

		glm::mat4 light_rotate = glm::rotate(glm::mat4(1.f), float(glm::radians(glfwGetTime() * 5.f)), glm::vec3(0, 1, 0));
		glm::vec3 new_light_dir = light_rotate * glm::vec4(light_dir, 1.f);
		normal_mapping_shader.set_uniform("light_dir", new_light_dir);
		plane_ptr->draw(normal_mapping_shader);
		CheckError();
		glfwSwapBuffers(window);
	}
}

void Scene::parallax_mapping() {
	std::unique_ptr<Model> plane_ptr = Loader::create_test_plane();
	GLuint diffuse_map = Loader::load_texture2ds("resources/test_plane/bricks2.jpg");
	GLuint normal_map = Loader::load_texture2d("resources/test_plane/bricks2_normal.jpg");
	GLuint displacement_map = Loader::load_texture2d("resources/test_plane/bricks2_disp.jpg");
	Shader parallax_mapping_shader("shader/parallax_mapping/parallax_mapping.vert", "shader/parallax_mapping/parallax_mapping.frag");
	if (!parallax_mapping_shader) {
		std::cerr << "Failed initialize parallax_mapping_shader" << std::endl;
		return;
	}

	glm::vec3 light_dir = glm::normalize(glm::vec3(0.5f, 0.5f, 0.f));
	parallax_mapping_shader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuse_map);
	parallax_mapping_shader.set_uniform("diffuse_map1", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normal_map);
	parallax_mapping_shader.set_uniform("normal_map1", 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, displacement_map);
	parallax_mapping_shader.set_uniform("displacement_map1", 2);
	parallax_mapping_shader.set_uniform("displacement_scale", 0.1f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	while (!glfwWindowShouldClose(window)) {
		poll_event();
		glClearColor(0.0f, 0.0f, 0.0f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		parallax_mapping_shader.use();
		//parallax_mapping_shader.set_uniform("model", glm::rotate(glm::mat4(1.f), glm::radians(-70.f), glm::vec3(1, 0, 0)));
		parallax_mapping_shader.set_uniform("model", glm::mat4(1.f));
		parallax_mapping_shader.set_uniform("view", camera_ptr->get_view());
		parallax_mapping_shader.set_uniform("projection", camera_ptr->get_projection());
		parallax_mapping_shader.set_uniform("eye_pos", camera_ptr->get_look_from());

		glm::mat4 light_rotate = glm::rotate(glm::mat4(1.f), float(glm::radians(glfwGetTime() * 5.f)), glm::vec3(0, 1, 0));
		glm::vec3 new_light_dir = light_rotate * glm::vec4(light_dir, 1.f);
		parallax_mapping_shader.set_uniform("light_dir", new_light_dir);
		plane_ptr->draw(parallax_mapping_shader);
		CheckError();
		glfwSwapBuffers(window);
	}
}

void Scene::shadow_mapping() {
	std::unique_ptr<Model> plane_ptr = Loader::create_test_plane();
	std::unique_ptr<Model> cube_ptr = Loader::create_trest_cube();
	GLuint plane_diffuse_map = Loader::load_texture2ds("resources/test_plane/wood.png");
	GLuint cube_diffuse_map = Loader::load_texture2ds("resources/test_cube/container2.png");

	Shader shadow_mapping_shader("shader/shadow_mapping/shadow_mapping.vert", "shader/shadow_mapping/shadow_mapping.frag");
	if (!shadow_mapping_shader) {
		std::cerr << "Failed initialize shadow_mapping_shader" << std::endl;
		return;
	}

	Shader sample_depth_shader("shader/shadow_mapping/sample_depth.vert", "shader/shadow_mapping/sample_depth.frag");
	if (!sample_depth_shader) {
		std::cerr << "Failed initialize sample_depth_shader" << std::endl;
		return;
	}

	shadow_mapping_shader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, plane_diffuse_map);
	glm::vec3 light_dir = glm::normalize(glm::vec3(-19.707130, 12.483460, -17.730694));

	glm::mat4 plane_scale = glm::scale(glm::mat4(1.f), glm::vec3(10.f, 1, 10.f));
	glm::mat4 plane_trans_matrix = glm::rotate(plane_scale, glm::radians(90.f), glm::vec3(1, 0, 0));
	glm::mat4 cube_trans = glm::translate(glm::mat4(1.f), glm::vec3(0, 1, 0));

	// 构建 fbo
	constexpr int SHADOW_WIDTH = 1024;
	constexpr int SHADOW_HEIGHT = 1024;
	GLuint depth_fbo;
	GLuint depth_map;
	glGenFramebuffers(1, &depth_fbo);
	glGenTextures(1, &depth_map);
	glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
	{
		// 为纹理对象分配内存
		glBindTexture(GL_TEXTURE_2D, depth_map);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "depth_fbo is not complete!" << std::endl;
			return;
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);	
	while (!glfwWindowShouldClose(window)) {
		poll_event();
		glClearColor(0.f, 0.f, 0.f, 1.f);

		// 构建光空间矩阵
		constexpr float near = 10.f;
		constexpr float far = 100.f;
		constexpr float left = -20.f;
		constexpr float right = 20.f;
		constexpr float top = 20.f;
		constexpr float bottom = -20.f;
		glm::mat4 light_view = glm::lookAt(light_dir * 30.f, glm::vec3(0), glm::vec3(0, 1, 0));
		glm::mat4 light_projection = glm::ortho(left, right, bottom, top, near, far);
		glm::mat4 light_space_matrix = light_projection * light_view;
		// 渲染深度贴图
		glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
		{
			glCullFace(GL_FRONT);		// 渲染场景深度贴图使用正面剔除, 避免悬浮问题出现
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glClear(GL_DEPTH_BUFFER_BIT);

			sample_depth_shader.use();
			sample_depth_shader.set_uniform("light_space_matrix", light_space_matrix);
			sample_depth_shader.set_uniform("model", plane_trans_matrix);
			plane_ptr->draw(sample_depth_shader);

			sample_depth_shader.set_uniform("model", cube_trans);
			cube_ptr->draw(sample_depth_shader);
			glCullFace(GL_BACK);		// 恢复背面剔除
		}

	// 绘制阴影场景
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, plane_diffuse_map);
		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depth_map);

		shadow_mapping_shader.use();
		shadow_mapping_shader.set_uniform("diffuse_map1", 0);
		shadow_mapping_shader.set_uniform("depth_map", 1);
		shadow_mapping_shader.set_uniform("light_space_matrix", light_space_matrix);

		shadow_mapping_shader.set_uniform("eye_pos", camera_ptr->get_look_from());
		shadow_mapping_shader.set_uniform("view", camera_ptr->get_view());
		shadow_mapping_shader.set_uniform("projection", camera_ptr->get_projection());
		shadow_mapping_shader.set_uniform("light_dir", light_dir);

		shadow_mapping_shader.set_uniform("model", plane_trans_matrix);
		plane_ptr->draw(shadow_mapping_shader);

		shadow_mapping_shader.set_uniform("model", cube_trans);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cube_diffuse_map);
		shadow_mapping_shader.set_uniform("diffuse_map1", 0);
		cube_ptr->draw(shadow_mapping_shader);

		glfwSwapBuffers(window);
	}

	glDeleteFramebuffers(1, &depth_fbo);
	glDeleteTextures(1, &depth_map);
}

