#include "common.h"


Camera::Camera(const glm::vec3 &lf, const glm::vec3 &lu, float _fov, 
			   float _near, float _far, float _sensitivity, float _speed)
: look_from(lf), world_y(glm::normalize(lu)), pitch(0)
, yaw(0), fov(_fov), sensitivity(_sensitivity), near(_near)
, far(_far), speed(_speed)
{
	update_wrold_axis();
}

glm::mat4 Camera::get_view() const {
	return glm::lookAt(look_from, look_from+look_at, look_up);
}

glm::mat4 Camera::get_projection() const {
	return glm::perspective(radian(fov), aspect, near, far);
}

const glm::vec3 &Camera::get_look_from() const {
	return look_from;
}

void Camera::set_aspect(float _aspect) noexcept {
	aspect = _aspect;
}

inline
constexpr float Camera::radian(float angle) {
	return angle / 180.f * 3.1415926f;
}

void Camera::init_euler_angle(float _pitch, float _yaw) {
	pitch = _pitch;
	yaw = _yaw;
	update_wrold_axis();
}

void Camera::mouse_callback(double offset_x, double offset_y) {
	if (prev_x == 0 && prev_y == 0) {
		prev_x = float(offset_x);
		prev_y = float(offset_y);
		return;
	}
	if (!enable_mouse)
		return;

	pitch = std::clamp(pitch + (float(prev_y - offset_y) * sensitivity), -89.5f, 89.5f);
	yaw = yaw + float(offset_x - prev_x) * sensitivity;
	prev_x = float(offset_x);
	prev_y = float(offset_y);
	update_wrold_axis();
}

void Camera::scroll_callback(double offset) {
	if (!enable_mouse)
		return;

	fov = std::clamp(fov + float(-offset * sensitivity), 0.1f, 89.f);
}

void Camera::framebuff_callback(int width, int height) {
	aspect = float(width) / float(height);
}

void Camera::key_callback(CameraKey key, float delat_time) {
	if (!enable_keyboard)
		return;

	switch (key) {
	case CameraKey::Forward:
		look_from += look_at * (delat_time * speed);
		break;
	case CameraKey::BackWard:
		look_from -= look_at * (delat_time * speed);
		break;
	case CameraKey::Right:
		look_from += world_x * (delat_time * speed);
		break;
	case CameraKey::Left:
		look_from -= world_x * (delat_time * speed);
		break;
	}
	//std::cout << "form\t" << glm::to_string(look_from) << std::endl;
	//std::cout << "look_at\t" << glm::to_string(look_at) << std::endl;
}


void Camera::camera_widgets() {
	ImGui::Begin("camera");
	{
		ImGui::InputFloat3("look_from", glm::value_ptr(look_from));
		ImGui::InputFloat3("look_up", glm::value_ptr(look_up));
		ImGui::DragFloat("fov", &fov, 0.5f, 1.f, 89.f);
		ImGui::DragFloat("pitch", &pitch, 1.f, -89.5, 89.5f);
		ImGui::DragFloat("yaw", &yaw);
		ImGui::InputFloat("sensitivity", &sensitivity);
		ImGui::InputFloat("speed", &speed);
		ImGui::Checkbox("mouse", &enable_mouse);
		ImGui::Checkbox("keyboard", &enable_keyboard);
	}
	ImGui::End();
	update_wrold_axis();
}

void Camera::update_wrold_axis() {
	glm::vec3 offset(0);
	offset.y = std::sin(glm::radians(pitch));
	offset.x = std::cos(glm::radians(pitch)) * std::cos(glm::radians(yaw));
	offset.z = std::cos(glm::radians(pitch)) * std::sin(glm::radians(yaw));
	look_at = glm::normalize(offset);
	world_x = glm::normalize(glm::cross(look_at, world_y));
	look_up = glm::normalize(glm::cross(world_x, look_at));
}


