#ifndef _TAREA2_RENDERER_HPP
#define _TAREA2_RENDERER_HPP

#include "Event.hpp"
#include "EventListener.hpp"
#include "Vector3.hpp"
#include "Matrix4.hpp"
#include "MeshBuilder.hpp"

namespace Tarea2
{

class Game;
class GpuProgram;

/**
 * Game renderer.
 */
class Renderer: EventListener
{
public:
    Renderer(Game *game);
    ~Renderer();

    bool initialize();
    virtual void handleEvent(Event *event);

    Vector2 windowToView(int x, int y) const;

    SimpleMeshBuilder *getBackgroundBuilder();
    SimpleMeshBuilder *getShadowBuilder();
    SimpleMeshBuilder *getNoShadowBuilder();


private:
    GpuProgram *createRenderProgram(const char *vertexShaderName, const char *vertexShaderSource,
        const char *fragmentShaderName, const char *fragmentShaderSource);

    void prepareFrame();
    void setupShadowVolumes();
    void setupAmbient();
    void setupReceivers();

    void drawBackground();
    void drawShadowCasters();
    void drawNoShadow();

    void renderFrame();
    void sendUniforms(GpuProgram *program);
    void sendModelView(GpuProgram *program, const Matrix4 &modelView);

    Game *game;

    SimpleMeshBuilder backgroundBuilder;
    SimpleMeshBuilder shadowBuilder;
    SimpleMeshBuilder noShadowBuilder;

    // Programs.
    GpuProgram *colorProgram;
    GpuProgram *lightProgram;
    GpuProgram *shadowVolumeProgram;

    // View bounds.
    float viewLeft, viewRight;
    float viewBottom, viewTop;

    // Uniforms.
    Matrix4 projectionMatrix;
    Matrix4 modelViewMatrix;

    // Shadow volumes.
    float shadowExtrusion;

    // Lighting.
    float lightRadius;
    Vector2 lightPosition;
    Color lightColor;
    Color ambientLight;
};

} // namespace Tarea2

#endif //_TAREA2_RENDERER_HPP
