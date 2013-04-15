//
//  IUniform.h
//  Torque2D
//
//  Created by Paul L Jan on 2013-04-11.
//
//

#ifndef Torque2D_IUniform_h
#define Torque2D_IUniform_h

#include "./gfxOpengGLStateBlock.h"

class IEngineUniform
{
public:
    
    virtual ~IEngineUniform() {}
    virtual void set(const GFXOpenGLStateBlock& state);
};

class IEngineUniformFactory
{
public:
    virtual ~IEngineUniformFactory() { }
    virutal IEngineUniform *Create(Uniform *uniform) = 0;
};

#endif
