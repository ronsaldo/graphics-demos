#ifndef _TAREA2_SHADER_HPP
#define _TAREA2_SHADER_HPP

#include <GL/glew.h>

#include <string>
#include <vector>
#include <stdexcept>

#include "Color.hpp"
#include "Vector2.hpp"
#include "Vector3.hpp"
#include "Matrix3.hpp"
#include "Matrix4.hpp"

namespace Tarea2
{

/**
 * Shade compile error.
 */
class ShaderCompileError: std::runtime_error
{
public:
    explicit ShaderCompileError(const std::string& what_arg)
        : std::runtime_error(what_arg) {}
};

/**
 * Program link error.
 */
class ProgramLinkError: std::runtime_error
{
public:
    explicit ProgramLinkError(const std::string& what_arg)
        : std::runtime_error(what_arg) {}

};

/**
 *  Shader abstract base.
 */
class Shader
{
public:
    Shader();
    virtual ~Shader();

    virtual GLenum getTarget() = 0;

    void setName(const std::string &name);
    const std::string &getName() const;

    void setSource(const std::string &source);
    const std::string &getSource() const;

    GLuint getHandle() const;

    void compile();

private:
    void emitInfoLog();
    void checkStatus();

    GLuint handle;
    std::string name, source;
};

/**
 *  Vertex shader.
 */
class VertexShader: public Shader
{
public:
    virtual GLenum getTarget()
    {
        return GL_VERTEX_SHADER;
    }
};

/**
 *  Fragment shader.
 */
class FragmentShader: public Shader
{
public:
    virtual GLenum getTarget()
    {
        return GL_FRAGMENT_SHADER;
    }
};

/**
 * Gpu program
 */
class GpuProgram
{
public:
    GpuProgram();
    ~GpuProgram();

    void setName(const std::string &name);
    const std::string &getName() const;

    void attach(Shader *shader);
    void detach(Shader *shader);

    void bindAttribute(const std::string &name, int bindId);

    void link();

    void use();

    void uniform(const std::string &name, float scalar);
    void uniform(const std::string &name, const Vector2 &vec2);
    void uniform(const std::string &name, const Vector3 &vec3);
    void uniform(const std::string &name, const Color &color);
    void uniform(const std::string &name, const Matrix3 &matrix);
    void uniform(const std::string &name, const Matrix4 &matrix);

private:
    void emitInfoLog();
    void checkStatus();

    typedef std::vector<Shader*> Shaders;
    Shaders shaders;

    GLuint handle;
    std::string name;
};

} // namespace Tarea2

#endif //_TAREA2_SHADER_HPP

