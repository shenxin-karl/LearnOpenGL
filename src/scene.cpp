#include "common.h"

Scene::Scene(GLFWwindow *_window, std::shared_ptr<Camera> _camera_ptr, int _widht, int _hegith, float _near, float _far)
: window(_window), camera_ptr(_camera_ptr), width(_widht), height(_hegith), near(_near), far(_far)
{
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
	if (width == 0 || height == 0) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		return;
	}

	this->width = width;
	this->height = height;
	camera_ptr->framebuff_callback(width, height);
	if (on_frambuffer_size_callback != nullptr)
		on_frambuffer_size_callback(width, height);
}


void Scene::add_model(std::shared_ptr<Model> model_ptr) {
	models_.push_back(model_ptr);
	model_flags_.push_back(0);
}

void Scene::scene_widgets() {
	ImGui::Begin("scene");
	{
		ImGui::Checkbox("camera", &show_camera_widgets_);
		//for (int i = 0; i < models_.size(); ++i)
		//	ImGui::Checkbox(models_[i]->get_name().c_str(), reinterpret_cast<bool *>(&model_flags_[i]));
	}
	ImGui::End();

	if (show_camera_widgets_)
		camera_ptr->camera_widgets();

	for (int i = 0; i < models_.size(); ++i) {
		if (model_flags_[i])
			models_[i]->show_widgets();
	}
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

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}


void Scene::swap_buffer() {
	scene_widgets();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(window);
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
	std::shared_ptr<Model> model_ptr = Loader::load_model("resources/alod/dino obj.obj");
	add_model(model_ptr);
	GLuint diffuse_map = Loader::load_texture2d("resources/alod/dino.jpg");
	Shader blinn_phong_shader("shader/blinn_phong/blinn_phong.vert", "shader/blinn_phong/blinn_phong.frag");
	if (!blinn_phong_shader) {
		std::cerr << "Failed initialize blinn_phong_shader" << std::endl;
		return;
	}

	blinn_phong_shader.use();
	glBindTexture(GL_TEXTURE_2D, diffuse_map);
	blinn_phong_shader.set_uniform("diffuse_map1", 0);

	glm::vec3 light_dir = glm::normalize(glm::vec3(0.5f, 0.5f, 0.f));
	glm::vec3 light_ambient = glm::vec3(0.3f);
	glm::vec3 light_diffuse = glm::vec3(0.5f);
	glm::vec3 light_specular = glm::vec3(0.2f);
	float shininess = 64.f;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	while (!glfwWindowShouldClose(window)) {
		poll_event();
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		blinn_phong_shader.use();
		blinn_phong_shader.set_uniform("model", glm::mat4(1));
		blinn_phong_shader.set_uniform("view", camera_ptr->get_view());
		blinn_phong_shader.set_uniform("projection", camera_ptr->get_projection());
		blinn_phong_shader.set_uniform("light_dir", light_dir);
		blinn_phong_shader.set_uniform("eye_pos", camera_ptr->get_look_from());
		
		light_dir = glm::normalize(light_dir);
		blinn_phong_shader.set_uniform("light.light_dir", light_dir);
		blinn_phong_shader.set_uniform("light.ambient", light_ambient);
		blinn_phong_shader.set_uniform("light.diffuse", light_diffuse);
		blinn_phong_shader.set_uniform("light.specular", light_specular);
		blinn_phong_shader.set_uniform("light.shininess", shininess);
		model_ptr->draw(blinn_phong_shader);

		ImGui::Begin("Light");
		{
			ImGui::ColorEdit3("ambient", glm::value_ptr(light_ambient));
			ImGui::ColorEdit3("diffuse", glm::value_ptr(light_diffuse));
			ImGui::ColorEdit3("specular", glm::value_ptr(light_specular));
			ImGui::DragFloat("shininess", &shininess, 1.f, 1.f, 256.f);
		}
		ImGui::End();
		swap_buffer();
	}
}

void Scene::normal_mapping() {
	std::shared_ptr<Model> plane_ptr = Loader::create_test_plane();
	GLuint diffuse_map = Loader::load_texture2d("resources/test_plane/brickwall.jpg");
	GLuint normal_map = Loader::load_texture2d("resources/test_plane/brickwall_normal.jpg");
	Shader normal_mapping_shader("shader/normal_mapping/normal_mapping.vert", "shader/normal_mapping/normal_mapping.frag");
	if (!normal_mapping_shader) {
		std::cerr << "Failed initialize normal_mapping_shader" << std::endl;
		return;
	}

	glm::vec3 light_dir = glm::normalize(glm::vec3(0.f, -1.f, -0.5f));
	normal_mapping_shader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuse_map);
	normal_mapping_shader.set_uniform("diffuse_map1", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normal_map);
	normal_mapping_shader.set_uniform("normal_map1", 1);

	float bump_scale = 1.0;
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
		normal_mapping_shader.set_uniform("bump_scale", bump_scale);

		glm::mat4 light_rotate = glm::rotate(glm::mat4(1.f), float(glm::radians(glfwGetTime() * 5.f)), glm::vec3(0, 1, 0));
		glm::vec3 new_light_dir = light_rotate * glm::vec4(light_dir, 1.f);
		normal_mapping_shader.set_uniform("light_dir", light_dir);
		plane_ptr->draw(normal_mapping_shader);

		ImGui::Begin("normal mapping");
		{
			ImGui::InputFloat("bump scale", &bump_scale);
		}
		ImGui::End();

		swap_buffer();
	}
}

void Scene::parallax_mapping() {
	std::shared_ptr<Model> plane_ptr = Loader::create_test_plane();
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
	float displacement_scale = 0.1f;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	while (!glfwWindowShouldClose(window)) {
		poll_event();
		glClearColor(0.0f, 0.0f, 0.0f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		parallax_mapping_shader.use();
		parallax_mapping_shader.set_uniform("model", glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1, 0, 0)));
		//parallax_mapping_shader.set_uniform("model", glm::mat4(1.f));
		parallax_mapping_shader.set_uniform("view", camera_ptr->get_view());
		parallax_mapping_shader.set_uniform("projection", camera_ptr->get_projection());
		parallax_mapping_shader.set_uniform("eye_pos", camera_ptr->get_look_from());
		parallax_mapping_shader.set_uniform("displacement_scale", displacement_scale);

		glm::mat4 light_rotate = glm::rotate(glm::mat4(1.f), float(glm::radians(glfwGetTime() * 5.f)), glm::vec3(0, 1, 0));
		glm::vec3 new_light_dir = light_rotate * glm::vec4(light_dir, 1.f);
		parallax_mapping_shader.set_uniform("light_dir", new_light_dir);
		plane_ptr->draw(parallax_mapping_shader);

		ImGui::Begin("parallax_mapping param");
		{
			ImGui::DragFloat("displacement_scale", &displacement_scale, 0.01f, 0.f, 0.3f);
		}
		ImGui::End();

		swap_buffer();
	}
}

void Scene::shadow_mapping() {
	std::shared_ptr<Model> plane_ptr = Loader::create_test_plane();
	std::shared_ptr<Model> cube_ptr = Loader::create_trest_cube();
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

	int pcf_kernel_size = 1;
	bool is_shadow = true;

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
		shadow_mapping_shader.set_uniform("pcf_kernel_size", pcf_kernel_size);
		shadow_mapping_shader.set_uniform("is_shadow", is_shadow);

		shadow_mapping_shader.set_uniform("model", plane_trans_matrix);
		plane_ptr->draw(shadow_mapping_shader);

		shadow_mapping_shader.set_uniform("model", cube_trans);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cube_diffuse_map);
		shadow_mapping_shader.set_uniform("diffuse_map1", 0);
		cube_ptr->draw(shadow_mapping_shader);

		ImGui::Begin("shadow_mapping param");
		{
			ImGui::DragInt("pcf size", &pcf_kernel_size, 1, 1, 5);
			ImGui::Checkbox("shadow", &is_shadow);
		}
		ImGui::End();

		swap_buffer();
	}

	glDeleteFramebuffers(1, &depth_fbo);
	glDeleteTextures(1, &depth_map);
}

void Scene::bloom() {
	std::shared_ptr<Model> plane_ptr = Loader::create_test_plane();
	std::shared_ptr<Model> cube_ptr = Loader::create_trest_cube();
	std::shared_ptr<Model> quad_ptr = Loader::create_quad();
	GLuint plane_diffuse_map = Loader::load_texture2ds("resources/test_plane/wood.png");
	GLuint cube_diffuse_map = Loader::load_texture2ds("resources/test_cube/container2.png");
	
	Shader single_color_shader("shader/bloom/single_color.vert", "shader/bloom/single_color.frag");
	if (!single_color_shader) {
		std::cerr << "Failed initialize single_color_shader" << std::endl;
		return;
	}

	Shader bloom_shader("shader/bloom/bloom.vert", "shader/bloom/bloom.frag");
	if (!bloom_shader) {
		std::cerr << "Failed initialize bloom_shader" << std::endl;
		return;
	}

	Shader bloom_final_shader("shader/bloom/bloom_final.vert", "shader/bloom/bloom_final.frag");
	if (!bloom_final_shader) {
		std::cerr << "Failed initialize bloom_final_shader" << std::endl;
		return;
	}

	Shader blur_shader("shader/bloom/blur.vert", "shader/bloom/blur.frag");
	if (!blur_shader) {
		std::cerr << "Failed initialize blur_shader" << std::endl;
		return;
	}
	
	glm::mat4 light_scale = glm::scale(glm::mat4(1), glm::vec3(1.f, 1.f, 1.f));
	std::vector<glm::vec3> light_pos = {
		glm::vec3(2, 3, 5),
		glm::vec3(-1, 4, 2),
		glm::vec3(-5, 5, 0),
		glm::vec3(4, 4, -3),
	};
	std::vector<glm::vec3> light_color = {
		glm::vec3(5.0f,   5.0f,  5.0f),
		glm::vec3(10.0f,  0.0f,  0.0f),
		glm::vec3(0.0f,   0.0f,  15.0f),
		glm::vec3(0.0f,   5.0f,  0.0f),
	};

	glm::mat4 plane_scale = glm::scale(glm::mat4(1.0), glm::vec3(10, 1, 10));
	glm::mat4 plane_trans = glm::rotate(plane_scale, glm::radians(90.f), glm::vec3(1, 0, 0));

	glm::mat4 cube_scale = glm::scale(glm::mat4(1), glm::vec3(2, 2, 2));
	std::vector<glm::vec3> cube_pos = {
		glm::vec3(0, 0.5, 0),
		glm::vec3(1, 0.5, 4),
		glm::vec3(4, 0.5, 1),
		glm::vec3(-4, 0.5, -1),
	};

	GLuint fbo;
	GLuint rbo;
	GLuint screen_texture[2];
	glGenFramebuffers(1, &fbo);
	glGenTextures(2, screen_texture);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	{
		for (int i = 0; i < 2; ++i) {
			glBindTexture(GL_TEXTURE_2D, screen_texture[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, screen_texture[i], 0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
								  GL_RENDERBUFFER, rbo);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
			return;
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLuint pingpong_fbo[2];
	GLuint pingpong_buffer[2];
	GLuint pingpong_rbo[2];
	glGenFramebuffers(2, pingpong_fbo);
	glGenRenderbuffers(2, pingpong_rbo);
	glGenTextures(2, pingpong_buffer);
	for (int i = 0; i < 2; ++i) {
		glBindFramebuffer(GL_FRAMEBUFFER, pingpong_fbo[i]);
		glBindTexture(GL_TEXTURE_2D, pingpong_buffer[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpong_buffer[i], 0);
		glBindRenderbuffer(GL_RENDERBUFFER, pingpong_rbo[i]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
			GL_RENDERBUFFER, pingpong_rbo[i]);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
			return;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	glEnable(GL_MULTISAMPLE);	
	glEnable(GL_CULL_FACE);
	while (!glfwWindowShouldClose(window)) {
		poll_event();

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// render light cube
		GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);
		single_color_shader.use();
		single_color_shader.set_uniform("view", camera_ptr->get_view());
		single_color_shader.set_uniform("projection", camera_ptr->get_projection());
		for (int i = 0; i < 4; ++i) {
			glm::mat4 model = glm::translate(light_scale, light_pos[i]);
			single_color_shader.set_uniform("model", model);
			single_color_shader.set_uniform("light_color", light_color[i]);
			cube_ptr->draw(single_color_shader);
		}

		bloom_shader.use();
		bloom_shader.set_uniform("model", plane_trans);
		bloom_shader.set_uniform("view", camera_ptr->get_view());
		bloom_shader.set_uniform("projection", camera_ptr->get_projection());
		bloom_shader.set_uniform("eye_pos", camera_ptr->get_look_from());
		bloom_shader.set_uniform("diffuse_map1", 0);
		for (int i = 0; i < 4; ++i) {
			std::string light_pos_var = std::format("light_pos[{}]", i);
			std::string light_color_var = std::format("light_color[{}]", i);
			bloom_shader.set_uniform(light_pos_var.c_str(), light_pos[i]);
			bloom_shader.set_uniform(light_color_var.c_str(), light_color[i]);
		}

		// render plane
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, plane_diffuse_map);
		plane_ptr->draw(bloom_shader);

		// render cube
		glBindTexture(GL_TEXTURE_2D, cube_diffuse_map);
		bloom_shader.set_uniform("diffuse_map1", 0);
		for (int i = 0; i < cube_pos.size(); ++i) {
			glm::mat4 model = glm::translate(cube_scale, cube_pos[i]);
			bloom_shader.set_uniform("model", model);
			cube_ptr->draw(bloom_shader);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glDisable(GL_DEPTH_TEST);
		// 高斯模糊
		blur_shader.use();
		GLuint blur_texture = screen_texture[1];
		for (int i = 0; i < 10; ++i) {
			int curr = i % 2;
			glBindFramebuffer(GL_FRAMEBUFFER, pingpong_fbo[curr]);
			glDisable(GL_DEPTH_TEST);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, blur_texture);
			blur_shader.set_uniform("color_map", 0);
			blur_shader.set_uniform("horizontal", curr);
			quad_ptr->draw(blur_shader);
			blur_texture = pingpong_buffer[curr];
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		// 绑定默认的帧缓冲
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		bloom_final_shader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, screen_texture[0]);
		bloom_final_shader.set_uniform("color_map", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, blur_texture);
		bloom_final_shader.set_uniform("birght_color_map", 1);
		
		quad_ptr->draw(bloom_final_shader);
		glfwSwapBuffers(window);
	}

	glDeleteFramebuffers(1, &fbo);
	glDeleteRenderbuffers(1, &rbo);
	glDeleteTextures(2, screen_texture);
	glDeleteFramebuffers(2, pingpong_fbo);
	glDeleteTextures(2, pingpong_buffer);
	glDeleteRenderbuffers(2, pingpong_rbo);
}

void Scene::pbr() {
	GLuint hdr_cube_map = Loader::equirectangular_to_cube_map("resources/skybox/Barce_Rooftop_C_3k.hdr");
	GLuint irradiance_env_map = Loader::irradiance_convolution(hdr_cube_map);
	GLuint prefilter_map = Loader::prefilter(hdr_cube_map);
	GLuint brdf_lut = Loader::brdf_lut(hdr_cube_map);
	auto sphere_ptr = Loader::create_sphere();
	auto skybox_cube_ptr = Loader::create_skybox();
	//add_model(sphere_ptr);

	Shader pbr_shader("shader/pbr/pbr.vert", "shader/pbr/pbr.frag");
	if (!pbr_shader) {
		std::cerr << "Failed initialize pbr_shader" << std::endl;
		return;
	}

	Shader skybox_shader("shader/skybox/skybox.vert", "shader/skybox/skybox.frag");
	if (!skybox_shader) {
		std::cerr << "Failed initialize skybox_shader" << std::endl;
		return;
	}

	constexpr int light_size = 4;
	glm::vec3 light_position[light_size] = {
        glm::vec3(-10.0f,  10.0f, 10.0f),
        glm::vec3( 10.0f,  10.0f, 10.0f),
        glm::vec3(-10.0f, -10.0f, 10.0f),
        glm::vec3( 10.0f, -10.0f, 10.0f),
	};
	glm::vec3 light_colors[light_size] = {
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
	};

	glm::vec3 albedo = glm::vec3(0.3f);
	float metallic = 0.0f;
	float roughness = 0.0f;

	int nrRows = 7;
	int nrColumns = 7;
	float spacing = 2.5;
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
	while (!glfwWindowShouldClose(window)) {
		poll_event();
		glViewport(0, 0, width, height);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		pbr_shader.use();
		pbr_shader.set_uniform("model", glm::mat4(1));
		pbr_shader.set_uniform("view_pos", camera_ptr->get_look_from());
		pbr_shader.set_uniform("view", camera_ptr->get_view());
		pbr_shader.set_uniform("projection", camera_ptr->get_projection());
		pbr_shader.set_uniform("albedo", albedo);
		pbr_shader.set_uniform("metallic", metallic);
		pbr_shader.set_uniform("roughness", roughness);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_env_map);
		pbr_shader.set_uniform("irradiance_env_map", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_map);
		pbr_shader.set_uniform("prefilter_map", 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, brdf_lut);
		pbr_shader.set_uniform("brdf_lut_map", 2);
		for (int i = 0; i < light_size; ++i) {
			std::string var_pos = std::format("lights[{}].position", i);
			std::string var_col = std::format("lights[{}].color", i);
			pbr_shader.set_uniform(var_pos.c_str(), light_position[i]);
			pbr_shader.set_uniform(var_col.c_str(), light_colors[i]);
		}
#if 1
		sphere_ptr->draw(pbr_shader);
#else
		glm::mat4 model = glm::mat4(1.0f);
		for (int row = 0; row < nrRows; ++row) {
			pbr_shader.set_uniform("metallic", float(row) / float(nrRows));
			for (int col = 0; col < nrColumns; ++col) {
				pbr_shader.set_uniform("roughness", glm::clamp(float(col) / float(nrColumns), 0.05f, 1.0f));

				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(
					(col - (nrColumns / 2)) * spacing,
					(row - (nrRows / 2)) * spacing,
					0.0f
				));
				sphere_ptr->set_model(model);
				sphere_ptr->draw(pbr_shader);
			}
		}
#endif
		skybox_shader.use();
		skybox_shader.set_uniform("view", camera_ptr->get_view());
		skybox_shader.set_uniform("projection", camera_ptr->get_projection());
		skybox_shader.set_uniform("roughness", roughness);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, hdr_cube_map);
		skybox_shader.set_uniform("env_cube_map", 0);
		skybox_cube_ptr->draw(skybox_shader);

		ImGui::Begin("PBR");
		{
			ImGui::ColorEdit3("albedo", glm::value_ptr(albedo));
			ImGui::SliderFloat("metallic", &metallic, 0.f, 1.0f);
			ImGui::SliderFloat("roughness", &roughness, 0.f, 1.f);
			ImGui::NewLine();
			for (int i = 0; i < light_size; ++i) {
				std::string var = std::format("light_position{}", i + 1);
				ImGui::InputFloat3(var.c_str(), glm::value_ptr(light_position[i]));
			}
			ImGui::NewLine();
			for (int i = 0; i < light_size; ++i) {
				std::string var = std::format("light_color{}", i + 1);
				ImGui::InputFloat3(var.c_str(), glm::value_ptr(light_colors[i]));
			}
		}
		ImGui::End();
		swap_buffer();
	}
}

void Scene::to_cube_map_test() {
	auto cube_ptr = Loader::create_skybox();
	Shader skybox_shader("shader/skybox/skybox.vert", "shader/skybox/skybox.frag");
	if (!skybox_shader) {
		std::cerr << "Failed initialize skybox_shader" << std::endl;
		return;
	}
	
	GLuint env_map = Loader::equirectangular_to_cube_map("resources/skybox/Barce_Rooftop_C_3k.hdr");
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	while (!glfwWindowShouldClose(window)) {
		poll_event();
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		skybox_shader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, env_map);
		skybox_shader.set_uniform("env_cube_map", 0);
		skybox_shader.set_uniform("view", camera_ptr->get_view());
		skybox_shader.set_uniform("projection", camera_ptr->get_projection());
		cube_ptr->draw(skybox_shader);

		swap_buffer();
	}
	return;
}

void Scene::AK47() {
	GLuint hdr_cube_map = Loader::equirectangular_to_cube_map("resources/skybox/Factory_Catwalk_2k.hdr", 1024, 1024);
	GLuint irradiance_map = Loader::irradiance_convolution(hdr_cube_map);
	GLuint prefilter_map = Loader::prefilter(hdr_cube_map);
	GLuint brdf_lut_map = Loader::brdf_lut(hdr_cube_map);

	GLuint albedo_map = Loader::load_texture2ds("resources/AK47/textures/AK47_albedo.tga");
	GLuint ambient_occlusion_map = Loader::load_texture2ds("resources/AK47/textures/AK47_ao.tga");
	GLuint normal_map = Loader::load_texture2d("resources/AK47/textures/AK47_normal.tga");
	GLuint metallic_map = Loader::load_texture2d("resources/AK47/textures/AK47_metalness.tga");
	GLuint roughness_map = Loader::load_texture2d("resources/AK47/textures/AK47_roughness.tga");

	auto ak47_model = Loader::load_model("resources/AK47/AK47.obj");
	auto skybox_ptr = Loader::create_skybox();

	Shader skybox_shader("shader/skybox/skybox.vert", "shader/skybox/skybox.frag");
	Shader ibl_shader("shader/IBL/ibl.vert", "shader/IBL/ibl.frag");
	if (!ibl_shader) {
		std::cerr << "Failed initialize ibl_shader" << std::endl;
		return;
	}

	constexpr int light_size = 4;
	glm::vec3 light_position[light_size] = {
		glm::vec3(-10.0f,  10.0f, 10.0f),
		glm::vec3(10.0f,  10.0f, 10.0f),
		glm::vec3(-10.0f, -10.0f, 10.0f),
		glm::vec3(10.0f, -10.0f, 10.0f),
	};
	glm::vec3 light_colors[light_size] = {
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
	};

	// bind texture
	{
		ibl_shader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_map);
		ibl_shader.set_uniform("irradiance_map", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_map);
		ibl_shader.set_uniform("prefilter_map", 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, brdf_lut_map);
		ibl_shader.set_uniform("brdf_lut_map", 2);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, albedo_map);
		ibl_shader.set_uniform("albedo_map", 3);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, normal_map);
		ibl_shader.set_uniform("normal_map", 4);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, metallic_map);
		ibl_shader.set_uniform("metallic_map", 5);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, roughness_map);
		ibl_shader.set_uniform("roughness_map", 6);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, ambient_occlusion_map);
		ibl_shader.set_uniform("ambient_occlusion_map", 7);

		skybox_shader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, hdr_cube_map);
	}

	glViewport(0, 0, width, height);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);		
	while (!glfwWindowShouldClose(window)) {
		poll_event();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		ibl_shader.use();
		ibl_shader.set_uniform("model", glm::mat4(1.0));
		ibl_shader.set_uniform("view", camera_ptr->get_view());
		ibl_shader.set_uniform("projection", camera_ptr->get_projection());
		ibl_shader.set_uniform("view_pos", camera_ptr->get_look_from());
		CheckError();
		for (int i = 0; i < light_size; ++i) {
			std::string var_pos = std::format("lights[{}].position", i);
			std::string var_col = std::format("lights[{}].color", i);
			ibl_shader.set_uniform(var_pos, light_position[i]);
			ibl_shader.set_uniform(var_col, light_colors[i]);
			CheckError();
		}
		ak47_model->draw(ibl_shader);

		skybox_shader.use();
		skybox_shader.set_uniform("view", camera_ptr->get_view());
		skybox_shader.set_uniform("projection", camera_ptr->get_projection());
		skybox_ptr->draw(skybox_shader);
		swap_buffer();
	}
}

void Scene::house() {
	Shader house_shader("shader/house/house.vert", "shader/house/house.frag", "shader/house/house.geom");
	if (!house_shader) {
		std::cerr << "Failed initialize house shader" << std::endl;
		return;
	}

	float points[] = {
		-0.5f, +0.5f, 1.0f, 0.0f, 0.0f,	// 左上
		+0.5f, +0.5f, 0.0f, 1.0f, 0.0f,	// 右上
		+0.5f, -0.5f, 0.0f, 0.0f, 1.0f,	// 右下
		-0.5f, -0.5f, 1.0f, 1.0f, 0.0f	// 左下
	};

	GLuint VAO;
	GLuint VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

		// 顶点
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void *)0);
		// 颜色
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void *)(sizeof(float) * 2));
	}
	glBindVertexArray(0);
	
	while (!glfwWindowShouldClose(window)) {
		poll_event();
		glClear(GL_COLOR_BUFFER_BIT);
		house_shader.use();
		glBindVertexArray(VAO);
		{
			glDrawArrays(GL_POINTS, 0, 4);
		}
		glBindVertexArray(0);
		swap_buffer();
	}
	
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void Scene::explode() {
	auto alod_ptr = Loader::load_model("resources/alod/dino obj.obj");
	auto diffuse_map = Loader::load_texture2ds("resources/alod/dino.jpg");
	
	Shader explode_shader("shader/explode/explode.vert", "shader/explode/explode.frag", "shader/explode/explode.geom");
	if (!explode_shader) {
		std::cerr << "Failed initialize explode shader" << std::endl;
		return;
	}

	Shader normal_display("shader/normal_display/normal_display.vert", "shader/normal_display/normal_display.frag",
		"shader/normal_display/normal_display.geom");
	if (!normal_display) {
		std::cerr << "Failed normal_display explode shader" << std::endl;
		return;
	}

	glm::vec3 light_pos = glm::vec3(30, 30, 30);
	glm::vec3 light_color = glm::vec3(1.f, 1.f, 1.f);
	explode_shader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuse_map);
	explode_shader.set_uniform("diffuse_map", 0);
	explode_shader.set_uniform("light_pos", light_pos);
	explode_shader.set_uniform("light_color", light_color);

	float offset = 0.0f;
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	while (!glfwWindowShouldClose(window)) {
		poll_event();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		explode_shader.use();
		explode_shader.set_uniform("model", glm::mat4(1));
		explode_shader.set_uniform("view", camera_ptr->get_view());
		explode_shader.set_uniform("projection", camera_ptr->get_projection());
		explode_shader.set_uniform("view_pos", camera_ptr->get_look_from());
		explode_shader.set_uniform("offset", offset);
		alod_ptr->draw(explode_shader);

		normal_display.use();
		normal_display.set_uniform("model", glm::mat4(1));
		normal_display.set_uniform("view", camera_ptr->get_view());
		normal_display.set_uniform("projection", camera_ptr->get_projection());
		alod_ptr->draw(normal_display);

		ImGui::Begin("explode param");
		{
			ImGui::DragFloat("offset", &offset, 0.1f, 0.f, 30.f);
		}
		ImGui::End();
		swap_buffer();
	}
}

void Scene::instantiation() {
	Shader shader("shader/instantiation/instantiation.vert", "shader/instantiation/instantiation.frag");
	if (!shader) {
		std::cerr << "Failed intialize shader" << std::endl;
		return;
	}

	float quad_vertices[] = {
		// 位置          // 颜色
		-0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
		 0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
		-0.05f, -0.05f,  0.0f, 0.0f, 1.0f,

		-0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
		 0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
		 0.05f,  0.05f,  0.0f, 1.0f, 1.0f
	};

	glm::vec2 offsets[100];
	int index = 0;
	for (int x = -10; x < 10; x += 2) {
		for (int y = -10; y < 10; y += 2) {
			glm::vec2 translates;
			translates.x = x / 10.f + 0.1f;
			translates.y = y / 10.f + 0.1f;
			offsets[index++] = translates;
		}
	}

	GLuint VAO;
	GLuint VBO;
	GLuint instance_VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &instance_VBO);
	glBindVertexArray(VAO);
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, (void *)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, (void *)(sizeof(float)*2));

		glBindBuffer(GL_ARRAY_BUFFER, instance_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(offsets), offsets, GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, (void *)0);
		glVertexAttribDivisor(2, 1);
	}
	glBindVertexArray(0);

	//shader.use();
	//for (int i = 0; i < std::size(offsets); ++i) {
	//	std::string var = std::format("offsets[{}]", i);
	//	shader.set_uniform(var, offsets[i]);
	//}

	while (!glfwWindowShouldClose(window)) {
		poll_event();
		glClear(GL_COLOR_BUFFER_BIT);
		shader.use();
		glBindVertexArray(VAO);
		{
			glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);
		}
		glBindVertexArray(0);
		swap_buffer();
	}

	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

void Scene::planet() {
	auto planet_ptr = Loader::load_model("resources/planet/planet.obj");
	auto rock_ptr = Loader::load_model("resources/planet/rock.obj");
	GLuint planet_diffuse_map = Loader::load_texture2ds("resources/planet/planet_Quom1200.png");
	GLuint rock_diffuse_map = Loader::load_texture2ds("resources/planet/Rock-Texture-Surface.jpg");

	Shader shader("shader/planet/planet.vert", "shader/planet/palent.frag");
	if (!shader) {
		std::cerr << "failed initialize shader" << std::endl;
		return;
	}

	unsigned int amout = 5000;
	std::vector<glm::mat4> model_matrices;
	model_matrices.reserve(amout);
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0.f, 1.0);
	float radius = 50.f;
	float offset = 2.5f;
	for (unsigned int i = 0; i < amout; ++i) {
		// 位移
		float angle = float(i) / float(amout) * 360.f;
		float displacement = mix(-offset, offset, dis(gen));
		float x = std::sin(angle) * radius + displacement;
		float y = displacement * mix(-2.f, 2.f, dis(gen));
		displacement = mix(-offset, offset, dis(gen));
		float z = std::cos(angle) * radius + displacement;
		glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(x, y, z));

		// 缩放在 0.05 和 0.25 之间
		float scale = mix(0.05f, 0.25f, dis(gen));
		model = glm::scale(model, glm::vec3(scale));

		// 旋转
		float rot_angle = dis(gen) * 360.f;
		model = glm::rotate(model, rot_angle, glm::vec3(0.4f, 0.6f, 0.8f));

		model_matrices.emplace_back(model);
	}

	GLuint VAO = rock_ptr->get_mesh_VAO(0);
	GLuint instance_VBO;
	glGenBuffers(1, &instance_VBO);
	glBindVertexArray(VAO);
	{
		glBindBuffer(GL_ARRAY_BUFFER, instance_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * model_matrices.size(), model_matrices.data(), GL_STATIC_DRAW);
		constexpr size_t vec4_size = sizeof(glm::vec4);
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)0);
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(vec4_size));
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(vec4_size * 2));
		glEnableVertexAttribArray(8);
		glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(vec4_size * 3));
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glVertexAttribDivisor(7, 1);
		glVertexAttribDivisor(8, 1);
	}
	glBindVertexArray(0);

	glm::mat4 model(1);
	model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
	model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	while (!glfwWindowShouldClose(window)) {
		poll_event();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.use();
		glBindTexture(GL_TEXTURE_2D, planet_diffuse_map);
		shader.set_uniform("diffuse_map", 0);
		shader.set_uniform("model", model);
		shader.set_uniform("view", camera_ptr->get_view());
		shader.set_uniform("projection", camera_ptr->get_projection());
		shader.set_uniform("is_planet", true);
		planet_ptr->draw(shader);

		glBindTexture(GL_TEXTURE_2D, rock_diffuse_map);
		shader.set_uniform("diffuse_map", 0);
		shader.set_uniform("is_planet", false);
		auto movement = glm::rotate(glm::mat4(1), float(glfwGetTime() / 30.f), glm::vec3(0, 1, 0));
		shader.set_uniform("movement", movement);
		rock_ptr->draw_instance(shader, amout);

		swap_buffer();
	}
}

void Scene::point_shadow() {
	auto plane_ptr = Loader::create_test_plane();
	auto cube_ptr = Loader::create_trest_cube();
	GLuint plane_diffuse_map = Loader::load_texture2ds("resources/test_plane/wood.png");
	GLuint cube_diffuse_map = Loader::load_texture2ds("resources/test_cube/container2.png");
	Shader shader("shader/point_shadow/point_shadow.vert", "shader/point_shadow/point_shadow.frag");
	Shader single_color_shader("shader/bloom/single_color.vert", "shader/bloom/single_color.frag");
	Shader sample_depth_shader("shader/point_shadow/sample_depth.vert", "shader/point_shadow/sample_depth.frag", 
		"shader/point_shadow/sample_depth.geom");

	int SHADOW_WIDTH = 1024;
	int SHADOW_HEIGHT = 1024;

	GLuint depth_cube_map;
	glGenTextures(1, &depth_cube_map);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cube_map);
	{
		for (int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 
				0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	GLuint depth_fbo;
	GLuint depth_rbo;
	glGenFramebuffers(1, &depth_fbo);
	glGenRenderbuffers(1, &depth_rbo);
	glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
	{
		glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, SHADOW_WIDTH, SHADOW_HEIGHT);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rbo);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_cube_map, 0);
		glDrawBuffer(GL_NONE);		
		glReadBuffer(GL_NONE);		
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glm::mat4 plane_scale = glm::scale(glm::mat4(1.f), glm::vec3(10.f, 1, 10.f));
	glm::mat4 plane_trans_matrix = glm::rotate(plane_scale, glm::radians(90.f), glm::vec3(1, 0, 0));
	glm::mat4 cube_trans = glm::translate(glm::mat4(1.f), glm::vec3(0, 1, 0));

	glm::vec3 light_pos(10, 10, 10);
	glm::vec3 light_color(300);
	glm::mat4 light_trans = glm::translate(glm::mat4(1), light_pos);

	float aspect = float(SHADOW_WIDTH) / float(SHADOW_HEIGHT);
	float near = 1.f;
	float far = 50.f;
	glm::mat4 projection = glm::perspective(glm::radians(90.0f), aspect, near, far);
	std::vector<glm::mat4> shadow_transforms = {
		projection * glm::lookAt(light_pos, light_pos + glm::vec3(+1, 0, 0), glm::vec3(0.0, -1.0,  0.0)),
		projection * glm::lookAt(light_pos, light_pos + glm::vec3(-1, 0, 0), glm::vec3(0.0, -1.0,  0.0)),
		projection * glm::lookAt(light_pos, light_pos + glm::vec3(0, +1, 0), glm::vec3(0.0,  0.0,  1.0)),
		projection * glm::lookAt(light_pos, light_pos + glm::vec3(0, -1, 0), glm::vec3(0.0,  0.0, -1.0)),
		projection * glm::lookAt(light_pos, light_pos + glm::vec3(0, 0, +1), glm::vec3(0.0, -1.0,  0.0)),
		projection * glm::lookAt(light_pos, light_pos + glm::vec3(0, 0, -1), glm::vec3(0.0, -1.0,  0.0)),
	};

	sample_depth_shader.use();
	sample_depth_shader.set_uniform("far_plane", far);
	sample_depth_shader.set_uniform("light_pos", light_pos);
	for (int i = 0; i < 6; ++i) {
		std::string var = std::format("light_space_matrix[{}]", i);
		sample_depth_shader.set_uniform(var, shadow_transforms[i]);
	}

	shader.use();
	glActiveTexture(GL_TEXTURE0);
	shader.set_uniform("light_pos", light_pos);

	bool is_shadow = true;
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
	while (!glfwWindowShouldClose(window)) {
		poll_event();

		glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
		{
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glClear(GL_DEPTH_BUFFER_BIT);
			sample_depth_shader.use();
			sample_depth_shader.set_uniform("model", plane_trans_matrix);
			plane_ptr->draw(sample_depth_shader);

			sample_depth_shader.set_uniform("model", cube_trans);
			cube_ptr->draw(sample_depth_shader);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(0, 0, width, height);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		shader.use();
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cube_map);
		shader.set_uniform("depth_cube_map", 1);
		shader.set_uniform("far_plane", far);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, plane_diffuse_map);
		shader.set_uniform("light_color", light_color);
		shader.set_uniform("is_shadow", is_shadow);
		shader.set_uniform("diffuse_map", 0);
		shader.set_uniform("model", plane_trans_matrix);
		shader.set_uniform("view", camera_ptr->get_view());
		shader.set_uniform("projection", camera_ptr->get_projection());
		shader.set_uniform("view_pos", camera_ptr->get_look_from());
		plane_ptr->draw(shader);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cube_diffuse_map);
		shader.set_uniform("diffuse_map", 0);
		shader.set_uniform("model", cube_trans);
		cube_ptr->draw(shader);

		single_color_shader.use();
		single_color_shader.set_uniform("light_color", light_color);
		single_color_shader.set_uniform("model", light_trans);
		single_color_shader.set_uniform("view", camera_ptr->get_view());
		single_color_shader.set_uniform("projection", camera_ptr->get_projection());
		cube_ptr->draw(single_color_shader);

		ImGui::Begin("point shadow");
		{
			ImGui::Checkbox("is_shadow", &is_shadow);
			ImGui::InputFloat3("light_color", glm::value_ptr(light_color));
		}
		ImGui::End();
		swap_buffer();
	}

	glDeleteTextures(1, &depth_cube_map);
	glDeleteRenderbuffers(1, &depth_rbo);
	glDeleteFramebuffers(1, &depth_fbo);
}

void Scene::deferred_shading() {
	auto alod_ptr = Loader::load_model("resources/alod/dino obj.obj");
	auto cube_ptr = Loader::create_trest_cube();
	auto quad_ptr = Loader::create_quad();
	GLuint alod_diffuse_map = Loader::load_texture2ds("resources/alod/dino.jpg");
	Shader gbuffer_shader("shader/gbuffer/gbuffer.vert", "shader/gbuffer/gbuffer.frag");
	Shader deferred_shader("shader/gbuffer/deferred_shading.vert", "shader/gbuffer/deferred_shading.frag");
	Shader single_color_shader("shader/bloom/single_color.vert", "shader/bloom/single_color.frag");

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0, 1);

	constexpr int alod_row = 5;
	constexpr int alod_col = 5;
	constexpr int alod_interval = 100;
	std::vector<glm::vec3> alod_transforms;
	alod_transforms.reserve(alod_row * alod_col);
	for (int i = 0; i < alod_row; ++i) {
		for (int j = 0; j < alod_col; ++j) {
			int x = i * alod_interval;
			int y = 0;
			int z = j * alod_interval;
			alod_transforms.emplace_back(x, y, z);
		}
	}

	glm::mat4 light_scale = glm::scale(glm::mat4(1), glm::vec3(5));
	constexpr int light_size = 32;
	std::vector<glm::vec3> light_transforms;
	std::vector<glm::vec3> light_colors;
	light_transforms.reserve(light_size);
	light_colors.reserve(light_size);
	for (int i = 0; i < light_size; ++i) {
		float xpos = mix(0.f, 500.f, dis(gen));
		float ypos = mix(90.f, 150.f, dis(gen));
		float zpos = mix(0.f, 500.f, dis(gen));
		light_transforms.emplace_back(xpos, ypos, zpos);
		float r = mix(0.5f, 1.0f, dis(gen));
		float g = mix(0.5f, 1.0f, dis(gen));
		float b = mix(0.5f, 1.0f, dis(gen));
		light_colors.emplace_back(r, g, b);
	}

	// 设置光源 uniform
	deferred_shader.use();
	for (int i = 0; i < light_size; ++i) {
		std::string pos_var = std::format("lights[{}].position", i);
		std::string col_var = std::format("lights[{}].color", i);
		deferred_shader.set_uniform(pos_var, light_transforms[i]);
		deferred_shader.set_uniform(col_var, light_colors[i]);
	}
	
	GLuint gbuffer;
	GLuint gbuffer_rbo;
	GLuint position_buffer;
	GLuint normal_buffer;
	GLuint color_buffer;
	glGenFramebuffers(1, &gbuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gbuffer);
	{
		// 位置纹理
		glGenTextures(1, &position_buffer);
		glBindTexture(GL_TEXTURE_2D, position_buffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, position_buffer, 0);

		// 法线纹理
		glGenTextures(1, &normal_buffer);
		glBindTexture(GL_TEXTURE_2D, normal_buffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal_buffer, 0);

		// 颜色纹理
		glGenTextures(1, &color_buffer);
		glBindTexture(GL_TEXTURE_2D, color_buffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, color_buffer, 0);

		glGenRenderbuffers(1, &gbuffer_rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, gbuffer_rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gbuffer_rbo);

		GLuint attachment[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, attachment);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
	while (!glfwWindowShouldClose(window)) {
		poll_event();
		glBindFramebuffer(GL_FRAMEBUFFER, gbuffer);
		{
			glViewport(0, 0, width, height);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			gbuffer_shader.use();
			gbuffer_shader.set_uniform("view", camera_ptr->get_view());
			gbuffer_shader.set_uniform("projection", camera_ptr->get_projection());
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, alod_diffuse_map);
			gbuffer_shader.set_uniform("diffuse_map", 0);
			for (int i = 0; i < alod_transforms.size(); ++i) {
				glm::mat4 model = glm::translate(glm::mat4(1.0f), alod_transforms[i]);
				gbuffer_shader.set_uniform("model", model);
				alod_ptr->draw(gbuffer_shader);
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 绑定 G-Buffer 纹理
		{
			deferred_shader.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, position_buffer);
			deferred_shader.set_uniform("position_buffer", 0);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, normal_buffer);
			deferred_shader.set_uniform("normal_buffer", 1);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, color_buffer);
			deferred_shader.set_uniform("albedo_buffer", 2);
		}
		quad_ptr->draw(deferred_shader);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, gbuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		// 绘制光源
		single_color_shader.use();
		single_color_shader.set_uniform("view", camera_ptr->get_view());
		single_color_shader.set_uniform("projection", camera_ptr->get_projection());
		for (int i = 0; i < light_size; ++i) {
			glm::mat4 model = glm::translate(glm::mat4(1), light_transforms[i]);
			model = glm::scale(model, glm::vec3(10));
			single_color_shader.set_uniform("model", model);
			single_color_shader.set_uniform("light_color", light_colors[i]);
			cube_ptr->draw(single_color_shader);
		}

		swap_buffer();
	}

	glDeleteTextures(1, &color_buffer);
	glDeleteTextures(1, &normal_buffer);
	glDeleteTextures(1, &position_buffer);
	glDeleteRenderbuffers(1, &gbuffer_rbo);
	glDeleteFramebuffers(1, &gbuffer);
}


