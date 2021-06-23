#include "common.h"

Shader::Shader(const std::string &vertex_path, const std::string &fragment_path) : id(-1) {
	std::ifstream vertex_file(vertex_path);
	std::ifstream fragment_file(fragment_path);
	if (!vertex_file.is_open() || !fragment_file.is_open()) {
		std::cerr << "Can't open shader file" << std::endl;
		return;
	}

	const char *code_ptr = nullptr;
	std::stringstream sbuf;
	sbuf << vertex_file.rdbuf();
	std::string vertex_content = sbuf.str();
	code_ptr = vertex_content.c_str();

	int success = 1;
	char errmsg[512] = { '\0' };

	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &code_ptr, nullptr);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex_shader, sizeof(errmsg), nullptr, errmsg);
		std::cerr << "Compile vertex shader error:" << errmsg << std::endl;
		return;
	}

	sbuf.str({});
	sbuf << fragment_file.rdbuf();
	std::string fragment_content = sbuf.str();
	code_ptr = fragment_content.c_str();

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &code_ptr, nullptr);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment_shader, sizeof(errmsg), nullptr, errmsg);
		std::cerr << "Compile fragment shader error:" << errmsg << std::endl;
		return;
	}

	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader_program, sizeof(errmsg), nullptr, errmsg);
		std::cerr << "Link program error:" << errmsg << std::endl;
		return;
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	id = shader_program;
}

Shader::Shader(Shader &&other) noexcept  
: id(std::exchange(other.id, -1))
, uniform_location_cache(std::move(other.uniform_location_cache)) 
{
}

Shader::operator bool() const noexcept {
	return id != -1;
}

GLuint Shader::get_id() const {
	return id;
}

void Shader::use() const {
	assert(id != -1);
	glUseProgram(id);
}

Shader::~Shader() {
	if (id != -1) glDeleteProgram(id);
}

void Shader::set_uniform(const std::string &var, bool b) {
	glUniform1i(get_uniform_location(var), b);
}

void Shader::set_uniform(const std::string &var, int v) {
	glUniform1i(get_uniform_location(var), v);
}

void Shader::set_uniform(const std::string &var, unsigned int v) {
	glUniform1ui(get_uniform_location(var), v);
}

void Shader::set_uniform(const std::string &var, float f) {
	glUniform1f(get_uniform_location(var), f);
}

void Shader::set_uniform(const std::string &var, const glm::vec2 &vec) {
	glUniform2fv(get_uniform_location(var), 1, glm::value_ptr(vec));
}

void Shader::set_uniform(const std::string &var, const glm::vec3 &vec) {
	glUniform3fv(get_uniform_location(var), 1, glm::value_ptr(vec));
}

void Shader::set_uniform(const std::string &var, const glm::vec4 &vec) {
	glUniform4fv(get_uniform_location(var), 1, glm::value_ptr(vec));
}

void Shader::set_uniform(const std::string &var, const glm::mat2 &mat) {
	glUniformMatrix2fv(get_uniform_location(var), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::set_uniform(const std::string &var, const glm::mat3 &mat) {
	glUniformMatrix3fv(get_uniform_location(var), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::set_uniform(const std::string &var, const glm::mat4 &mat) {
	glUniformMatrix4fv(get_uniform_location(var), 1, GL_FALSE, glm::value_ptr(mat));
}

GLint Shader::get_uniform_location(const std::string &var) {
	if (auto iter = uniform_location_cache.find(var); iter != uniform_location_cache.end())
		return iter->second;

	GLint location = glGetUniformLocation(id, var.c_str());
	if (location != -1)
		uniform_location_cache.emplace(std::make_pair(var, location));
	return location;
}

