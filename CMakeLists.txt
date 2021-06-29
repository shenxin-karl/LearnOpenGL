cmake_minimum_required(VERSION 3.8)	
set(CMAKE_CXX_STANDARD 20)
project(LearnOpenGL)

add_executable(LearnOpenGL
	"glad/include/glad/glad.h"		"glad/src/glad.c"
	"include/common.h"				"src/main.cpp"
	"include/loader.h"				"src/loader.cpp"	"include/mesh.h"				"src/mesh.cpp"
	"include/model.h"				"src/model.cpp"
	"include/shader.h"				"src/shader.cpp" 
	"include/scene.h"				"src/scene.cpp"		
 	"include/camera.h"				"src/camera.cpp"
	"include/utility.h"				"src/utility.cpp" 
)

target_include_directories(LearnOpenGL PUBLIC
	${PROJECT_SOURCE_DIR}/glfw/include
	${PROJECT_SOURCE_DIR}/assimp/include
	${PROJECT_SOURCE_DIR}/glad/include
	${PROJECT_SOURCE_DIR}/stb/include
	${PROJECT_SOURCE_DIR}/glm
	${PROJECT_SOURCE_DIR}/resources
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/shader
)

target_link_directories(LearnOpenGL PUBLIC
	${PROJECT_SOURCE_DIR}/glfw/lib
	${PROJECT_SOURCE_DIR}/assimp/lib
)

target_link_libraries(LearnOpenGL PUBLIC
	glfw3
	assimp-vc142-mt
)