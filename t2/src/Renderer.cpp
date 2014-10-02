#include <GL/glew.h>
#include "Game.hpp"
#include "Renderer.hpp"
#include "Shader.hpp"

#include <stdio.h>

namespace Tarea2
{

// Vertex attributes bindings.
static const int VertexPositionBinding = 1;
static const int VertexColorBinding = 2;
static const int VertexNormalBinding = 3;

// Color gpu program.
static const char *ColorFragmentShaderSource = 
    "#version 120"                              "\n"
    "uniform vec4 ambientLight;"                "\n"
    "varying vec4 fColor;"                      "\n"
    "void main() {"                             "\n"
    "    gl_FragData[0] = ambientLight*fColor;" "\n"
    "}"                                         "\n"
    ;

static const char *ColorVertexShaderSource = 
    "#version 120"                              "\n"
    "uniform mat4 projectionMatrix;"            "\n"
    "uniform mat4 modelViewMatrix;"             "\n"
    "attribute vec2 vPosition;"                 "\n"
    "attribute vec4 vColor;"                    "\n"
    "varying vec4 fColor;"                      "\n"
    "void main() {"                             "\n"
    "    fColor = vColor;"                      "\n"
    "    gl_Position = projectionMatrix*modelViewMatrix*vec4(vPosition, 0.0, 1.0);\n"
    "}"
    ;

// Light gpu program.
static const char *LightFragmentShaderSource = 
    "#version 120"                              "\n"
    "uniform float lightRadius;"                "\n"
    "uniform vec4 lightColor;"                  "\n"
    "varying vec4 fColor;"                      "\n"
    "varying vec2 lightDir;"                    "\n"
    "void main() {"                             "\n"
    "    float dist = length(lightDir);"        "\n"
    "    float distFactor = dist/lightRadius;"
    "    float att = clamp(1.0 - distFactor, 0.0, 1.0);"
    "    gl_FragData[0] = att*lightColor*fColor;"   "\n"
    "}"                                             "\n"
    ;

static const char *LightVertexShaderSource = 
    "#version 120"                              "\n"
    "uniform mat4 projectionMatrix;"            "\n"
    "uniform mat4 modelViewMatrix;"             "\n"
    "uniform vec2 lightPosition;"               "\n"
    "attribute vec2 vPosition;"                 "\n"
    "attribute vec4 vColor;"                    "\n"
    "varying vec4 fColor;"                      "\n"
    "varying vec2 lightDir;"                    "\n"
    "void main() {"                             "\n"
    "    fColor = vColor;"                      "\n"
    "    lightDir = vPosition - lightPosition;"     "\n"
    "    gl_Position = projectionMatrix*modelViewMatrix*vec4(vPosition, 0.0, 1.0);\n"
    "}"
    ;

// Shadow volume program.
static const char *ShadowVolumeFragmentShaderSource = 
    "#version 120"                              "\n"
    "void main() {"                             "\n"
    "    gl_FragData[0] = vec4(1.0, 1.0, 0.0, 1.0);" "\n"
    "}"                                         "\n"
    ;

static const char *ShadowVolumeVertexShaderSource = 
    "#version 120"                              "\n"
    "uniform mat4 projectionMatrix;"            "\n"
    "uniform mat4 modelViewMatrix;"             "\n"
    "uniform vec2 lightPosition;"               "\n"
    "uniform float shadowExtrusion;"            "\n"
    "attribute vec2 vPosition;"                 "\n"
    "attribute vec2 vNormal;"                   "\n"
    "void main() {"                             "\n"
    "    vec2 lightDir = normalize(vPosition - lightPosition);"     "\n"
    "    float extrusion = (dot(lightDir, vNormal) > 0.0) ? shadowExtrusion : 0.0;" "\n"
    "    vec2 position = vPosition + extrusion*lightDir;"
    "    gl_Position = projectionMatrix*modelViewMatrix*vec4(position, 0.0, 1.0);\n"
    "}"
    ;

Renderer::Renderer(Game *game)
    : game(game)
{
    colorProgram = NULL;
    lightProgram = NULL;
    shadowVolumeProgram = NULL;

    lightPosition = Vector2::zero();
    lightColor = Color::white();
    shadowExtrusion = 1000.0f;
}

Renderer::~Renderer()
{
    delete colorProgram;
    delete lightProgram;
    delete shadowVolumeProgram;
}

GpuProgram *Renderer::createRenderProgram(const char *vertexShaderName, const char *vertexShaderSource,
    const char *fragmentShaderName, const char *fragmentShaderSource)
{
    // Create the vertex shader.
    Shader *vertexShader = new VertexShader();
    vertexShader->setName(vertexShaderName);
    vertexShader->setSource(vertexShaderSource);
    vertexShader->compile();

    // Create the fragment shader.
    Shader *fragmentShader = new FragmentShader();
    fragmentShader->setName(fragmentShaderName);
    fragmentShader->setSource(fragmentShaderSource);
    fragmentShader->compile();

    // Create the render program.
    GpuProgram *renderProgram = new GpuProgram();
    renderProgram->attach(vertexShader);
    renderProgram->attach(fragmentShader);
    renderProgram->bindAttribute("vPosition", VertexPositionBinding);
    renderProgram->bindAttribute("vColor", VertexColorBinding);
    renderProgram->bindAttribute("vNormal", VertexNormalBinding);
    renderProgram->link();

    return renderProgram;
}

bool Renderer::initialize()
{
    // Register myself as an event listener.
    game->getSystem().registerListener(this);

    // Create the render programs.
    colorProgram = createRenderProgram("ColorVS", ColorVertexShaderSource, "ColorFS", ColorFragmentShaderSource);
    lightProgram = createRenderProgram("LightVS", LightVertexShaderSource, "LightFS", LightFragmentShaderSource);
    shadowVolumeProgram = createRenderProgram("ShadowVolumeVS", ShadowVolumeVertexShaderSource, "ShadowVolumeFS", ShadowVolumeFragmentShaderSource);

    // Create the projection matrix.
    float aspect = (float)game->getSystem().getWidth()/(float)game->getSystem().getHeight();
    float h = 20.0f;
    float w = h*aspect;

    // Compute the view bounds.
    viewLeft = -w;
    viewRight = w;
    viewBottom = -h;
    viewTop = h;

    /// Create the projection matrix.
    projectionMatrix = Matrix4::ortho(viewLeft, viewRight, viewBottom, viewTop, -1.0f, 1.0f);

    // Create the model matrix
    modelViewMatrix = Matrix4::identity();

    return true;
}

Vector2 Renderer::windowToView(int x, int y) const
{
    int sw = game->getSystem().getWidth();
    int sh = game->getSystem().getHeight();
    y = sh - y - 1;
    return Vector2(x/float(sw)*(viewRight - viewLeft) + viewLeft, y/float(sh)*(viewTop - viewBottom) + viewBottom);
}

void Renderer::handleEvent(Event *event)
{
    switch(event->getType())
    {
    case Event::FrameDraw:
        renderFrame();
        break;
    default:
        // Do nothing.
        break;
    }
}

void Renderer::renderFrame()
{
    // Set the clearing and clear the framebuffer.
    glClearColor(0, 0, 0, 0);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Use blending and stencil testing.
    glEnable(GL_BLEND);
    glEnable(GL_STENCIL_TEST);

    // Prepare the frame.
    prepareFrame();

    // Setup shadow volumes.
    setupShadowVolumes();

    // Draw the background.
    drawBackground();

    // Draw the shadow casters.
    drawShadowCasters();

    // Draw not in shadow.
    drawNoShadow();
}

void Renderer::prepareFrame()
{
    // Clear the builders.
    backgroundBuilder.clear();
    shadowBuilder.clear();
    noShadowBuilder.clear();

    // Fill the builders.
    Sector *sector = game->getSector();
    if(sector)
    {
        sector->draw(this);
        modelViewMatrix = sector->getCameraTransform();
    }

    // Get the player.
    Player *player = game->getPlayer();
    if(player)
    {
        lightPosition = player->getPosition();
        lightColor = player->getLightColor().applyIntensity(player->getLightIntensity());
        lightRadius = player->getLightRadius();
    }
}

void Renderer::setupShadowVolumes()
{
    // Setup the stencil buffer.
    glStencilFunc(GL_ALWAYS, 0, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

    // Disable color writing.
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    // Use the shadow volume program.
    shadowVolumeProgram->use();
    sendUniforms(shadowVolumeProgram);

    // Draw the shadowed geometry.
    shadowBuilder.getMesh()->draw();

    // Make holes in the shadows.
    glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
    shadowVolumeProgram->uniform("shadowExtrusion", 0.0f);

    // Draw again without extrusion.
    shadowBuilder.getMesh()->draw();
}

void Renderer::setupAmbient()
{
    // Use replace the content blending.
    glBlendFunc(GL_ONE, GL_ZERO);

    // Don't modify the stencil buffer.
    glStencilFunc(GL_ALWAYS, 0, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    // Enable color writing.
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // Get the ambient light.
    ambientLight = Color::white();
    Sector *sector = game->getSector();
    if(sector)
        ambientLight = sector->getAmbientLight().applyIntensity(sector->getAmbientIntensity());

    // Use the program.
    colorProgram->use();
    sendUniforms(colorProgram);
}

void Renderer::setupReceivers()
{
    // Use additive blending.
    glBlendFunc(GL_ONE, GL_ONE);

    // Setup the stencil buffer.
    glStencilFunc(GL_EQUAL, 0, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    // Enable color writing.
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // Use the program.
    lightProgram->use();
    ambientLight = Color::black();
    sendUniforms(lightProgram);
}

void Renderer::drawBackground()
{
    if(backgroundBuilder.empty())
        return;

    // Add ambient.
    setupAmbient();
    backgroundBuilder.getMesh()->draw();

    // Add lighting.
    setupReceivers();
    backgroundBuilder.getMesh()->draw();
}

void Renderer::drawShadowCasters()
{
    if(shadowBuilder.empty())
        return;

    // Add ambient.
    setupAmbient();
    shadowBuilder.getMesh()->draw();

    // Add lighting.
    setupReceivers();
    shadowBuilder.getMesh()->draw();
}

void Renderer::drawNoShadow()
{
    if(noShadowBuilder.empty())
        return;

    // Here we can use alpha blending.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Don't modify the stencil buffer.
    glStencilFunc(GL_ALWAYS, 0, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    // Don't use the stencil buffer.
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // Use the program.
    colorProgram->use();
    ambientLight = Color::white();
    sendUniforms(colorProgram);

    // Draw the not in shadow geometry.
    noShadowBuilder.getMesh()->draw();
}

void Renderer::sendUniforms(GpuProgram *program)
{
    // Use the projection and the model view.
    program->uniform("projectionMatrix", projectionMatrix);
    sendModelView(program, modelViewMatrix);

    // Lighting.
    program->uniform("shadowExtrusion", shadowExtrusion);
    program->uniform("lightPosition", lightPosition);
    program->uniform("lightRadius", lightRadius);
    program->uniform("lightColor", lightColor);
    program->uniform("ambientLight", ambientLight);
}

void Renderer::sendModelView(GpuProgram *program, const Matrix4 &modelView)
{
    program->uniform("modelViewMatrix", modelView);
}

SimpleMeshBuilder *Renderer::getBackgroundBuilder()
{
    return &backgroundBuilder;
}

SimpleMeshBuilder *Renderer::getShadowBuilder()
{
    return &shadowBuilder;
}

SimpleMeshBuilder *Renderer::getNoShadowBuilder()
{
    return &noShadowBuilder;
}

}

