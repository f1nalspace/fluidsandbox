#include "VBO.h"

CVBO::CVBO(void)
{
    ibo = 0;
    vbo = 0;
    indexCount = 0;
}

CVBO::~CVBO(void)
{
    clear();
}

void CVBO::clear()
{
    if(ibo)
        glDeleteBuffers(1, &ibo);

    ibo = 0;

    if(vbo)
        glDeleteBuffers(1, &vbo);

    vbo = 0;
    indexCount = 0;
}

void CVBO::bufferVertices(GLfloat* vertices, GLuint vertexSize, GLenum usage)
{
    if(vbo == 0)
    {
        glGenBuffers(1, &vbo);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexSize, vertices, usage);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CVBO::bufferIndices(GLuint* indices, GLuint indexCount, GLenum usage)
{
    if(ibo == 0)
    {
        glGenBuffers(1, &ibo);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLuint), indices, usage);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    this->indexCount = indexCount;
}


void CVBO::bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    /*
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    glNormalPointer(GL_FLOAT, sizeof(float) * 6, (void*)(sizeof(float) * 3));
    glVertexPointer(3, GL_FLOAT, sizeof(float) * 6, (void*)(0));
    */
}

void CVBO::drawElements(GLenum mode)
{
    glDrawElements(mode, indexCount, GL_UNSIGNED_INT, NULL);
}

void CVBO::unbind()
{
    /*
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    */

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

