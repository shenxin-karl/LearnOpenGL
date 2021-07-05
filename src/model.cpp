#include "common.h"

Model::Model() : model_(1) {
}

void Model::draw(Shader &shader) const {
	//shader.set_uniform("model", model_);
	for (auto &mesh : meshs)
		mesh.draw(shader);
}

const std::string &Model::get_name() const {
	return model_name_;
}

void Model::set_model(const glm::mat4 &model) {
	model_ = model;
}

void Model::show_widgets() {
	glm::vec3 translate = model_[3];
	float scales_array[] = {
		model_[0].x,
		model_[1].y,
		model_[2].z,
	};
	ImGui::Begin(model_name_.c_str());
	{
		ImGui::InputFloat3("scale", scales_array);
		ImGui::InputFloat3("translate", glm::value_ptr(translate));
	}
	ImGui::End();
	model_ = glm::scale(glm::mat4(1), glm::vec3(scales_array[0], scales_array[1], scales_array[2]));
	model_ = glm::translate(model_, translate);
}

const std::vector<Mesh> &Model::get_meshs() {
	return meshs;
}

