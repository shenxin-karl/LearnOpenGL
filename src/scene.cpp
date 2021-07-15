#include "common.h"

Scene::Scene(GLFWwindow *_window, std::shared_ptr<Camera> _camera_ptr, int _widht, int _hegith, float _near, float _far)
: window(_window), camera_ptr(_camera_ptr), width(_widht), height(_hegith), near(_near), far(_far)
{
	scene_mapping.insert(std::make_pair(window, this));
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
}

Scene::~Scene() {
	scene_mapping.erase(window);

	// imgui Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
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


void Scene::add_model(std::shared_ptr<Model> model_ptr) {
	models_.push_back(model_ptr);
	model_flags_.push_back(0);
}

void Scene::scene_widgets() {
	ImGui::Begin("scene");
	{
		ImGui::Checkbox("camera", &show_camera_widgets_);
		for (int i = 0; i < models_.size(); ++i)
			ImGui::Checkbox(models_[i]->get_name().c_str(), reinterpret_cast<bool *>(&model_flags_[i]));
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

	glm::vec3 light_dir = glm::normalize(glm::vec3(0.5f, 0.5f, 0.f));
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
		normal_mapping_shader.set_uniform("light_dir", new_light_dir);
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
	parallax_mapping_shader.set_uniform("displacement_scale", 0.1f);

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

		glm::mat4 light_rotate = glm::rotate(glm::mat4(1.f), float(glm::radians(glfwGetTime() * 5.f)), glm::vec3(0, 1, 0));
		glm::vec3 new_light_dir = light_rotate * glm::vec4(light_dir, 1.f);
		parallax_mapping_shader.set_uniform("light_dir", new_light_dir);
		plane_ptr->draw(parallax_mapping_shader);
		CheckError();
		//glfwSwapBuffers(window);
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

	// ���� fbo
	constexpr int SHADOW_WIDTH = 1024;
	constexpr int SHADOW_HEIGHT = 1024;
	GLuint depth_fbo;
	GLuint depth_map;
	glGenFramebuffers(1, &depth_fbo);
	glGenTextures(1, &depth_map);
	glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
	{
		// Ϊ�����������ڴ�
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

		// ������ռ����
		constexpr float near = 10.f;
		constexpr float far = 100.f;
		constexpr float left = -20.f;
		constexpr float right = 20.f;
		constexpr float top = 20.f;
		constexpr float bottom = -20.f;
		glm::mat4 light_view = glm::lookAt(light_dir * 30.f, glm::vec3(0), glm::vec3(0, 1, 0));
		glm::mat4 light_projection = glm::ortho(left, right, bottom, top, near, far);
		glm::mat4 light_space_matrix = light_projection * light_view;
		// ��Ⱦ�����ͼ
		glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
		{
			glCullFace(GL_FRONT);		// ��Ⱦ���������ͼʹ�������޳�, ���������������
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glClear(GL_DEPTH_BUFFER_BIT);

			sample_depth_shader.use();
			sample_depth_shader.set_uniform("light_space_matrix", light_space_matrix);
			sample_depth_shader.set_uniform("model", plane_trans_matrix);
			plane_ptr->draw(sample_depth_shader);

			sample_depth_shader.set_uniform("model", cube_trans);
			cube_ptr->draw(sample_depth_shader);
			glCullFace(GL_BACK);		// �ָ������޳�
		}

	// ������Ӱ����
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
		// ��˹ģ��
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

		// ��Ĭ�ϵ�֡����
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
	add_model(sphere_ptr);

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
	glDepthFunc(GL_LEQUAL);
	while (!glfwWindowShouldClose(window)) {
		poll_event();
		glViewport(0, 0, width, height);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		pbr_shader.use();
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
		-0.5f,  0.5f, 1.0f, 0.0f, 0.0f,	// ����
		 0.5f,  0.5f, 0.0f, 1.0f, 0.0f,	// ����
		 0.5f, -0.5f, 0.0f, 0.0f, 1.0f,	// ����
		-0.5f, -0.5f, 1.0f, 1.0f, 0.0f	// ����
	};

	GLuint VAO;
	GLuint VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

		// ����
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void *)0);
		// ��ɫ
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
			glDrawArrays(GL_POINTS, 0, std::size(points) / 2);
		}
		glBindVertexArray(0);
		swap_buffer();
	}
	
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}
