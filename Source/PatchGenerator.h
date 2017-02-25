/*
  ==============================================================================

    PatchGenerator.h
    Created: 19 Feb 2017 9:50:46pm
    Author:  tollie

  ==============================================================================
*/

#ifndef PATCHGENERATOR_H_INCLUDED
#define PATCHGENERATOR_H_INCLUDED

#include "RenderEngine.h"

//==============================================================================
class PatchGenerator
{

public:
    PatchGenerator (RenderEngine& engine) :
        generator (std::random_device()())
    {
        skeletonPatch = engine.getPatch();
        for (auto& parameter : skeletonPatch)
            parameter.second = 0.0f;
    }

    std::pair<int, float> getRandomParameter (int index);

    PluginPatch getRandomPatch();

private:
    PluginPatch skeletonPatch;
    
    std::mt19937_64 generator;
};

#endif  // PATCHGENERATOR_H_INCLUDED
