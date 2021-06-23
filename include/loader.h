#pragma once

struct ImageInfo {
	unsigned char *data;
	int width;
	int height;
	int channel;
};



class Loader {
public:
	static std::unique_ptr<Model> load_model(const std::string &path);
	static const std::shared_ptr<ImageInfo> load_image(const std::string &path);

	constexpr static std::array<int, 4> load_texture2d_default_flag = {
		GL_REPEAT,						// WRAP_S
		GL_REPEAT,						// WRAP_T
		GL_LINEAR_MIPMAP_LINEAR,		// GL_TEXTURE_MIN_FILTER	
		GL_NEAREST,						// GL_TEXTURE_MAG_FILTER
	};
	static GLuint load_texture2d(const std::string &path, std::array<int, 4> flag = load_texture2d_default_flag);
private:
	struct ImageCacheRecycle {
		~ImageCacheRecycle();
	};
	inline static ImageCacheRecycle image_cache_recycle;
	inline static std::unordered_map<std::string, std::shared_ptr<ImageInfo>> image_cache;
	inline static std::unordered_map<std::string, GLuint> texture2d_cache;
private:
	static void process_node(aiNode *node, const aiScene *scene, std::unique_ptr<Model> &model_ptr);
	static Mesh process_mesh(aiMesh *mesh, const aiScene *scene, std::unique_ptr<Model> &model_ptr);
	static std::vector<Texture> load_material_textures(aiMaterial *material, aiTextureType type, const std::string &var_name,
														std::unique_ptr<Model> &model_ptr);
};