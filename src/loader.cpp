#include "common.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//ImageCacheRecycle Loader::image_cache_recycle;
//std::unordered_map<std::string, std::shared_ptr<ImageInfo>> image_cache;
//std::unordered_map<std::string, GLuint> texture_cache;

std::shared_ptr<Model> Loader::load_model(const std::string &path) {
	Assimp::Importer importer;
	auto flag = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace;
	const aiScene *scene = importer.ReadFile(path, flag);
	if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr) {
		std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return nullptr;
	}
	
	std::shared_ptr<Model> model_ptr = std::make_unique<Model>();
	auto pos = path.find_last_of('/');
	model_ptr->directory = path.substr(0, pos);
	model_ptr->model_name_ = path.substr(pos + 1);
	process_node(scene->mRootNode, scene, model_ptr);
	return model_ptr;
}	

std::shared_ptr<Model> Loader::create_test_plane() {
	std::shared_ptr<Model> model_ptr = std::make_unique<Model>();

	std::vector<Vertex> vertices = {
		Vertex{ glm::vec3(-1.0,  1.0, 0.0), glm::vec2(0.0, 1.0) },		// ◊Û…œΩ«
		Vertex{ glm::vec3(-1.0, -1.0, 0.0), glm::vec2(0.0, 0.0) },		// ◊Ûœ¬Ω«
		Vertex{ glm::vec3( 1.0, -1.0, 0.0), glm::vec2(1.0, 0.0) },		// ”“œ¬Ω«
		Vertex{ glm::vec3( 1.0,  1.0, 0.0), glm::vec2(1.0, 1.0) },		// ”“…œΩ«
	};

	std::vector<uint> indices = {
		0, 2, 1,
		0, 3, 2,
	};

	static int counter = 0;
	generate_normal(vertices, indices);
	generate_tangent(vertices, indices);
	Mesh mesh(std::move(vertices), std::move(indices), {});
	model_ptr->meshs.push_back(std::move(mesh));
	model_ptr->directory = "create_test_plane";
	model_ptr->model_name_ = std::format("test_plane{}", ++counter);
	return model_ptr;
}


std::shared_ptr<Model> Loader::create_trest_cube() {
	std::shared_ptr<Model> model_ptr = std::make_unique<Model>();
	std::vector<Vertex> vertices = {
		#include "test_cube/vertex.txt"
	};

	std::vector<uint> indices;
	indices.reserve(vertices.size());
	std::generate_n(std::back_inserter(indices), vertices.size(), [n = 0]() mutable {
		return n++;
	});

	static int counter = 0;
	generate_normal(vertices, indices);
	generate_tangent(vertices, indices);
	Mesh mesh(std::move(vertices), std::move(indices), {});
	model_ptr->meshs.push_back(std::move(mesh));
	model_ptr->directory = "create_trest_cube";
	model_ptr->model_name_ = std::format("cube{}", ++counter);
	return model_ptr;
}

std::shared_ptr<Model> Loader::create_quad() {
	std::vector<Vertex> vertices = {
		Vertex{ glm::vec3(-1.0f,  1.0f, 0.f), glm::vec2(0.0f, 1.0f) },		// ◊Û…œΩ«
		Vertex{ glm::vec3(-1.0f, -1.0f, 0.f), glm::vec2(0.0f, 0.0f) },		// ◊Ûœ¬Ω«
		Vertex{ glm::vec3( 1.0f, -1.0f, 0.f), glm::vec2(1.0f, 0.0f) },		// ”“œ¬Ω«

		Vertex{ glm::vec3(-1.0f,  1.0f, 0.f), glm::vec2(0.0f, 1.0f) },		// ◊Û…œΩ«
		Vertex{ glm::vec3( 1.0f, -1.0f, 0.f), glm::vec2(1.0f, 0.0f) },		// ”“œ¬Ω«
		Vertex{ glm::vec3( 1.0f,  1.0f, 0.f), glm::vec2(1.0f, 1.0f) },		// ”“…œΩ«
	};	

	std::vector<uint> indices;
	indices.reserve(vertices.size());
	std::generate_n(std::back_inserter(indices), vertices.size(), [n = 0]() mutable {
		return n++;
	});

	static int counter = 0;
	generate_normal(vertices, indices);
	generate_tangent(vertices, indices);
	Mesh mesh(std::move(vertices), std::move(indices), {});
	std::shared_ptr<Model> model_ptr = std::make_unique<Model>();
	model_ptr->meshs.push_back(std::move(mesh));
	model_ptr->directory = "create_quad";
	model_ptr->model_name_ = std::format("quad{}", ++counter);
	return model_ptr;
}


std::shared_ptr<Model> Loader::create_sphere() {
	std::vector<Vertex> vertices;
	constexpr uint X_SEGMENTS = 64;
	constexpr uint Y_SEGMENTS = 64;
	constexpr float PI = 3.141592653f;
	for (uint y = 0; y <= Y_SEGMENTS; ++y) {
		for (uint x = 0; x <= X_SEGMENTS; ++x) {
			float x_segment = float(x) / float(X_SEGMENTS);
			float y_segment = float(y) / float(Y_SEGMENTS);
			float xpos = std::cos(x_segment * 2.0f * PI) * std::sin(y_segment * PI);
			float ypos = std::cos(y_segment * PI);
			float zpos = std::sin(x_segment * 2.0f * PI) * std::sin(y_segment * PI);
			glm::vec3 normal = glm::normalize(glm::vec3(xpos, ypos, zpos));
			vertices.push_back({ glm::vec3(xpos, ypos, zpos), glm::vec2(x_segment, y_segment), normal });
		}
	}

	std::vector<uint> indices;
	bool odd_row = false;
	for (uint y = 0; y < Y_SEGMENTS; ++y) {
		if (!odd_row) {
			for (int x = 0; x <= X_SEGMENTS; ++x) {
				indices.push_back(y     * (X_SEGMENTS + 1) + x);
				indices.push_back((y+1) * (X_SEGMENTS + 1) + x);
			}
		} else {
			for (int x = X_SEGMENTS; x >= 0; --x) {
				indices.push_back((y+1) * (X_SEGMENTS + 1) + x);
				indices.push_back(y     * (X_SEGMENTS + 1) + x);
			}
		}
		odd_row = !odd_row;
	}

	static int counter = 0;
	//generate_normal(vertices, indices);
	generate_tangent(vertices, indices);
	Mesh mesh(std::move(vertices), std::move(indices), {});
	mesh.draw_type_ = GL_TRIANGLE_STRIP;
	std::shared_ptr<Model> model_ptr = std::make_unique<Model>();
	model_ptr->meshs.push_back(std::move(mesh));
	model_ptr->directory = "create_sphere";
	model_ptr->model_name_ = std::format("sphere{}", ++counter);
	return model_ptr;
}

std::shared_ptr<Model> Loader::create_skybox() {
	std::vector<Vertex> vertices = {
		#include "skybox/vertex.txt"
	};

	std::vector<uint> indices;
	indices.reserve(vertices.size());
	std::generate_n(std::back_inserter(indices), vertices.size(), [n = 0]() mutable {
		return n++;
	});

	static int counter = 0;
	generate_normal(vertices, indices);
	generate_tangent(vertices, indices);
	Mesh mesh(std::move(vertices), std::move(indices), {});
	std::shared_ptr<Model> model_ptr = std::make_unique<Model>();
	model_ptr->meshs.push_back(std::move(mesh));
	model_ptr->directory = "create_skybox";
	model_ptr->model_name_ = std::format("skybox{}", ++counter);
	return model_ptr;
}

const std::shared_ptr<ImageInfo> Loader::load_image(const std::string &path) {
	if (auto iter = image_cache.find(path); iter != image_cache.end())
		return iter->second;

	std::shared_ptr<ImageInfo> res = std::make_shared<ImageInfo>();
	res->data = stbi_load(path.c_str(), &res->width, &res->height, &res->channel, 0);
	if (res->data == nullptr)
		return nullptr;

	image_cache.insert(std::make_pair(path, res));
	return res;
}

const std::shared_ptr<HdrImageInfo> Loader::load_hdr_image(const std::string &path) {
	if (auto iter = hdr_image_cache.find(path); iter != hdr_image_cache.end())
		return iter->second;

	stbi_set_flip_vertically_on_load(true);
	std::shared_ptr<HdrImageInfo> res = std::make_shared<HdrImageInfo>();
	res->data = stbi_loadf(path.c_str(), &res->width, &res->height, &res->channel, 0);
	stbi_set_flip_vertically_on_load(false);
	if (res->data == nullptr)
		return nullptr;

	hdr_image_cache.insert(std::make_pair(path, res));
	return res;
}

void Loader::generate_normal(std::vector<Vertex> &vertice, const std::vector<uint> &indices) {
	if (indices.size() < 3)
		return;

	std::vector<glm::vec3> temp_normal(vertice.size(), glm::vec3(0));
	size_t limit = indices.size() - 2;
	for (size_t i = 0; i < limit; i += 3) {
		Vertex &v1 = vertice[indices[i+0]];
		Vertex &v2 = vertice[indices[i+1]];
		Vertex &v3 = vertice[indices[i+2]];
		glm::vec3 edge1 = v2.position - v1.position;
		glm::vec3 edge2 = v3.position - v1.position;
		glm::vec3 normal = glm::cross(edge1, edge2);
		for (size_t j = 0; j < 3; ++j)
			temp_normal[indices[i+j]] += normal;
	}

	for (size_t i = 0; i < vertice.size(); ++i) {
		glm::vec3 normal = glm::normalize(temp_normal[i]);
		vertice[i].normal = normal;
	}
}

void Loader::generate_tangent(std::vector<Vertex> &vertice, const std::vector<uint> &indices) {
	if (indices.size() < 3)
		return;

	std::vector<glm::vec3> temp_tangent(vertice.size(), glm::vec3(0));
	size_t limit = indices.size() - 2;
	for (size_t i = 0; i < limit; i += 3) {
		const Vertex &v1 = vertice[indices[i+0]];
		const Vertex &v2 = vertice[indices[i+1]];
		const Vertex &v3 = vertice[indices[i+2]];
		glm::vec3 edge1 = v2.position - v1.position;
		glm::vec3 edge2 = v3.position - v1.position;
		float t1 = v2.texcoord.t - v1.texcoord.t;
		float t2 = v3.texcoord.t - v1.texcoord.t;
		glm::vec3 tangent = (t2 * edge1) - (t1 * edge2);
		for (size_t j = 0; j < 3; ++j)
			temp_tangent[indices[i+j]] += tangent;
	}

	for (size_t i = 0; i < vertice.size(); ++i) {
		Vertex &v = vertice[i];
		glm::vec3 t = temp_tangent[i];
		t -= v.normal * dot(t, v.normal);		// ’˝Ωª–ﬁ’˝
		t = glm::normalize(t);
		v.tangent = t;
		v.bitangent = cross(v.normal, t);
	}
}

GLuint Loader::load_texture2d(const std::string &path) {
	constexpr static std::array<int, 6> flag = {
		GL_REPEAT,						// WRAP_S
		GL_REPEAT,						// WRAP_T
		GL_LINEAR_MIPMAP_LINEAR,		// GL_TEXTURE_MIN_FILTER	
		GL_NEAREST,						// GL_TEXTURE_MAG_FILTER
		GL_RGB,		
		GL_RGBA,						
	};
	return load_texture2d_impl(path, flag);
}

GLuint Loader::load_texture2ds(const std::string &path) {
	constexpr static std::array<int, 6> flag = {
		GL_REPEAT,						// WRAP_S
		GL_REPEAT,						// WRAP_T
		GL_LINEAR_MIPMAP_LINEAR,		// GL_TEXTURE_MIN_FILTER	
		GL_NEAREST,						// GL_TEXTURE_MAG_FILTER
		GL_SRGB,
		GL_SRGB_ALPHA,	
	};
	return load_texture2d_impl(path, flag);
}

GLuint Loader::equirectangular_to_cube_map(const std::string &path, int width, int height) {
	std::shared_ptr<HdrImageInfo> image_ptr = load_hdr_image(path);
	auto hdr_channel_type = image_ptr->channel == 3 ? GL_RGB : GL_RGBA;
	if (image_ptr == nullptr) {
		std::cerr << std::format("equirectangular_to_cube_map::load_hdr_image error: load {} error", path);
		return 0;
	}
	
	Shader to_cube_shader("shader/to_cube_map/to_cube_map.vert", "shader/to_cube_map/to_cube_map.frag");
	if (!to_cube_shader) {
		std::cerr << "Failed initialize to_cube_shader" << std::endl;
		return 0;
	}

	GLuint equirectangular_map;
	glGenTextures(1, &equirectangular_map);
	glBindTexture(GL_TEXTURE_2D, equirectangular_map);
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, image_ptr->width, image_ptr->height, 0, hdr_channel_type, GL_FLOAT, image_ptr->data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	glm::vec3 look_from(0, 0, 0);
	glm::mat4 projection = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.f);
	glm::mat4 capture_view[] = {
		glm::lookAt(look_from, glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(look_from, glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(look_from, glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(look_from, glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(look_from, glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(look_from, glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	GLuint env_cub_map;
	glGenTextures(1, &env_cub_map);
	glBindTexture(GL_TEXTURE_CUBE_MAP, env_cub_map);
	{
		for (int i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	CheckError();
	GLuint capture_fbo;
	GLuint capture_rbo;
	glGenFramebuffers(1, &capture_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
	{
		glGenRenderbuffers(1, &capture_rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, capture_rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, capture_rbo);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	to_cube_shader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, equirectangular_map);
	to_cube_shader.set_uniform("texture_map", 0);
	to_cube_shader.set_uniform("projection", projection);

	glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
	auto cube_ptr = Loader::create_trest_cube();
	for (int i = 0; i < 6; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, env_cub_map, 0);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		to_cube_shader.set_uniform("view", capture_view[i]);
		cube_ptr->draw(to_cube_shader);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDeleteTextures(1, &equirectangular_map);
	glDeleteFramebuffers(1, &capture_fbo);
	glDeleteRenderbuffers(1, &capture_rbo);
	texture_cache.insert(std::make_pair(path, env_cub_map));
	return env_cub_map;
}

GLuint Loader::irradiance_convolution(GLuint env_map, int width, int heght) {
	auto cube_ptr = Loader::create_trest_cube();
	Shader convolution_shader("shader/irradiance/irradiance_convolution.vert", "shader/irradiance/irradiance_convolution.frag");
	if (!convolution_shader) {
		std::cerr << "Failed irradiance_convolution shader" << std::endl;
		return 0;
	}

	GLuint res;
	glGenTextures(1, &res);
	glBindTexture(GL_TEXTURE_CUBE_MAP, res);
	{
		for (int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, heght, 
				0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	GLuint fbo;
	GLuint rbo;
	glGenFramebuffers(1, &fbo);
	glGenRenderbuffers(1, &rbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	{
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, heght);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	CheckError();
	glm::vec3 look_from(0, 0, 0);
	glm::mat4 projection = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.f);
	glm::mat4 capture_view[] = {
		glm::lookAt(look_from, glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(look_from, glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(look_from, glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(look_from, glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(look_from, glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(look_from, glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	convolution_shader.use();
	convolution_shader.set_uniform("projection", projection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, env_map);
	convolution_shader.set_uniform("env_map", 0);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	{
		for (int i = 0; i < 6; ++i) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, res, 0);
			glViewport(0, 0, width, heght);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			convolution_shader.set_uniform("view", capture_view[i]);
			cube_ptr->draw(convolution_shader);
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	static int counter = 0;
	glDeleteFramebuffers(1, &fbo);
	glDeleteRenderbuffers(1, &rbo);
	std::string key = std::format("irradiance_convolution{}", ++counter);
	texture_cache.insert(std::make_pair(key, res));
	return res;
}

GLuint Loader::prefilter(GLuint env_map, int width /*= 512*/, int height /*= 512*/) {
	auto cube_ptr = Loader::create_trest_cube();
	Shader prefilter_shader("shader/to_cube_map/to_cube_map.vert", "shader/pbr/prefilter.frag");
	if (!prefilter_shader) {
		std::cerr << "Failed initialize prefilter shader " << std::endl;
		return 0;
	}

	GLuint prefilter_map;
	glGenTextures(1, &prefilter_map);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_map);
	{
		for (int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 
				0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	GLuint fbo;
	GLuint rbo;
	glGenFramebuffers(1, &fbo);
	glGenRenderbuffers(1, &rbo);

	glm::vec3 look_from(0, 0, 0);
	glm::mat4 projection = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.f);
	glm::mat4 capture_view[] = {
		glm::lookAt(look_from, glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(look_from, glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(look_from, glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(look_from, glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(look_from, glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(look_from, glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	{
		prefilter_shader.use();
		prefilter_shader.set_uniform("projection", projection);
		prefilter_shader.set_uniform("env_map", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, env_map);
		constexpr int max_mip_level = 5;
		for (int mip = 0; mip < max_mip_level; ++mip) {
			int mip_width = static_cast<int>(width * std::pow(0.5, mip));
			int mip_height = static_cast<int>(height * std::pow(0.5, mip));
			glBindRenderbuffer(GL_RENDERBUFFER, rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mip_width, mip_height);
			glViewport(0, 0, mip_width, mip_height);
			float roughness = float(mip) / float(max_mip_level - 1);
			prefilter_shader.set_uniform("roughness", roughness);
			for (int i = 0; i < 6; ++i) {
				prefilter_shader.set_uniform("view", capture_view[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilter_map, mip);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				cube_ptr->draw(prefilter_shader);
			}
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDeleteFramebuffers(1, &fbo);
	glDeleteRenderbuffers(1, &rbo);
	static int counter = 0;
	std::string key = std::format("irradiance_convolution{}", ++counter);
	texture_cache.insert(std::make_pair(key, prefilter_map));
	return prefilter_map;
}

GLuint Loader::brdf_lut(GLuint env_map, int width /*= 512*/, int height /*= 512*/) {
	auto quad_ptr = Loader::create_quad();
	Shader brdf_shader("shader/pbr/integrate_brdf.vert", "shader/pbr/integrate_brdf.frag");
	if (!brdf_shader) {
		std::cerr << "Failed initialize brdf_shader error" << std::endl;
		return 0;
	}

	GLuint integrate_brdf_map;
	glGenTextures(1, &integrate_brdf_map);
	glBindTexture(GL_TEXTURE_2D, integrate_brdf_map);
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	GLuint fbo;
	GLuint rbo;
	glGenFramebuffers(1, &fbo);
	glGenRenderbuffers(1, &rbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	{
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, integrate_brdf_map, 0);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		brdf_shader.use();
		brdf_shader.set_uniform("env_map", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, env_map);
		quad_ptr->draw(brdf_shader);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDeleteFramebuffers(1, &fbo);
	glDeleteRenderbuffers(1, &rbo);
	static int counter = 0;
	std::string key = std::format("integrate_brdf{}", ++counter);
	texture_cache.insert(std::make_pair(key, integrate_brdf_map));
	return integrate_brdf_map;
}

void Loader::destroy() {
	for (auto &&[_, ptr] : image_cache) {
		if (ptr != nullptr && ptr->data != nullptr)
			stbi_image_free(ptr->data);
	}
	image_cache.clear();

	for (auto &&[_, texture] : texture_cache)
		glDeleteTextures(1, &texture);
	texture_cache.clear();

	for (auto &&[_, ptr] : hdr_image_cache) {
		if (ptr != nullptr && ptr->data != nullptr)
			stbi_image_free(ptr->data);
	}
	hdr_image_cache.clear();
}

GLuint Loader::load_texture2d_impl(const std::string &path, std::array<int, 6> flag) {
	if (auto iter = texture_cache.find(path); iter != texture_cache.end())
		return iter->second;

	std::shared_ptr<ImageInfo> image_ptr = load_image(path);
	if (image_ptr == nullptr) {
		std::cerr << "load_texture::load_image error: " << path << std::endl;
		assert(false);
		return -1;
	}

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, flag[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, flag[1]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, flag[2]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, flag[3]);

		//auto type = image_ptr->channel == 3 ? flag[4] : flag[5];
		int type = 0;
		int store_type = 0;
		if (image_ptr->channel == 3) {
			type = GL_RGB;
			store_type = flag[4];
		} else {
			type = GL_RGBA;
			store_type = flag[5];
		}
		glTexImage2D(GL_TEXTURE_2D, 0, store_type, image_ptr->width, image_ptr->height, 0, type, GL_UNSIGNED_BYTE, image_ptr->data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	
	texture_cache.insert(std::make_pair(path, texture));
	return texture;
}

void Loader::process_node(aiNode *node, const aiScene *scene, std::shared_ptr<Model> &model_ptr) {
	for (size_t i = 0; i < node->mNumMeshes; ++i) {
		int index = node->mMeshes[i];
		aiMesh *mesh = scene->mMeshes[index];
		model_ptr->meshs.push_back(process_mesh(mesh, scene, model_ptr));
	}
	for (size_t i = 0; i < node->mNumChildren; ++i)
		process_node(node->mChildren[i], scene, model_ptr);
}

Mesh Loader::process_mesh(aiMesh *mesh, const aiScene *scene, std::shared_ptr<Model> &model_ptr) {
	std::vector<Vertex> vertices;
	for (size_t i = 0; i < mesh->mNumVertices; ++i) {
		Vertex vertex;
		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;
		vertex.normal.x = mesh->mNormals[i].x;
		vertex.normal.y = mesh->mNormals[i].y;
		vertex.normal.z = mesh->mNormals[i].z;
		if (mesh->mTextureCoords[0]) {
			vertex.texcoord.s = mesh->mTextureCoords[0][i].x;
			vertex.texcoord.t = mesh->mTextureCoords[0][i].y;
			vertex.tangent.x = mesh->mTangents[i].x;
			vertex.tangent.y = mesh->mTangents[i].y;
			vertex.tangent.z = mesh->mTangents[i].z;
			vertex.bitangent.x = mesh->mBitangents[i].x;
			vertex.bitangent.y = mesh->mBitangents[i].y;
			vertex.bitangent.z = mesh->mBitangents[i].z;
		} else {
			vertex.texcoord = glm::vec2(0);
			vertex.tangent = glm::vec3(0);
			vertex.bitangent = glm::vec3(0);
		}

		vertices.push_back(vertex);
	}

	std::vector<uint> indices;
	for (size_t i = 0; i < mesh->mNumFaces; ++i) {
		const aiFace &face = mesh->mFaces[i];
		std::copy(face.mIndices, face.mIndices + face.mNumIndices, std::back_insert_iterator(indices));
	}

	std::vector<Texture> textures;
	int index = mesh->mNumAnimMeshes;
	if (index >= 0) {
		aiMaterial *material = scene->mMaterials[index];
#if 1
		std::vector<Texture> diffuse_map = load_material_textures(material, aiTextureType_DIFFUSE, "diffuse_map", model_ptr);
		textures.insert(textures.end(), std::move_iterator(diffuse_map.begin()), std::move_iterator(diffuse_map.end()));
		std::vector<Texture> specular_map = load_material_textures(material, aiTextureType_SPECULAR, "specular_map", model_ptr);
		textures.insert(textures.end(), std::move_iterator(specular_map.begin()), std::move_iterator(specular_map.end()));
		std::vector<Texture> normal_map = load_material_textures(material, aiTextureType_HEIGHT, "normal_map", model_ptr);
		textures.insert(textures.end(), std::move_iterator(normal_map.begin()), std::move_iterator(normal_map.end()));
#else
		auto albedo_map = load_material_textures(material, aiTextureType_BASE_COLOR, "albedo_map", model_ptr);
		textures.insert(textures.end(), std::move_iterator(albedo_map.begin()), std::move_iterator(albedo_map.end()));
		auto normal_map = load_material_textures(material, aiTextureType_NORMAL_CAMERA, "normal_map", model_ptr);
		textures.insert(textures.end(), std::move_iterator(normal_map.begin()), std::move_iterator(normal_map.end()));
		auto emission_map = load_material_textures(material, aiTextureType_EMISSION_COLOR, "emission_map", model_ptr);
		textures.insert(textures.end(), std::move_iterator(emission_map.begin()), std::move_iterator(emission_map.end()));
		auto metallic_map = load_material_textures(material, aiTextureType_METALNESS, "metallic_map", model_ptr);
		textures.insert(textures.end(), std::move_iterator(metallic_map.begin()), std::move_iterator(metallic_map.end()));
		auto roughness_map = load_material_textures(material, aiTextureType_DIFFUSE_ROUGHNESS, "roughness_map", model_ptr);
		textures.insert(textures.end(), std::move_iterator(roughness_map.begin()), std::move_iterator(roughness_map.end()));
		auto ambient_occlusion = load_material_textures(material, aiTextureType_AMBIENT_OCCLUSION, "ambient_occlusion", model_ptr);
		textures.insert(textures.end(), std::move_iterator(ambient_occlusion.begin()), std::move_iterator(ambient_occlusion.end()));
#endif
	}
	return { std::move(vertices), std::move(indices), std::move(textures) };
}

std::vector<Texture> Loader::load_material_textures(aiMaterial *material, aiTextureType type, const std::string &var_name,
	std::shared_ptr<Model> &model_ptr) 
{
	std::vector<Texture> textures;
	for (size_t i = 0; i < material->GetTextureCount(type); ++i) {
		aiString str;
		material->GetTexture(type, uint(i), &str);
		std::string path = model_ptr->directory + "/" + str.C_Str();
		GLuint texture_obj;
		if (type == aiTextureType_DIFFUSE)
			texture_obj = load_texture2ds(path);
		else
			texture_obj = load_texture2d(path);
		textures.emplace_back(texture_obj, var_name, type);
	}
	return textures;
}