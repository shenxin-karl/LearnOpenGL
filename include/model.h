#pragma once

class Model {
	friend class Loader;
	std::vector<Mesh>	meshs;
	std::string			directory;
public:
	Model() = default;
	void draw(Shader &shader) const;
};