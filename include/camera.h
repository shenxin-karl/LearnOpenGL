#pragma once

enum class CameraKey {
	Forward,
	BackWard,
	Left,
	Right,
};

class Camera {
	glm::vec3	look_from;
	glm::vec3	look_up;
	glm::vec3	look_at;
	glm::vec3	world_x;
	glm::vec3	world_y;
	float		yaw;
	float		pitch;
	float		fov;
	float		aspect;
	float		sensitivity;
	float		near;
	float		far;
	float		speed;
	float prev_x = 0.f;
	float prev_y = 0.f;
	bool		enable_mouse = false;
	bool		enable_keyboard = true;
public:
	Camera(const glm::vec3 &lf, const glm::vec3 &lu, float _fov, float _near, float _far, 
		   float _sensitivity, float _speed);
	glm::mat4 get_view() const;
	glm::mat4 get_projection() const;
	const glm::vec3 &get_look_from() const;
	void set_aspect(float _aspect) noexcept;
	constexpr static float radian(float angle);
public:
	void mouse_callback(double offset_x, double offset_y);
	void scroll_callback(double offset);
	void framebuff_callback(int width, int height);
	void key_callback(CameraKey key, float speed);
	void camera_widgets();
private:
	void update_wrold_axis();
};




