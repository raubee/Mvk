#include "Camera.h"

Camera::Camera() :
	position(glm::vec3(0)),
	rotation(glm::vec3(0)),
	target(glm::vec3(0)),
	viewMatrix(glm::mat4(1)),
	projMatrix(glm::mat4(1)),
	fov(0.0f),
	zNear(0.0f),
	zFar(0.0f),
	aspectRatio(0.0f),
	distance(1.0f),
	type(ORBIT)
{
}

void Camera::setType(const CameraType type)
{
	this->type = type;
	updateMatrix();
}

void Camera::updateMatrix()
{
	if (type == CameraType::FLY)
	{
		auto trans = glm::mat4(1);
		auto rot = glm::mat4(1);

		trans = glm::translate(trans, position);

		rot = glm::rotate(rot, rotation.x, Camera::down);
		rot = glm::rotate(rot, rotation.y, Camera::left);
		rot = glm::rotate(rot, rotation.z, Camera::forward);

		viewMatrix = rot * trans;
	}
	else if (type == CameraType::ORBIT)
	{
		position = glm::euclidean(glm::vec2(phi, theta)) * distance + target;
		viewMatrix = glm::lookAt(position, target, Camera::up);
	}
}

void Camera::setPerspective(const float fov, const float aspectRatio,
                            const float near, const float far)
{
	this->fov = fov;
	this->aspectRatio = aspectRatio;
	this->zNear = near;
	this->zFar = far;

	projMatrix = glm::perspective(fov, aspectRatio, zNear, zFar);
}

void Camera::updateAspectRatio(const float aspectRatio)
{
	this->aspectRatio = aspectRatio;
	projMatrix = glm::perspective(fov, aspectRatio, zNear, zFar);
}

void Camera::setPosition(const glm::vec3 worldPosition)
{
	this->position = worldPosition;
	updateMatrix();
}

void Camera::translate(const glm::vec3 translation)
{
	this->position += translation;
	updateMatrix();
}

void Camera::rotate(const glm::vec3 rotation)
{
	if (type == FLY)
	{
		this->rotation += rotation;
	}
	else if (type == ORBIT)
	{
		this->theta += rotation.x;
		this->phi += rotation.y;
	}
	updateMatrix();
}

void Camera::setLookAt(const glm::vec3 worldTarget)
{
	this->target = worldTarget;
	updateMatrix();
}

void Camera::pan(const glm::vec3 translation)
{
	if (type == ORBIT)
	{
		const auto w = glm::euclidean(glm::vec2(phi, theta));
		const auto u = glm::cross(w, Camera::up);
		const auto v = glm::cross(u, w);
		const auto pan = glm::mat3(u, v, w);
		this->target += pan * translation;
	}
	else if (type == FLY)
	{
		auto rot = glm::mat4(1);

		rot = glm::rotate(rot, rotation.x, Camera::down);
		rot = glm::rotate(rot, rotation.y, Camera::left);
		rot = glm::rotate(rot, rotation.z, Camera::forward);

		const auto t = glm::vec4(translation, 0.0f) * rot;
		this->position += glm::vec3(t.x, -t.y, t.z);
	}

	updateMatrix();
}

void Camera::setDistance(const float distance)
{
	this->distance = distance;
	updateMatrix();
}

void Camera::zoom(const float zoom)
{
	this->distance += zoom;
	updateMatrix();
}
