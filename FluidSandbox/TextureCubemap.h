/*
======================================================================================================================
	Fluid Sandbox - TextureCubemap.h

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#pragma once

#include "Texture2D.h"

class CTextureCubemap: public CTexture2D
{
protected:
public:
	CTextureCubemap(const int width, const int height);
	~CTextureCubemap(void);
	virtual void Upload(const uint8_t *pixels);
};

