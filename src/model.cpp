#include "common.h"

void Model::draw(Shader &shader) const {
	for (auto &mesh : meshs)
		mesh.draw(shader);
}

