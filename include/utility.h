#pragma once

constexpr std::string_view project_name = "LearnOpenGL";

GLenum _check_error(const char *file, int line) noexcept;
#define CheckError() _check_error(__FILE__, __LINE__)

GLFWwindow *create_window(int width, int height, const std::string &title);
void blinn_phong();
void normal_mapping();
void parallax_mapping();
void shadow_mapping();
void bloom();
void pbr();
void to_cube_map_test();
void AK47();
void house();
void explode();