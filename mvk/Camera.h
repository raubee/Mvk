#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/polar_coordinates.hpp>

class Camera
{
	const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	const glm::vec3 down = glm::vec3(0.0f, -1.0f, 0.0f);
	const glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
	const glm::vec3 backward = glm::vec3(0.0f, 0.0f, 1.0f);
	const glm::vec3 left = glm::vec3(1.0f, 0.0f, 0.0f);
	const glm::vec3 right = glm::vec3(-1.0f, 0.0f, 0.0f);

	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 target;

	float fov;
	float zNear;
	float zFar;
	float aspectRatio;

	float phi;
	float theta;
	float distance;

	void updateMatrix();

public:

	enum CameraType
	{
		ORBIT,
		FLY
	};

	CameraType type;

	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;

	Camera();

	void setType(CameraType type);

	void setPerspective(float fov, float aspectRatio, float near, float far);
	void updateAspectRatio(float aspectRatio);

	void setPosition(glm::vec3 worldPosition);
	void translate(glm::vec3 translation);
	void rotate(glm::vec3 rotation);

	void pan(glm::vec3 translation);
	void zoom(float zoom);

	void setDistance(float distance);
	void setLookAt(glm::vec3 worldTarget);
};
