#ifndef SWBF2_REDENGINEFL_GRAPHICS_REDMATERIAL_H
#define SWBF2_REDENGINEFL_GRAPHICS_REDMATERIAL_H

#include <stddef.h>

#include "RedTextureTransform.h"

struct RedMaterialAnimationData;

class RedColor;

class RedColorValue
{
public:

    RedColorValue() {}
    RedColorValue(float fR, float fG, float fB, float fA) : r(fR), g(fG), b(fB), a(fA) {}
    RedColorValue(const RedColorValue& c) : r(c.r), g(c.g), b(c.b), a(c.a) {}
    RedColorValue(const RedColor& color);

    float r;
    float g;
    float b;
    float a;

    static RedColorValue WHITE;

    static RedColorValue BLACK;
};
 
 
 
 
 

class RedLight;

struct RedMaterialAnimationData
{
    RedColorValue diffuseColor;
    unsigned int attachedLightName;
    RedLight* attachedLight;
    unsigned char scrollRateU;
    unsigned char scrollRateV;
    unsigned char minIntensity;
    unsigned char blinkRate;
    unsigned char numRows;
    unsigned char numCols;
    unsigned char frameRate;
};
 

struct RedMaterial
{
    unsigned int diffuseMapHash;
    unsigned int detailMapHash;
    unsigned int bumpMapHash;
    unsigned int envMapHash;
    unsigned int materialFlags;
    RedColorValue diffuseColor;
    RedColorValue specularColor;
    float specularPower;
    RedTextureTransform diffuseTexTransform;
    RedTextureTransform detailTexTransform;
    RedMaterialAnimationData* animationData;
};
 
 
 
 
 
 
 
 

#endif
