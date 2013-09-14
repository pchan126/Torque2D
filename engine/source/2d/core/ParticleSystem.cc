//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "2d/core/particleSystem.h"

//------------------------------------------------------------------------------

ParticleSystem* ParticleSystem::Instance = nullptr;

//------------------------------------------------------------------------------

void ParticleSystem::Init( void )
{
    // Create the particle system.
    Instance = new ParticleSystem();
}

//------------------------------------------------------------------------------

void ParticleSystem::destroy( void )
{
    // Delete the particle system.
    delete Instance;
    Instance = nullptr;
}

//------------------------------------------------------------------------------

ParticleSystem::ParticleSystem() :
                    mParticlePoolBlockSize(512)
{
    // Reset the active particle count.
    mActiveParticleCount = 0;
}

//------------------------------------------------------------------------------

ParticleSystem::~ParticleSystem()
{
   for (auto particle:mParticlePool)
      delete particle;
}

//------------------------------------------------------------------------------

ParticleSystem::ParticleNode* ParticleSystem::createParticle( void )
{
    // Have we got any free particle nodes?
    if ( mpFreeParticleNodes.front() == nullptr )
    {
        auto oldSize = mParticlePool.size();
        for (int i = 0; i < mParticlePoolBlockSize; i++)
        {
            ParticleNode* temp = new ParticleNode();
            mParticlePool.push_back(temp);
        }

        for (auto itr = mParticlePool.begin()+oldSize; itr != mParticlePool.end(); itr++)
        {
            mpFreeParticleNodes.push_back(*itr);
        }
    }

    // Fetch a free node,
    ParticleNode* pFreeParticleNode = mpFreeParticleNodes.front();

    // Set the new free node reference.
    mpFreeParticleNodes.pop_front();

    // Increase the active particle count.
    mActiveParticleCount++;

    return pFreeParticleNode;
}

//------------------------------------------------------------------------------

void ParticleSystem::freeParticle( ParticleNode* pParticleNode )
{
    // Reset the particle.
    pParticleNode->resetState();

    mpFreeParticleNodes.push_back(pParticleNode);

    // Decrease the active particle count.
    mActiveParticleCount--;
}

