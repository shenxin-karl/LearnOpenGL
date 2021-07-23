#pragma once

constexpr std::string_view project_name = "LearnOpenGL";

GLenum _check_error(const char *file, int line) noexcept;
#define CheckError() _check_error(__FILE__, __LINE__)

GLFWwindow *create_window(int width, int height, const std::string &title);

template<typename T> requires requires{ std::declval<T>() + (std::declval<T>() - std::declval<T>()) * 1.f; }
T mix(const T &lhs, const T &rhs, float t) {
	return lhs + static_cast<T>(t * (rhs - lhs));
}

void blinn_phong(GLFWwindow *window);
void normal_mapping(GLFWwindow *window);
void parallax_mapping(GLFWwindow *window);
void shadow_mapping(GLFWwindow *window);
void bloom(GLFWwindow *window);
void pbr(GLFWwindow *window);
void to_cube_map_test(GLFWwindow *window);
void AK47(GLFWwindow *window);
void house(GLFWwindow *window);
void explode(GLFWwindow *window);
void instantiation(GLFWwindow *window);
void planet(GLFWwindow *window);
void point_shadow(GLFWwindow *window);
void deferred_shading(GLFWwindow *window);