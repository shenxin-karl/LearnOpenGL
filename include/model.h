#pragma once

class Model {
	friend class Loader;
	std::vector<Mesh>	meshs;
	std::string			directory;
	std::string			model_name_;
	glm::mat4			model_;
public:
	Model();
	void draw(Shader &shader) const;
	const std::string &get_name() const;
	void set_model(const glm::mat4 &model);
	void show_widgets();
	const std::vector<Mesh> &get_meshs();
};