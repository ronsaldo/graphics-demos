#include <assert.h>
#include <stdio.h>
#include "Shader.hpp"

namespace Tarea2
{

Shader::Shader()
{
    handle = 0;
}

Shader::~Shader()
{
    if(handle)
        glDeleteShader(handle);
}

void Shader::setName(const std::string &name)
{
    this->name = name;
}

const std::string &Shader::getName() const
{
    return this->name;
}

void Shader::setSource(const std::string &source)
{
    this->source = source;
}

const std::string &Shader::getSource() const
{
    return this->source;
}

GLuint Shader::getHandle() const
{
    return this->handle;
}

void Shader::compile()
{
    assert(!handle);

    // Create the shader.
    handle = glCreateShader(getTarget());

    // Set the shader source.
    const char *string = source.c_str();
    glShaderSource(handle, 1, &string, NULL);

    // Compile the shader.
    glCompileShader(handle);

    // Send the info log.
    emitInfoLog();

    // Check the error.
    checkStatus();
}

void Shader::emitInfoLog()
{
    // Get the info log length.
    GLint length;
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &length);

    if(length > 0)
    {
        // Get the info log.
        char *infoLog = new char[length+1];
        glGetShaderInfoLog(handle, length, NULL, infoLog);

        // Print and delete the info log.
        fprintf(stderr, "Info Log for '%s'\n%s", name.c_str(), infoLog);
        delete [] infoLog;
    }
}

void Shader::checkStatus()
{
    GLint status;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE)
        throw ShaderCompileError("Shader compile error.");
}

//-----------------------------------------------------------------------------
GpuProgram::GpuProgram()
{
    handle = glCreateProgram();
}

GpuProgram::~GpuProgram()
{
    // Detach and delete the shaders.
    Shaders::iterator it = shaders.begin();
    for(; it != shaders.end(); it++)
    {
        Shader *shader = *it;
        glDetachShader(handle, shader->getHandle());
        delete shader;
    }

    // Delete the program.
    glDeleteProgram(handle);
}

void GpuProgram::setName(const std::string &name)
{
    this->name = name;
}

const std::string &GpuProgram::getName() const
{
    return this->name;
}

void GpuProgram::attach(Shader *shader)
{
    assert(shader);
    glAttachShader(handle, shader->getHandle());
    shaders.push_back(shader);
}

void GpuProgram::detach(Shader *shader)
{
    assert(shader);
    glDetachShader(handle, shader->getHandle());

    Shaders::iterator it = shaders.begin();
    for(; it != shaders.end(); it++)
    {
        if(*it == shader)
        {
            shaders.erase(it);
            break;
        }
    }
}

void GpuProgram::bindAttribute(const std::string &name, int bindId)
{
    glBindAttribLocation(handle, bindId, name.c_str());
}

void GpuProgram::link()
{
    // Link the program.
    glLinkProgram(handle);

    // Emit the info log.
    emitInfoLog();

    // Check the status.
    checkStatus();
}

void GpuProgram::use()
{
    glUseProgram(handle);
}

void GpuProgram::emitInfoLog()
{
    // Get the info log length.
    GLint length;
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &length);
    if(length > 0)
    {
        // Get the info log.
        char *infoLog = new char[length+1];
        glGetProgramInfoLog(handle, length, NULL, infoLog);

        // Print and delete the info log.
        fprintf(stderr, "Info Log for program '%s'\n%s", name.c_str(), infoLog);
        delete [] infoLog;
    }
}

void GpuProgram::checkStatus()
{
    GLint status;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);
    if(status == GL_FALSE)
        throw ProgramLinkError("Program link error.");
}

void GpuProgram::uniform(const std::string &name, float scalar)
{
    GLint loc = glGetUniformLocation(handle, name.c_str());
    glUniform1f(loc, scalar);
}

void GpuProgram::uniform(const std::string &name, const Vector2 &vec2)
{
    GLint loc = glGetUniformLocation(handle, name.c_str());
    glUniform2f(loc, vec2.x, vec2.y);
}

void GpuProgram::uniform(const std::string &name, const Vector3 &vec3)
{
    GLint loc = glGetUniformLocation(handle, name.c_str());
    glUniform3f(loc, vec3.x, vec3.y, vec3.z);
}

void GpuProgram::uniform(const std::string &name, const Color &color)
{
    GLint loc = glGetUniformLocation(handle, name.c_str());
    glUniform4f(loc, color.r, color.g, color.b, color.a);
}

void GpuProgram::uniform(const std::string &name, const Matrix3 &matrix)
{
    GLint loc = glGetUniformLocation(handle, name.c_str());
    glUniformMatrix3fv(loc, 1, GL_TRUE, matrix.rawData());
}

void GpuProgram::uniform(const std::string &name, const Matrix4 &matrix)
{
    GLint loc = glGetUniformLocation(handle, name.c_str());
    glUniformMatrix4fv(loc, 1, GL_TRUE, matrix.rawData());
}


} // namespace Tarea2

