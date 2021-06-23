#pragma once

#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <type_traits>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <string_view>
#include <filesystem>

#include <stb_image.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


struct Vertex;
struct Texture;
struct ImageInfo;

class Loader;
class Mesh;
class Model;
class Shader;

using uint = unsigned int;

#include "loader.h"
#include "mesh.h"
#include "model.h"
#include "shader.h"
#include "camera.h"
#include "utility.h"
#include "scene.h"
