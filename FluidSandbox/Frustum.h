#pragma once

#include <glm/glm.hpp> 

#include "Singleton.hpp"
class CFrustum: public CSingleton<CFrustum>
{
friend CSingleton<CFrustum>;
private:
	float frustum[6][4];
public:
	CFrustum(void);
	~CFrustum(void);
	void update(const float* proj, const float* modl);
	bool containsPoint(const glm::vec3 &pos);
	bool containsSphere(const glm::vec3 &pos, const float radius);
	bool containsBounds(const glm::vec3 &minimum, const glm::vec3 &maximum);
};

