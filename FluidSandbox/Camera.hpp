#pragma once

#include <GL/glew.h> 
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class CCamera
{
private:
	glm::vec3 fPosition;
	GLfloat fRotateX;
	GLfloat fRotateY;
	GLfloat fNearClip;
	GLfloat fFarClip;
	GLfloat fFieldOfView;
	GLfloat fAspect;
	glm::mat4 mProj;
	glm::mat4 mMV;
	glm::mat4 mMVP;

	inline void Update()
	{
		glm::mat4 modelview = glm::translate(glm::mat4(1.0f), glm::vec3(-fPosition[0], -fPosition[1], -fPosition[2])); 
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), fRotateX, glm::vec3(1.0f, 0.0f, 0.0f));
		rotation = glm::rotate(rotation, fRotateY, glm::vec3(0.0f, 1.0f, 0.0f));
		mProj = glm::perspective(fFieldOfView, fAspect, fNearClip, fFarClip); 
		mMV = modelview * rotation;
		mMVP = mProj * (modelview * rotation);
	};
public:

	CCamera()
	{
		mProj = glm::mat4(1.0f);
		mMVP = glm::mat4(1.0f);
		mMV = glm::mat4(1.0f);
		fPosition.x = 0.0f;
		fPosition.y = 0.0f;
		fPosition.z = 0.0f;
		fRotateX = 0.0f;
		fRotateY = 0.0f;
		fNearClip = 0.1f;
		fFarClip = 1000.0f;
		fFieldOfView = 60.0f;
		fAspect = 4.0f / 3.0f;
		Update();
	}

	CCamera(float px, float py, float pz, float rx, float ry, float znear, float zfar, float fov, float aspect)
	{
		mProj = glm::mat4(1.0f);
		mMVP = glm::mat4(1.0f);
		mMV = glm::mat4(1.0f);
		fPosition.x = px;
		fPosition.y = py;
		fPosition.z = pz;
		fRotateX = rx;
		fRotateY = ry;
		fNearClip = znear;
		fFarClip = zfar;
		fFieldOfView = fov;
		fAspect = aspect;
		Update();
	}

	inline glm::mat4 GetModelViewProjection()
	{
		return mMVP;
	}

	inline glm::mat4 GetModelview()
	{
		return mMV;
	}

	inline glm::mat4 GetProjection()
	{
		return mProj;
	}

	inline float GetNearClip()
	{
		return fNearClip;
	}

	inline float GetFarClip()
	{
		return fFarClip;
	}

	inline glm::vec3 GetPosition()
	{
		return fPosition;
	}
};