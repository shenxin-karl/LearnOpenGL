#pragma once

struct Vertex {
	glm::vec3	position;
	glm::vec2	texcoord;
	glm::vec3	normal;
	glm::vec3	tangent;
	glm::vec3	bitangent;
};

struct Texture {
	GLuint			obj;			// object id
	std::string		var_name;		// uniform variable name
	aiTextureType	type;			// texture type
};

class Mesh {
	std::vector<Vertex>		vertices;	
	std::vector<uint>		indices;
	std::vector<Texture>	textures;
	GLuint					VAO;
	GLuint					VBO;
	GLuint					EBO;
public:
	GLuint					draw_type_ = GL_TRIANGLES;
public:
	Mesh(std::vector<Vertex> &&_vertices, std::vector<uint> &&_indices, std::vector<Texture> &&_textures);
	Mesh(const Mesh &) = delete;
	Mesh(Mesh &&other) noexcept;
	Mesh &operator=(Mesh &&other) noexcept;
	void draw(Shader &shader) const;
	void draw_instance(Shader &shader, int count) const;
	GLuint get_VAO() const;
	~Mesh();
};