#include "common.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//ImageCacheRecycle Loader::image_cache_recycle;
//std::unordered_map<std::string, std::shared_ptr<ImageInfo>> image_cache;
//std::unordered_map<std::string, GLuint> texture2d_cache;

std::unique_ptr<Model> Loader::load_model(const std::string &path) {
	Assimp::Importer importer;
	auto flag = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace;
	const aiScene *scene = importer.ReadFile(path, flag);
	if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr) {
		std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return nullptr;
	}
	
	std::unique_ptr<Model> model_ptr = std::make_unique<Model>();
	model_ptr->directory = path.substr(0, path.find_last_of('/'));
	process_node(scene->mRootNode, scene, model_ptr);
	return model_ptr;
}	

Loader::ImageCacheRecycle::~ImageCacheRecycle() {
	return;
	for (auto &&[_, ptr] : image_cache) {
		if (ptr != nullptr && ptr->data != nullptr) {
			stbi_image_free(ptr->data);
			ptr->data = nullptr;
		}
	}
	image_cache.clear();

	for (auto &&[_, texture] : texture2d_cache)
		glDeleteTextures(1, &texture);
	texture2d_cache.clear();
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

GLuint Loader::load_texture2d(const std::string &path, std::array<int, 4> flag) {
	if (auto iter = texture2d_cache.find(path); iter != texture2d_cache.end())
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

		auto type = image_ptr->channel == 3 ? GL_RGB : GL_RGBA;
		glTexImage2D(GL_TEXTURE_2D, 0, type, image_ptr->width, image_ptr->height, 0, type, GL_UNSIGNED_BYTE, image_ptr->data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	
	texture2d_cache.insert(std::make_pair(path, texture));
	return texture;
}

void Loader::process_node(aiNode *node, const aiScene *scene, std::unique_ptr<Model> &model_ptr) {
	for (size_t i = 0; i < node->mNumMeshes; ++i) {
		int index = node->mMeshes[i];
		aiMesh *mesh = scene->mMeshes[index];
		model_ptr->meshs.push_back(process_mesh(mesh, scene, model_ptr));
	}
	for (size_t i = 0; i < node->mNumChildren; ++i)
		process_node(node->mChildren[i], scene, model_ptr);
}

Mesh Loader::process_mesh(aiMesh *mesh, const aiScene *scene, std::unique_ptr<Model> &model_ptr) {
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
		} else {
			vertex.texcoord.s = 0.f;
			vertex.texcoord.t = 0.f;
		}
		vertex.tangent.x = mesh->mTangents[i].x;
		vertex.tangent.y = mesh->mTangents[i].y;
		vertex.tangent.z = mesh->mTangents[i].z;
		vertex.bitangent.x = mesh->mBitangents[i].x;
		vertex.bitangent.y = mesh->mBitangents[i].y;
		vertex.bitangent.z = mesh->mBitangents[i].z;
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
		std::vector<Texture> diffuse_map = load_material_textures(material, aiTextureType_DIFFUSE, "diffuse_map", model_ptr);
		textures.insert(textures.end(), std::move_iterator(diffuse_map.begin()), std::move_iterator(diffuse_map.end()));
		std::vector<Texture> specular_map = load_material_textures(material, aiTextureType_SPECULAR, "specular_map", model_ptr);
		textures.insert(textures.end(), std::move_iterator(specular_map.begin()), std::move_iterator(specular_map.end()));
		std::vector<Texture> normal_map = load_material_textures(material, aiTextureType_NORMALS, "normal_map", model_ptr);
		textures.insert(textures.end(), std::move_iterator(normal_map.begin()), std::move_iterator(normal_map.end()));
		std::vector<Texture> height_map = load_material_textures(material, aiTextureType_HEIGHT, "height_map", model_ptr);
		textures.insert(textures.end(), std::move_iterator(height_map.begin()), std::move_iterator(height_map.end()));
	}

	return { std::move(vertices), std::move(indices), std::move(textures) };
}

std::vector<Texture> Loader::load_material_textures(aiMaterial *material, aiTextureType type, const std::string &var_name,
	std::unique_ptr<Model> &model_ptr) 
{
	std::vector<Texture> textures;
	for (size_t i = 0; i < material->GetTextureCount(type); ++i) {
		aiString str;
		material->GetTexture(type, uint(i), &str);
		std::string path = model_ptr->directory + "/" + str.C_Str();
		GLuint texture_obj = load_texture2d(path);
		textures.emplace_back(texture_obj, var_name, type);
	}
	return textures;
}