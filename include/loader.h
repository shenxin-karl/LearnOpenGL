#pragma once

struct ImageInfo {
	unsigned char *data;
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
	static std::shared_ptr<Model> create_sphere();		// 渲染球时不能开启背面裁剪
	static const std::shared_ptr<ImageInfo> load_image(const std::string &path);
	static void generate_normal(std::vector<Vertex> &vertice, const std::vector<uint> &indices);
	static void generate_tangent(std::vector<Vertex> &vertice, const std::vector<uint> &indices);
	static GLuint load_texture2d(const std::string &path);
	static GLuint load_texture2ds(const std::string &path);
private:
	static GLuint load_texture2d_impl(const std::string &path, std::array<int, 6> flag);
	struct ImageCacheRecycle {
		~ImageCacheRecycle();
	};
	inline static ImageCacheRecycle image_cache_recycle;
	inline static std::unordered_map<std::string, std::shared_ptr<ImageInfo>> image_cache;
	inline static std::unordered_map<std::string, GLuint> texture2d_cache;
private:
	static void process_node(aiNode *node, const aiScene *scene, std::shared_ptr<Model> &model_ptr);
	static Mesh process_mesh(aiMesh *mesh, const aiScene *scene, std::shared_ptr<Model> &model_ptr);
	static std::vector<Texture> load_material_textures(aiMaterial *material, aiTextureType type, const std::string &var_name,
														std::shared_ptr<Model> &model_ptr);
};