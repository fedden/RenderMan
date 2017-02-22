/*
  ==============================================================================

    PatchGenerator.cpp
    Created: 19 Feb 2017 9:50:47pm
    Author:  tollie

  ==============================================================================
*/

#include "PatchGenerator.h"

//==============================================================================
std::pair<int, float> PatchGenerator::getRandomParameter (int index)
{
    std::uniform_real_distribution<float> distribution (0, 1);
    float randomValue = distribution (generator);
    return std::make_pair (index, randomValue);
}

//==============================================================================
PluginPatch PatchGenerator::getRandomPatch()
{
    PluginPatch randomPatch = skeletonPatch;
    std::uniform_real_distribution<float> distribution (0, 1);
    for (auto& parameter : randomPatch)
        parameter.second = distribution (generator);
    return randomPatch;
}
