/*
  ==============================================================================

    source.cpp
    Created: 18 Feb 2017 6:37:01pm
    Author:  tollie

  ==============================================================================
*/

#include "PatchGenerator.h"
#include <boost/python.hpp>

// Could also easily be namespace crap.
namespace wrap
{
    //==========================================================================
    // Converts a C++ vector to a Python list. All following functions
    // are essentially cheap ripoffs from this one.
    // https://gist.github.com/octavifs/5362272
    template <class T>
    boost::python::list vectorToList (std::vector<T> vector)
    {
    	typename std::vector<T>::iterator iter;
    	boost::python::list list;
    	for (iter = vector.begin(); iter != vector.end(); ++iter)
        {
    		list.append(*iter);
    	}
    	return list;
    }

    //==========================================================================
    // Yeah this is lazy. I know.
    template <class T>
    boost::python::list arrayToList (std::array<T, 13> array)
    {
    	typename std::array<T, 13>::iterator iter;
    	boost::python::list list;
    	for (iter = array.begin(); iter != array.end(); ++iter)
        {
    		list.append(*iter);
    	}
    	return list;
    }

    //==========================================================================
    // Converts a std::pair which is used as a parameter in C++
    // into a tuple with the respective types int and float for
    // use in Python.
    boost::python::tuple parameterToTuple (std::pair<int, float> parameter)
    {
        boost::python::tuple parameterTuple;
        parameterTuple = boost::python::make_tuple (parameter.first,
                                                    parameter.second);
        return parameterTuple;
    }

    //==========================================================================
    // Converts a PluginPatch ( std::vector <std::pair <int, float>> )
    // to a Python list.
    boost::python::list pluginPatchToListOfTuples (PluginPatch parameters)
    {
    	std::vector<std::pair<int, float>>::iterator iter;
    	boost::python::list list;
    	for (iter = parameters.begin(); iter != parameters.end(); ++iter)
        {
            auto tup = parameterToTuple (*iter);
    		list.append(tup);
    	}
    	return list;
    }

    //==========================================================================
    // std::vector <std::array <double, 13>> MFCCFeatures;
    boost::python::list mfccFramesToListOfLists (MFCCFeatures frames)
    {
        boost::python::list list;
        std::vector <std::array <double, 13>>::iterator iter;
        for (iter = frames.begin(); iter != frames.end(); ++iter)
        {
            auto l = arrayToList (*iter);
    		list.append(l);
    	}
    	return list;
    }

    //==========================================================================
    PluginPatch listOfTuplesToPluginPatch (boost::python::list listOfTuples)
    {
        PluginPatch patch;
        const int size = boost::python::len (listOfTuples);
        patch.reserve (size);
        std::pair <int, float> parameter;
        for (int i = 0; i < size; ++i)
        {
            boost::python::tuple tup;
            tup = boost::python::extract<boost::python::tuple> (listOfTuples[i]);
            int   index = int (boost::python::extract<float> (tup[0]));
            float value = float (boost::python::extract<float> (tup[1]));
            parameter = std::make_pair (index, value);
            patch.push_back (parameter);
        }
        return patch;
    }

    //==========================================================================
    class RenderEngineWrapper : public RenderEngine
    {
    public:
        RenderEngineWrapper (int sr, int bs, int ffts) :
            RenderEngine (sr, bs, ffts)
        { }

        void wrapperSetPatch (boost::python::list listOfTuples)
        {
            PluginPatch patch = listOfTuplesToPluginPatch (listOfTuples);
            RenderEngine::setPatch(patch);
        }

        boost::python::list wrapperGetPatch()
        {
            return pluginPatchToListOfTuples (RenderEngine::getPatch());
        }

        void wrapperRenderPatch (int    midiNote,
                                 int    midiVelocity,
                                 double noteLength,
                                 double renderLength)
        {
            if (midiNote > 255) midiNote = 255;
            if (midiNote < 0) midiNote = 0;
            if (midiVelocity > 255) midiVelocity = 255;
            if (midiVelocity < 0) midiVelocity = 0;
            RenderEngine::renderPatch(midiNote,
                                      midiVelocity,
                                      noteLength,
                                      renderLength);
        }

        boost::python::list wrapperGetMFCCFrames()
        {
            return mfccFramesToListOfLists (RenderEngine::getMFCCFrames());
        }

        int wrapperGetPluginParameterSize()
        {
            return int (RenderEngine::getPluginParameterSize());
        }

        std::string wrapperGetPluginParametersDescription()
        {
            return RenderEngine::getPluginParametersDescription().toStdString();
        }

        boost::python::list wrapperGetAudioFrames()
        {
            return vectorToList (RenderEngine::getAudioFrames());
        }

        boost::python::list wrapperGetRMSFrames()
        {
            return vectorToList (RenderEngine::getRMSFrames());
        }
    };

    //==========================================================================
    class PatchGeneratorWrapper : public PatchGenerator
    {
    public:
        PatchGeneratorWrapper (RenderEngine& engine) :
            PatchGenerator (engine)
        { }

        boost::python::tuple wrapperGetRandomParameter (int index)
        {
            return parameterToTuple (PatchGenerator::getRandomParameter (index));
        }

        boost::python::list wrapperGetRandomPatch()
        {
            return pluginPatchToListOfTuples (PatchGenerator::getRandomPatch());
        }
    };
}

//==============================================================================
BOOST_PYTHON_MODULE(librenderman)
{
    using namespace boost::python;
    using namespace wrap;

    class_<RenderEngineWrapper>("RenderEngine", init<int, int, int>())
    .def("load_plugin", &RenderEngineWrapper::loadPlugin)
    .def("set_patch", &RenderEngineWrapper::wrapperSetPatch)
    .def("get_patch", &RenderEngineWrapper::wrapperGetPatch)
    .def("render_patch", &RenderEngineWrapper::wrapperRenderPatch)
    .def("get_mfcc_frames", &RenderEngineWrapper::wrapperGetMFCCFrames)
    .def("get_plugin_parameter_size", &RenderEngineWrapper::wrapperGetPluginParameterSize)
    .def("get_plugin_parameters_description", &RenderEngineWrapper::wrapperGetPluginParametersDescription)
    .def("override_plugin_parameter", &RenderEngineWrapper::overridePluginParameter)
    .def("remove_overriden_plugin_parameter", &RenderEngineWrapper::removeOverridenParameter)
    .def("get_audio_frames", &RenderEngineWrapper::wrapperGetAudioFrames)
    .def("get_rms_frames", &RenderEngineWrapper::wrapperGetRMSFrames)
    .def("write_to_wav", &RenderEngineWrapper::writeToWav);

    class_<PatchGeneratorWrapper>("PatchGenerator", init<RenderEngineWrapper&>())
    .def("get_random_parameter", &PatchGeneratorWrapper::wrapperGetRandomParameter)
    .def("get_random_patch", &PatchGeneratorWrapper::wrapperGetRandomPatch);
}
