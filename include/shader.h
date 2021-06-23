#pragma once

class Shader {
	GLuint	id;
	std::unordered_map<std::string, GLint> uniform_location_cache;
public:
	Shader(const std::string &vertex_path, const std::string &fragment_path);
	Shader(Shader &&other) noexcept;
	Shader(const Shader &) = delete;
	explicit operator bool() const noexcept;
	GLuint get_id() const;
	void use() const;
	~Shader();
public:
	void set_uniform(const std::string &var, bool b);
	void set_uniform(const std::string &var, int v);
	void set_uniform(const std::string &var, unsigned int v);
	void set_uniform(const std::string &var, float f);
	void set_uniform(const std::string &var, const glm::vec2 &vec);
	void set_uniform(const std::string &var, const glm::vec3 &vec);
	void set_uniform(const std::string &var, const glm::vec4 &vec);
	void set_uniform(const std::string &var, const glm::mat2 &mat);
	void set_uniform(const std::string &var, const glm::mat3 &mat);
	void set_uniform(const std::string &var, const glm::mat4 &mat);
public:
	GLint get_uniform_location(const std::string &var);
};