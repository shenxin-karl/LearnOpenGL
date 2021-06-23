#include "common.h"

Mesh::Mesh(std::vector<Vertex> &&_vertices, std::vector<uint> &&_indices, std::vector<Texture> &&_textures) 
: vertices(std::move(_vertices)), indices(std::move(_indices)), textures(std::move(_textures)), VAO(0), VBO(0), EBO(0)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), indices.data(), GL_STATIC_DRAW);

		// 顶点
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));

		// 纹理坐标
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texcoord));

		// 法线
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));

		// 切线
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, tangent));

		// 副切线
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, bitangent));

	}
	glBindVertexArray(0);
}

Mesh::Mesh(Mesh &&other) noexcept : VAO(0), VBO(0), EBO(0) {
	*this = std::move(other);
}

Mesh &Mesh::operator=(Mesh &&other) noexcept {
	if (this == &other)
		return *this;
	vertices = std::move(other.vertices);
	indices = std::move(other.indices);
	textures = std::move(other.textures);
	VAO = std::exchange(other.VAO, 0);
	VBO = std::exchange(other.VBO, 0);
	EBO = std::exchange(other.EBO, 0);
	return *this;
}

void Mesh::draw(Shader &shader) const {
	shader.use();
	std::unordered_map<aiTextureType, int> type_counter;
	int i = 0;
	for (auto &texture : textures) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texture.obj);
		int count = type_counter[texture.type] + 1;
		type_counter[texture.type]++;
		std::string uniform_name = texture.var_name + std::to_string(count);
		shader.set_uniform(uniform_name, i);
		++i;
	}

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, GLsizei(vertices.size()), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

Mesh::~Mesh() {
	if (VAO != 0) glDeleteVertexArrays(1, &VAO);
	if (VBO != 0) glDeleteBuffers(1, &VBO);
	if (EBO != 0) glDeleteBuffers(1, &EBO);
}

