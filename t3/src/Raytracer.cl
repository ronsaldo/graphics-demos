// Some pieces of code are extracted from a tutorial series in Flipcode available in:
// http://www.flipcode.com/archives/Raytracing_Topics_Techniques-Part_1_Introduction.shtml

#include "Geometry.hpp"

__constant const float PositionDisp = 0.001f;
__constant const float ShadowMin = 0.1f;

template<typename T, int N=7>
class BoundedStack
{
public:
    typedef T Frame;

    BoundedStack()
        : size(0) {}
    ~BoundedStack() {}

    bool empty() const
    {
        return size == 0;
    }

    bool full() const
    {
        return size == N;
    }

    void push(const Frame &frame)
    {
        frames[size++] = frame;
    }

    Frame &back()
    {
        return frames[size-1];
    }

    void pop()
    {
        --size;
    }

private:
    size_t size;
    Frame frames[N];
};

inline Vector3 reflect(Vector3 I, Vector3 N)
{
    return I - 2.0f * dot(I, N)*N;
}

/**
 * Gpu raytracer
 */
class GpuRaytracer
{
public:
    GpuRaytracer(const __global unsigned char *sceneData,
                 const __global unsigned int *imageDescs,
                 const __global float4 *images)
        : scene(sceneData), imageDescs(imageDescs), images(images) {}

    float sampleShadow(Vector3 position,  const __global Shape *lightShape);
    Color raytrace(const Ray &primaryRay);

private:
    // Shading
    void setShadingShape(const __global Shape *shape, const Ray &ray, float amount);
    Color addLightContribution(const __global Shape *lightShape);
    Color computeShading();

    // Texture images.
    Color sampleImageNormalized(int id, Vector2 texCoord);

    // Sky.
    Color computeSkyColor(const Vector3 &direction);

    // Texturing/Materials.
    Color getTextureColor(int textureId);
    Vector3 getTextureNormal(int textureId);

    // Scene
    SceneAccess scene;
    const __global unsigned int *imageDescs;
    const __global float4 *images;

    // Shading data.
    const __global Shape* currentShape;
    const __global Material* currentMaterial;
    Vector3 P; // Position vector
    Vector3 SN; // Surface normal vector.
    Vector3 N; // Normal vector.
    Vector3 V; // View vector.= ray.direction;
    Color emissionColor;
    Color diffuseColor;
    Color specularColor;
    float shininess;

    int lastTextureId;
    Color lastTextureColor;
};

float GpuRaytracer::sampleShadow(Vector3 position,  const __global Shape *lightShape)
{
    float res = 0.0f;
    Vector3 lightDir = normalize(Shape::lightDir(lightShape, position));
    if(!scene.blockedLine(Ray(position, lightDir), lightShape))
        res = 1.0f;

    return res;
}

Color GpuRaytracer::addLightContribution(const __global Shape *lightShape)
{
    const __global Material *lightMaterial = scene.getMaterial(lightShape->materialId);
    if(!lightMaterial->light)
        return color_zero();

    // Compute the shadow
    float shadow = sampleShadow(P, lightShape);
    if(shadow < ShadowMin)
        return color_zero();
    
    Color lightColor = lightMaterial->emission*getTextureColor(lightMaterial->emissionTexture);

    // Compute the diffuse lighting.
    Color res = color_zero();
    Vector3 L = normalize(Shape::lightDir(lightShape, P));
    float NdotL = dot(L, N);
    if(NdotL > 0.0f)
    {
        res += shadow*NdotL*diffuseColor;

        // Try to compute specular lighting
        Vector3 R = reflect(L, N);
        float VdotR = dot(V, R);
        if(VdotR > 0.0f)
        {
            float spec = pow(VdotR, shininess);
            res += shadow*spec*specularColor;
        }
    }
    
    return res*lightColor;
}

Color GpuRaytracer::computeShading()
{
    Color color = emissionColor;

    // Add the lights contributions.
    for(int i = 0; i < scene.getShapeCount(); ++i)
        color += addLightContribution(scene.getShape(i));

    return color;
}

Color GpuRaytracer::getTextureColor(int textureId)
{
    // Avoid computing the texture.
    if(textureId == lastTextureId)
        return lastTextureColor;

    if(textureId >= 0)
        lastTextureColor = scene.getTexture(textureId)->computeColor(P);
    else if(textureId == -2)
        lastTextureColor = color_white();
    else
        lastTextureColor = color_black();
    return lastTextureColor;
}

Vector3 GpuRaytracer::getTextureNormal(int textureId)
{
    if(textureId < 0)
        return SN;
    return scene.getTexture(textureId)->computeNormal(P, SN);
}

void GpuRaytracer::setShadingShape(const __global Shape *shape, const Ray &ray, float rayAmount)
{
    // Compute the shading vectors.
    currentShape = shape;
    P = ray.at(rayAmount);
    SN = Shape::normalAt(shape, P);
    V = ray.direction;
    currentMaterial = scene.getMaterial(shape->materialId);

    // Get the material data.
    lastTextureId = -3;
    emissionColor = currentMaterial->emission*getTextureColor(currentMaterial->emissionTexture);
    diffuseColor = currentMaterial->diffuse*getTextureColor(currentMaterial->diffuseTexture);
    specularColor = currentMaterial->specular*getTextureColor(currentMaterial->specularTexture);
    shininess = currentMaterial->shininess;

    // Apply normal mapping.
    N = getTextureNormal(currentMaterial->normalTexture);
    P += SN*PositionDisp;
}

Color GpuRaytracer::sampleImageNormalized(int id, Vector2 texCoord)
{
    int offset = imageDescs[id*3];
    int width = imageDescs[id*3 + 1];
    int height = imageDescs[id*3 + 2];
    int x = ((int)(texCoord.x*width + 0.5f)) % width;
    int y = ((int)(texCoord.y*height + 0.5f)) % height;
    return images[offset + width*y + x];
}

Color GpuRaytracer::computeSkyColor(const Vector3 &direction)
{
    float phi = atan2(direction.z, direction.x) + M_PI_F;
    float theta = acos(direction.y);
    return sampleImageNormalized(0, make_vector2(phi*M_1_PI_F*0.5, theta*M_1_PI_F));
}

enum RaytraceState
{
    RS_Initial = 0,
    RS_ReflectionReturn,
    RS_RefractionReturn,
    RS_Return,
};

struct RaytraceFrame
{
    RaytraceFrame() {}
    RaytraceFrame(const Ray &ray, const Color &color, RaytraceState returnState = RS_Return, float rindex=1.0f)
        : ray(ray), color(color), returnState(returnState), refractionIndex(rindex) {}
    ~RaytraceFrame() {}

    const __global Shape *shape;
    Ray ray;
    Color color;
    Color reflectionColor;
    Color refractionColor;
    float refractionIndex;
    RaytraceState returnState;
};

Color GpuRaytracer::raytrace(const Ray &primaryRay)
{
    BoundedStack<RaytraceFrame> stack;
    Color color = color_zero();
    RaytraceState state = RS_Initial;

    // Start with the primary ray.
    stack.push(RaytraceFrame(primaryRay, color_zero()));
    while(!stack.empty())
    {
        // References to the stack frame.
        Ray &ray = stack.back().ray;
        const __global Shape *&shape = stack.back().shape;
        Color &currentColor = stack.back().color;
        Color &reflectionColor = stack.back().reflectionColor;
        Color &refractionColor = stack.back().refractionColor;
        float &currentRIndex = stack.back().refractionIndex;

        // Act according to the state.
        switch(state)
        {
        case RS_Initial:
            {
                // Cast the ray.
                float rayAmount;                

                // Check the result.                
                if(scene.firstIntersection(ray, &rayAmount, &shape))
                {
                    // Set the shading shape data.
                    setShadingShape(shape, ray, rayAmount);
                    color = currentColor = computeShading();

                    // Finish when the stack is full
                    if(!stack.full())
                    {
                        // Try to add reflection.
                        if(currentMaterial->reflection > 0.0f)
                        {
                            // Compute the reflected vector
                            Vector3 R = reflect(ray.direction, N);

                            // Cast the reflection.
                            reflectionColor = currentMaterial->reflection*this->specularColor;
                            Vector3 pos = P;
                            stack.push(RaytraceFrame(Ray(pos, R), color_zero(), RS_ReflectionReturn));
                            state = RS_Initial;
                            continue;
                        }
                        else if(currentMaterial->refraction > 0.0f)
                        {
                            float rindex = currentMaterial->refractionIndex;
	                        float n = currentRIndex / rindex;

                            Vector3 RN = N*Shape::computeSideFactor(shape, ray.start);
	                        float cosI = -dot(ray.direction, RN);
	                        float cosT2 = 1.0f - n * n * (1.0f - cosI * cosI);
	                        if (cosT2 > 0.0f)
	                        {
                                // Compute the refracted vector
                                Vector3 T = (n * ray.direction) + (n * cosI - sqrt(cosT2)) * RN;

                                // Cast the refraction.
                                refractionColor = currentMaterial->refraction*this->specularColor;
                                Vector3 pos = ray.at(rayAmount) + T*PositionDisp;
                                stack.push(RaytraceFrame(Ray(pos, T), color_zero(), RS_RefractionReturn, rindex));
                                state = RS_Initial;
                                continue;
                            }
                        }
                    }
                }
                else
                {
                    color = currentColor = computeSkyColor(ray.direction);
                }
            }
            break;
        case RS_ReflectionReturn:
            currentColor += reflectionColor*color;
            color = currentColor;
            break;
        case RS_RefractionReturn:
            currentColor += refractionColor*color;
            color = currentColor;
            break;
        default:
            color = currentColor;
            break;
        }

        // Pop the stack.
        state = stack.back().returnState;
        stack.pop();
    }

    return color;
}

inline float exposeChannel(float c, float l)
{
    return c/(l+1);
}

inline Color toneMap(Color c)
{
    float l = 0.2126f*c.x + 0.7152f*c.y + 0.0722f*c.z;
    return make_color(exposeChannel(c.x, l), exposeChannel(c.y, l), exposeChannel(c.z, l), exposeChannel(c.w, l));
}

__kernel void castPrimaryRays(const __global unsigned char *sceneData,
                              float4 origin,
                              float4 screenPlaneP1, float4 screenPlaneP2,
                              float4 screenPlaneP3, float4 screenPlaneP4,
                              const __global unsigned int *imageDescs,
                              const __global float4 *images,
                              __write_only image2d_t colorBuffer)
{
    // Compute the buffer coordinates.
    size_t x = get_global_id(0);
    size_t y = get_global_id(1);
    int2 coord = (int2)(x, y);

    // Compute the image coordinate.
    int2 dims = get_image_dim(colorBuffer);
    float3 screenU = screenPlaneP2.xyz - screenPlaneP1.xyz;
    float3 screenV = screenPlaneP4.xyz - screenPlaneP1.xyz;
    float3 screenCoord = screenPlaneP1.xyz + screenU*((float)x/(float)dims.x) + screenV*((float)y/(float)dims.y);

    // Create the ray.
    float3 rayDir = normalize(screenCoord - origin.xyz);
    Ray ray(origin.xyz, rayDir);

    // Perform raytracing.
    GpuRaytracer raytracer(sceneData, imageDescs, images);
    Color color = raytracer.raytrace(ray);

    // Emit a color
    write_imagef(colorBuffer, coord, toneMap(color));
}

//------------------------------------------------------------------------------
// Sky
//

/// Constants taken from http://www.gamedev.net/topic/584256-atmospheric-scattering-and-dark-sky/

__constant const int ScatteringSamples = 20;
__constant const float ScatteringInvSamples = 1.0f/20.0f;

__constant const float EarthRadius = 6360e3f;
__constant const float AtmosphereRadius = 6420e3f;

__constant const float RayleighScaleHeight = 7994.0f;
__constant const float MieScaleHeight = 1200.0f;
__constant const Vector4 RayleighConstants = constant_vector4(5.5e-6f, 13.0e-6f, 22.4e-6f, 0.0f);
__constant const Vector4 MieConstants = constant_vector4(21e-6f, 21e-6f, 21e-6f, 0.0f);
__constant const float ScatterG = 0.75f;

float rayleighScatterPhase(float c)
{
    return (1 + c*c)*0.75f;
}

float mieScatterPhase(float c, float g)
{
    float gg = g*g;
    float A = 1.5f*(1.0f  - gg)/(2.0f + gg);
    float B = (1.0f + c*c)/pow(1.0f + gg - 2.0f*g*c, 1.5f);
    return A*B;
}

/**
 * Sky scattering computation.
 * Code adapted from: http://www.scratchapixel.com/lessons/3d-advanced-lessons/simulating-the-colors-of-the-sky/atmospheric-scattering/
 */
Color inScattering(Vector3 camera, Vector3 b, Vector3 sunDirection, Color sunColor)
{
    // Phase functions.
    Vector3 a = camera;
    float cosTheta = dot(normalize(b - camera), sunDirection);
    float phaseR = rayleighScatterPhase(cosTheta);
    float phaseM = mieScatterPhase(cosTheta, ScatterG);

    // Loop state.
    Vector3 delta = (b - a)*ScatteringInvSamples;
    float sampleLength = length(delta);
    float opticalDepthR = 0.0;
    float opticalDepthM = 0.0;
    Color sumR = color_zero();
    Color sumM = color_zero();

    // Used for light direction.
    SphereShape atmosSphere(vector3_zero(), AtmosphereRadius);
   
    // Integral evaluation
    Vector3 samplePoint = a;
    for(int i = 0; i < ScatteringSamples; ++i)
    {
        samplePoint += 0.5f*delta;

        // Optical depth
        float height = length(samplePoint) - EarthRadius;
        float hr = exp(-height/RayleighScaleHeight)*sampleLength;
        float hm = exp(-height/MieScaleHeight)*sampleLength;
        opticalDepthR += hr;
        opticalDepthM += hm;

        // Light optical depth.
        Ray ray = Ray(samplePoint, sunDirection.xyz);
        Vector3 la = samplePoint;
        Vector3 lb = ray.at(atmosSphere.intersects(ray));
        Vector3 ldelta = (lb - la)*ScatteringInvSamples;
        float opticalDepthLightR = 0.0f, opticalDepthLightM = 0.0f;
        float lightSampleLength = length(ldelta);
        int j = 0;
        Vector3 lightSamplePoint = la;
        for(j = 0; j < ScatteringSamples; ++j)
        {
            lightSamplePoint += 0.5f*ldelta;
            float lightHeight = length(lightSamplePoint) - EarthRadius;
            if(lightHeight < 0) break;
            opticalDepthLightR += exp(-lightHeight/RayleighScaleHeight)*lightSampleLength;
            opticalDepthLightM += exp(-lightHeight/MieScaleHeight)*lightSampleLength;
        }

        if(j == ScatteringSamples)
        {
            Color tau = RayleighConstants*(opticalDepthR + opticalDepthLightR) + MieConstants* 1.1 *(opticalDepthM + opticalDepthLightM);
            Color att = make_color(exp(-tau.x), exp(-tau.y), exp(-tau.z), 0.0f);
            sumR += hr*tau;
            sumM += hm*att;
        }
    }

    return 5.0f*sunColor*(sumR*phaseR*RayleighConstants + sumM*phaseM*MieConstants);
}

Vector3 sphericalCoordinates(float radius, float phi, float theta)
{
    // Compute the actual position.
    return make_vector3(radius*sin(theta)*cos(phi), radius*cos(theta), radius*sin(theta)*sin(phi));
}

__kernel void createDaySky(int offset, int width, int height, __global float4 *image,
                            float skyRadius, Color sunColor, Vector4 sunDirection)
{
    // Compute the buffer coordinates.
    size_t xc = get_global_id(0);
    size_t yc = get_global_id(1);

    // Compute the angle.
    float phi = 2.0f*M_PI_F*(xc+0.5f)/(float)width;
    float theta = M_PI_F*(yc+0.5f)/(float)height;
    Vector3 direction = sphericalCoordinates(1.0f, phi, theta);

    // Compute a end position.
    Vector3 start = EarthRadius*make_vector3(0, 1, 0);

    // Correct the end position.
    Ray ray(start, direction);
    SphereShape sphere(vector3_zero(), AtmosphereRadius);
    Vector3 end = ray.at(sphere.intersects(ray));

    // Emit the result.
    image[offset + yc*width + xc] = inScattering(start, end, sunDirection.xyz, sunColor);
}

__kernel void createNightSky(int offset, int width, int height, __global float4 *image,
                            float skyRadius, float starScale, float starThreshold)
{
    // Compute the buffer coordinates.
    size_t xc = get_global_id(0);
    size_t yc = get_global_id(1);

    // Compute the angle.
    float phi = 2.0f*M_PI_F*(xc+0.5f)/(float)width;
    float theta = M_PI_F*(yc+0.5f)/(float)height;

    // Compute the actual position.
    float x = skyRadius*sin(theta)*cos(phi);
    float y = skyRadius*cos(theta);
    float z = skyRadius*sin(theta)*sin(phi);

    // Compute the stars.
    float star = smoothstep(starThreshold, 1.0f, simplex_noise3D(x*starScale, y*starScale, z*starScale));

    // Emit a color
    image[offset + yc*width + xc] = color_white()*star;
}

