#include "FBO.h"

#include <iostream>

CFBO::CFBO(int width, int height)
{
    this->width = width;
    this->height = height;
    this->bufferId = 0;
    this->textureCount = 0;
	for(uint32_t i = 0; i < MaxTextureCount; ++i)
		textures[i] = NULL;

    GLint temp;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &temp);
    maxColorAttachments = temp;
}

CFBO::~CFBO(void)
{
    // Remove textures
    for(std::map<FBOTextureID, CTexture2D*>::iterator it = textureMap.begin(); it != textureMap.end(); ++it)
        delete it->second;

    textureMap.clear();
    textureCount = 0;

    if(bufferId)
        glDeleteFramebuffers(1, &bufferId);
}

uint32_t CFBO::getMaxColorAttachments()
{
    int temp = 0;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &temp);
    uint32_t result = temp;
    return temp;
}

bool StatusFBO()
{
    GLenum error = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if(GL_FRAMEBUFFER_COMPLETE)
        return true;
    else
    {
        switch(error)
        {
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
                printf("    Incomplete attachment!\n");
                break;

            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
                printf("    Missing attachment!\n");
                break;

            case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
                printf("    Incomplete dimensions!\n");
                break;

            case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
                printf("    Incomplete formats!\n");
                break;

            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
                printf("    Incomplete draw buffer!\n");
                break;

            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
                printf("    Incomplete read buffer!\n");
                break;

            case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
                printf("    Framebufferobjects unsupported!\n");
                break;
        }

        return false;
    }
}

uint32_t CFBO::addTextureTarget(GLint internalFormat, GLenum format, GLenum type, GLenum fbotype, const FBOTextureID id, GLuint texfilter)
{
    assert(textureCount < MaxTextureCount);

    // Create new texture
    GLuint texFilters[2] = {texfilter, texfilter};
    CTexture2D* newtex = new CTexture2D(GL_TEXTURE_2D, internalFormat, format, type, width, height, &texFilters[0]);
    newtex->setUserData(fbotype);
    newtex->upload(NULL);

    // Add texture to array and map
    uint32_t result = textureCount++;
    textures[result] = newtex;
    textureMap[id] = newtex;
    return(result);
}

uint32_t CFBO::addRenderTarget(GLint internalFormat, GLenum format, GLenum type, GLenum fbotype, const FBOTextureID id, GLuint texfilter)
{
	assert(textureCount < MaxTextureCount);

    // Create new texture
    GLuint texFilters[2] = {texfilter, texfilter};
    CTexture2D* newtex = new CTexture2D(GL_TEXTURE_2D, internalFormat, format, type, width, height, &texFilters[0]);
    newtex->setUserData(fbotype);
    newtex->upload(NULL);

    // Add texture to array and map
    uint32_t result = textureCount++;
    textures[result] = newtex;
    textureMap[id] = newtex;
    return(result);
}

CTexture2D* CFBO::getTexture(const FBOTextureID id)
{
    return textureMap[id];
}

void CFBO::enable()
{
    if(bufferId)
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, bufferId);
}

void CFBO::disable()
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void CFBO::resize(int width, int height)
{
    this->width = width;
    this->height = height;

    // Recreate textures
    for(std::map<FBOTextureID, CTexture2D*>::iterator it = textureMap.begin(); it != textureMap.end(); ++it)
        it->second->resize(width, height);

    // Update FBO
    update();
}

void CFBO::update()
{
    // Create framebuffer if not present
    if(!bufferId)
        glGenFramebuffers(1, &bufferId);

    // Bind framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, bufferId);

    // Update/Add textures to framebuffer
    for(std::map<FBOTextureID, CTexture2D*>::iterator it = textureMap.begin(); it != textureMap.end(); ++it)
    {
        CTexture2D* tex = it->second;
        GLuint userdata = tex->getUserData();

        if(userdata > 0)  // We want only color buffer
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, userdata, GL_TEXTURE_2D, tex->getID(), 0);
    }

    // Check FBO Status
    StatusFBO();

    // Unbind framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void CFBO::changeDepthTexture(const FBOTextureID id)
{
    CTexture2D* tex = getTexture(id);

    if(tex)
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex->getID(), 0);
}

void CFBO::removeDepthTexture()
{
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
}

GLint CFBO::getDrawBuffer()
{
    GLint savedDrawBuffer;
    glGetIntegerv(GL_DRAW_BUFFER, &savedDrawBuffer);
    return savedDrawBuffer;
}

void CFBO::setDrawBuffer(GLenum buffer)
{
    glDrawBuffer(buffer);
}

void CFBO::setDrawBuffers(GLenum* buffers, int count)
{
    glDrawBuffers(count, buffers);
}


