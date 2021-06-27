#pragma once

class Scene {
	GLFWwindow				*window;
	std::shared_ptr<Camera> camera_ptr;
	int width;
	int height;
	float near;
	float far;
	float last_time = 0;
	int   next_time = 0;
	int   fps = 0;
	bool  keys[512] = { false };
public:
	std::function<void(int key, int scancode, int action, int mode)>	on_key_callback;
	std::function<void(double xpos, double ypos)>						on_mouse_callback;
	std::function<void(double xoffset, double yoffset)>					on_scroll_callback;
	std::function<void(int width, int height)>							on_frambuffer_size_callback;
public:
	Scene(GLFWwindow *_window, std::shared_ptr<Camera> _camera_ptr, int _widht, int _hegith, float _near, float _far);
	Scene(const Scene &) = delete;
	~Scene();
public:
	void process_key_callback(int key, int scancode, int action, int mode);
	void process_mouse_callback(double xpos, double ypos);
	void process_scroll_callback(double xoffset, double yoffset);
	void process_framebuffer_size_callback(int width, int height);
	void poll_event();
public:
	static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
	static void mouse_callback(GLFWwindow *window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
	static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
	inline static std::unordered_map<GLFWwindow *, Scene *> scene_mapping;	
public:
	void blinn_phong();
	void normal_mapping();
	void parallax_mapping();
	void shadow_mapping();
};