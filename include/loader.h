#pragma once

struct ImageInfo {
	unsigned char *data;
	int width;
	int height;
	int channel;
};

struct HdrImageInfo {
	float *data;
	int width;
	int height;
	int channel;
};

class Loader {
public:
	static std::shared_ptr<Model> load_model(const std::string &path);
	static std::shared_ptr<Model> create_test_plane();
	static std::shared_ptr<Model> create_trest_cube();
	static std::shared_ptr<Model> create_quad();
	static std::shared_ptr<Model> create_sphere();		// ��Ⱦ��ʱ���ܿ�������ü�
	static std::shared_ptr<Model> create_skybox();
	static const std::shared_ptr<ImageInfo> load_image(const std::string &path);
	static const std::shared_ptr<HdrImageInfo> load_hdr_image(const std::string &path);
	static void generate_normal(std::vector<Vertex> &vertice, const std::vector<uint> &indices);
	static void generate_tangent(std::vector<Vertex> &vertice, const std::vector<uint> &indices);
	static GLuint load_texture2d(const std::string &path);
	static GLuint load_texture2ds(const std::string &path);
	static GLuint equirectangular_to_cube_map(const std::string &path, int width = 512, int height = 512);
	static GLuint irradiance_convolution(GLuint env_map, int width = 32, int height = 32);
	static GLuint prefilter(GLuint env_map, int width = 512, int height = 512);
	static GLuint brdf_lut(GLuint env_map, int width = 512, int height = 512);
	static void destroy();
private:
	static GLuint load_texture2d_impl(const std::string &path, std::array<int, 6> flag);
	inline static std::unordered_map<std::string, std::shared_ptr<ImageInfo>>	 image_cache;
	inline static std::unordered_map<std::string, std::shared_ptr<HdrImageInfo>> hdr_image_cache;
	inline static std::unordered_map<std::string, GLuint> texture_cache;
private:
	static void process_node(aiNode *node, const aiScene *scene, std::shared_ptr<Model> &model_ptr);
	static Mesh process_mesh(aiMesh *mesh, const aiScene *scene, std::shared_ptr<Model> &model_ptr);
	static std::vector<Texture> load_material_textures(aiMaterial *material, aiTextureType type, const std::string &var_name,
														std::shared_ptr<Model> &model_ptr);
};