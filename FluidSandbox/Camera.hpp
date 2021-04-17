/*
======================================================================================================================
	Fluid Sandbox - Camera.hpp

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#pragma once

#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct CCamera {
	glm::mat4 projection;
	glm::mat4 modelview;
	glm::mat4 mvp;
	glm::vec3 position;
	glm::vec2 rotation;
	float nearClip;
	float farClip;
	float fieldOfView;
	float aspect;

	CCamera() {
		projection = glm::mat4(1.0f);
		modelview = glm::mat4(1.0f);
		mvp = glm::mat4(1.0f);
		position = glm::vec3(0.0f);
		rotation = glm::vec2(0.0f);
		nearClip = 0.1f;
		farClip = 1000.0f;
		fieldOfView = 60.0f;
		aspect = 4.0f / 3.0f;
		Update();
	}

	CCamera(const float px, const float py, const float pz, const float rx, const float ry, const float znear, const float zfar, const float fov, const float aspect) {
		projection = glm::mat4(1.0f);
		modelview = glm::mat4(1.0f);
		mvp = glm::mat4(1.0f);
		position.x = px;
		position.y = py;
		position.z = pz;
		rotation = glm::vec2(rx, ry);
		nearClip = znear;
		farClip = zfar;
		fieldOfView = fov;
		this->aspect = aspect;
		Update();
	}

	inline void Update() {
		glm::mat4 mv = glm::translate(glm::mat4(1.0f), glm::vec3(-position[0], -position[1], -position[2]));
		glm::mat4 rot = glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		rot = glm::rotate(rot, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		projection = glm::perspective(fieldOfView, aspect, aspect, farClip);
		modelview = mv * rot;
		mvp = projection * modelview;
	};
};