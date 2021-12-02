/*
  ==============================================================================

    RenderEngine.cpp
    Created: 19 Feb 2017 9:47:15pm
    Author:  tollie

  ==============================================================================
*/

#include "RenderEngine.h"

namespace p = boost::python;

//==============================================================================

void RenderEngine::fillAvailablePluginsInfo(const std::string& path,
                                            AudioPluginFormatManager& pluginFormatManager,
                                            OwnedArray<PluginDescription>& pluginDescriptions,
                                            KnownPluginList& pluginList
                                            ) {

    pluginFormatManager.addDefaultFormats();

    for (int i = pluginFormatManager.getNumFormats(); --i >= 0;)
    {
        pluginList.scanAndAddFile (String (path),
                                   true,
                                   pluginDescriptions,
                                   *pluginFormatManager.getFormat(i));
    }
}


std::string RenderEngine::getAvailablePluginsXml(const std::string& path) {
    AudioPluginFormatManager pluginFormatManager;
    OwnedArray<PluginDescription> pluginDescriptions;
    KnownPluginList pluginList;
    fillAvailablePluginsInfo(path, pluginFormatManager, pluginDescriptions, pluginList);
    
    XmlElement* ptr = pluginList.createXml();
    String serialized = ptr->createDocument("");
    delete ptr;
    
    return serialized.toStdString();
}


bool RenderEngine::loadPlugin (const std::string& path, int index)
{
    AudioPluginFormatManager pluginFormatManager;
    OwnedArray<PluginDescription> pluginDescriptions;
    KnownPluginList pluginList;
    fillAvailablePluginsInfo(path, pluginFormatManager, pluginDescriptions, pluginList);

    // If there is a problem here first check the preprocessor definitions
    // in the projucer are sensible - is it set up to scan for plugin's?
    jassert (pluginDescriptions.size() > 0);

    if (index >= pluginDescriptions.size()) {
        std::cout << "RenderEngine::loadPlugin error: plugin index " << index
        << "provided, but only " << pluginDescriptions.size()
        << "plugins detected" << std::endl;
        return false;
    }
    
    String errorMessage;

    if (plugin != nullptr) {
        plugin->releaseResources();
        delete plugin;
    }

    plugin = pluginFormatManager.createPluginInstance (*pluginDescriptions[index],
                                                       sampleRate,
                                                       bufferSize,
                                                       errorMessage);
    if (plugin != nullptr)
    {
        // Success so set up plugin, then set up features and get all available
        // parameters from this given plugin.
        plugin->prepareToPlay (sampleRate, bufferSize);
        plugin->setNonRealtime (true);

        mfcc.setup (512, 42, 13, 20, int (sampleRate / 2), sampleRate);

        // Resize the pluginParameters patch type to fit this plugin and init
        // all the values to 0.0f!
        fillAvailablePluginParameters (pluginParameters);

        return true;
    }

    std::cout << "RenderEngine::loadPlugin error: "
    << errorMessage.toStdString()
    << std::endl;

    return false;
}

//==============================================================================
void RenderEngine::renderPatch (const uint8  midiNote,
                                const uint8  midiVelocity,
                                const double noteLength,
                                const double renderLength)
{
    // Get the overriden patch and set the vst parameters with it.
    PluginPatch overridenPatch = getPatch();
    for (const auto& parameter : overridenPatch)
        plugin->setParameter (parameter.first, parameter.second);

    // Get the note on midiBuffer.
    MidiMessage onMessage = MidiMessage::noteOn (1,
                                                 midiNote,
                                                 midiVelocity);
    MidiBuffer midiNoteBuffer;
    midiNoteBuffer.addEvent (onMessage, 0);

    // Setup fft here so it is destroyed when rendering is finished and
    // the stack unwinds so it doesn't share frames with a new patch.
    maxiFFT fft;
    fft.setup (fftSize, fftSize / 2, fftSize / 4);

    // Data structure to hold multi-channel audio data.
    AudioSampleBuffer audioBuffer (plugin->getTotalNumOutputChannels(),
                                   bufferSize);

    int numberOfBuffers = int (std::ceil (renderLength * sampleRate / bufferSize));

    // Clear and reserve memory for the audio storage!
    processedMonoAudioPreview.clear();
    processedMonoAudioPreview.reserve (numberOfBuffers * bufferSize);

    // Number of FFT, MFCC and RMS frames.
    int numberOfFFT = int (std::ceil (renderLength * sampleRate / fftSize)) * 4;
    rmsFrames.clear();
    rmsFrames.reserve (numberOfFFT);
    currentRmsFrame = 0.0;
    mfccFeatures.clear();
    mfccFeatures.reserve (numberOfFFT);

    plugin->prepareToPlay (sampleRate, bufferSize);

    for (int i = 0; i < numberOfBuffers; ++i)
    {
        // Only trigger one note on during first audio buffer
        if (i > 0)
            midiNoteBuffer.clear();
        
        // Trigger note off if in the correct audio buffer.
        ifTimeSetNoteOff (noteLength,
                          sampleRate,
                          bufferSize,
                          1,
                          midiNote,
                          midiVelocity,
                          i,
                          midiNoteBuffer);

        // Turn Midi to audio via the vst.
        plugin->processBlock (audioBuffer, midiNoteBuffer);

        // Get audio features and fill the datastructure.
        fillAudioFeatures (audioBuffer, fft);
    }
}

//=============================================================================
void RenderEngine::renderWav(boost::python::object wav)
{
    long sampleCount = p::len(wav);
    // Get the overriden patch and set the vst parameters with it.
    PluginPatch overridenPatch = getPatch();
    for (const auto& parameter : overridenPatch)
        plugin->setParameter (parameter.first, parameter.second);

    // empty MIDI note buffer
    MidiBuffer midiNoteBuffer;

    // Setup fft here so it is destroyed when rendering is finished and
    // the stack unwinds so it doesn't share frames with a new patch.
    maxiFFT fft;
    fft.setup (fftSize, fftSize / 2, fftSize / 4);

    // Data structure to hold multi-channel audio data.
    AudioSampleBuffer audioBuffer (plugin->getTotalNumOutputChannels(),
                                   bufferSize);

    int numberOfBuffers = int (std::ceil (sampleCount / bufferSize));

    // Clear and reserve memory for the audio storage!
    processedMonoAudioPreview.clear();
    processedMonoAudioPreview.reserve (numberOfBuffers * bufferSize);

    // Number of FFT, MFCC and RMS frames.
    int numberOfFFT = int (std::ceil (sampleCount / fftSize)) * 4;
    rmsFrames.clear();
    rmsFrames.reserve (numberOfFFT);
    currentRmsFrame = 0.0;
    mfccFeatures.clear();
    mfccFeatures.reserve (numberOfFFT);
    
    //wav = wav.astype(np::dtype::get_builtin<double>());
    

    plugin->prepareToPlay (sampleRate, bufferSize);

    for (int i = 0; i < numberOfBuffers; ++i)
    {
        // TODO: SLOW
        auto writePointers = audioBuffer.getArrayOfWritePointers();
        int numChannels = audioBuffer.getNumChannels();
        int remaining = (i < numberOfBuffers - 1) ? bufferSize : (sampleCount % bufferSize);
        // if buffer size divides sample count, last buffer is full-size
        remaining = (remaining == 0) ? bufferSize : remaining;
        
        for(int j = 0; j < remaining; j++) {
            double value = p::extract<double>(wav[j + i*bufferSize]);
            for (int k = 0; k < numChannels; k++) {
                writePointers[k][j] = value;
            }
        }
        
        // Turn Midi to audio via the vst.
        plugin->processBlock (audioBuffer, midiNoteBuffer);

        // Get audio features and fill the datastructure.
        fillAudioFeatures (audioBuffer, fft);
    }

}

//=============================================================================
void RenderEngine::fillAudioFeatures (const AudioSampleBuffer& data,
                                      maxiFFT&                 fft)
{
    // Keep it auto as it may or may not be double precision.
    const auto readptrs = data.getArrayOfReadPointers();
    for (int i = 0; i < data.getNumSamples(); ++i)
    {
        // Mono the frame.
        int channel = 0;
        auto currentFrame = readptrs[channel][i];
        const int numberChannels = data.getNumChannels();

        while (++channel < numberChannels)
            currentFrame += readptrs[channel][i];

        currentFrame /= numberChannels;

        // Save the audio for playback and plotting!
        processedMonoAudioPreview.push_back (currentFrame);

        // RMS.
        currentRmsFrame += (currentFrame * currentFrame);

        // Extract features.
        if (fft.process (currentFrame))
        {
            // This isn't real-time so I can take the luxuary of allocating
            // heap memory here.
            double* mfccs = new double[13];
            mfcc.mfcc (fft.magnitudes, mfccs);

            std::array<double, 13> mfccsFrame;
            std::memcpy (mfccsFrame.data(), mfccs, sizeof (double) * 13);

            // Add the mfcc frames here.
            mfccFeatures.push_back (mfccsFrame);
            delete[] mfccs;

            // Root Mean Square.
            currentRmsFrame /= fftSize;
            currentRmsFrame = sqrt (currentRmsFrame);
            rmsFrames.push_back (currentRmsFrame);
            currentRmsFrame = 0.0;
        }
    }
}

//=============================================================================
void RenderEngine::ifTimeSetNoteOff (const double noteLength,
                                     const double sampleRate,
                                     const int    bufferSize,
                                     const uint8  midiChannel,
                                     const uint8  midiPitch,
                                     const uint8  midiVelocity,
                                     const int    currentBufferIndex,
                                     MidiBuffer&  bufferToNoteOff)
{
    double eventFrame = noteLength * sampleRate;
    double bufferStartSample = currentBufferIndex * bufferSize;
    
    bool bufferBeginIsBeforeEvent = bufferStartSample < eventFrame;
    bool bufferEndIsAfterEvent = bufferStartSample + bufferSize >= eventFrame;
    bool noteOffEvent = bufferBeginIsBeforeEvent && bufferEndIsAfterEvent;
    
    if (noteOffEvent)
    {
        MidiMessage offMessage = MidiMessage::noteOff (midiChannel,
                                                       midiPitch,
                                                       midiVelocity);
        
        double samplesUntilNoteOff = eventFrame - bufferStartSample;
        bufferToNoteOff.addEvent(offMessage, samplesUntilNoteOff);
    }
}

//==============================================================================
bool RenderEngine::overridePluginParameter (const int   index,
                                            const float value)
{
    int biggestParameterIndex = pluginParameters.size() - 1;

    if (biggestParameterIndex < 0)
    {
        std::cout << "RenderEngine::overridePluginParameter error: " <<
                     "No patch set. Is the plugin loaded?" << std::endl;
        return false;
    }
    else if (index > pluginParameters[biggestParameterIndex].first)
    {
        std::cout << "RenderEngine::overridePluginParameter error: " <<
                     "Overriden parameter index is greater than the biggest parameter index." <<
                     std::endl;
        return false;
    }
    else if (index < 0)
    {
        std::cout << "RenderEngine::overridePluginParameter error: " <<
                     "Overriden parameter index is less than the smallest parameter index." <<
                     std::endl;
        return false;
    }
    else if (value < 0.0 || value > 1.0)
    {
        std::cout << "RenderEngine::overridePluginParameter error: " <<
                     "Keep the overriden value between 0.0 and 1.0." <<
                     std::endl;
        return false;
    }

    auto iterator = std::find_if (overridenParameters.begin(),
                                  overridenParameters.end(),
                                  [&index] (const std::pair<int, float>& parameter)
                                  {
                                      return parameter.first == index;
                                  });

    bool exists = (iterator != overridenParameters.end());

    if (exists)
        iterator->second = value;
    else
        overridenParameters.push_back(std::make_pair(index, value));

    return true;
}

//==============================================================================
bool RenderEngine::removeOverridenParameter (const int index)
{
    int biggestParameterIndex = pluginParameters.size() - 1;

    if (biggestParameterIndex < 0)
    {
        std::cout << "RenderEngine::removeOverridenParameter error: " <<
                     "No patch set. Is the plugin loaded?" << std::endl;
        return false;
    }
    else if (index > pluginParameters[biggestParameterIndex].first)
    {
        std::cout << "RenderEngine::removeOverridenParameter error: " <<
                     "Overriden parameter index is greater than the biggest parameter index." <<
                     std::endl;
        return false;
    }
    else if (index < 0)
    {
        std::cout << "RenderEngine::removeOverridenParameter error: " <<
                     "Overriden parameter index is less than the smallest parameter index." <<
                     std::endl;
        return false;
    }

    auto iterator = std::find_if (overridenParameters.begin(),
                                  overridenParameters.end(),
                                  [&index] (const std::pair<int, float>& parameter)
                                  {
                                      return parameter.first == index;
                                  });

    bool exists = (iterator != overridenParameters.end());

    if (exists)
    {
        overridenParameters.erase(iterator);
        return true;
    }

    std::cout << "RenderEngine::removeOverridenParameter error: " <<
                 "Overriden parameter does not exist." <<
                 std::endl;
    return false;
}

//==============================================================================
void RenderEngine::fillAvailablePluginParameters (PluginPatch& params)
{
    params.clear();
    params.reserve (plugin->getNumParameters());

    int usedParameterAmount = 0;
    for (int i = 0; i < plugin->getNumParameters(); ++i)
    {
        // Ensure the parameter is not unused.
        if (plugin->getParameterName(i) != "Param")
        {
            ++usedParameterAmount;
            params.push_back (std::make_pair (i, 0.0f));
        }
    }
    params.shrink_to_fit();
}

//==============================================================================
ParameterNameList RenderEngine::getPluginParametersDescription()
{
    ParameterNameList namedParameters;

    if (plugin != nullptr)
    {
        std::ostringstream ss;

        for (const auto& pair : pluginParameters)
        {
            namedParameters.push_back(std::make_pair(pair.first, plugin->getParameterName (pair.first).toStdString()));
        }
    }
    return namedParameters;
}

//==============================================================================
void RenderEngine::setPatch (const PluginPatch patch)
{
    const size_t currentParameterSize = pluginParameters.size();
    const size_t newPatchParameterSize = patch.size();

    if (currentParameterSize == newPatchParameterSize)
    {
        pluginParameters = patch;
    }
    else
    {
        std::cout << "RenderEngine::setPatch error: Incorrect patch size!" <<
        "\n- Current size:  " << currentParameterSize <<
        "\n- Supplied size: " << newPatchParameterSize << std::endl;
    }
}

//==============================================================================
const PluginPatch RenderEngine::getPatch()
{
    if (overridenParameters.size() == 0)
        return pluginParameters;

    PluginPatch overridenPluginParameters = pluginParameters;
    std::pair<int, float> copy;

    for (auto& parameter : overridenPluginParameters)
    {
        // Should we have overriden this parameter's index...
        if (std::any_of(overridenParameters.begin(), overridenParameters.end(),
                        [parameter, &copy] (std::pair<int, float> p)
                        {
                            copy = p;
                            return p.first == parameter.first;
                        }))
        {
            parameter = copy;
        }
    }
    return overridenPluginParameters;
}

//==============================================================================
const size_t RenderEngine::getPluginParameterSize()
{
    return pluginParameters.size();
}

//==============================================================================
const MFCCFeatures RenderEngine::getMFCCFrames()
{
    return mfccFeatures;
}

//==============================================================================
const MFCCFeatures RenderEngine::getNormalisedMFCCFrames(const std::array<double, 13>& mean,
                                                         const std::array<double, 13>& variance)
{
    MFCCFeatures normalisedMFCCFrames;
    normalisedMFCCFrames.resize (mfccFeatures.size());

    for (size_t i = 0; i < normalisedMFCCFrames.size(); ++i)
    {
        for (size_t j = 0; j < 13; ++j)
        {
            normalisedMFCCFrames[i][j] = mfccFeatures[i][j] - mean[j];
            normalisedMFCCFrames[i][j] /= variance[j];
        }
    }
    return normalisedMFCCFrames;
}

//==============================================================================
const std::vector<double> RenderEngine::getAudioFrames()
{
    return processedMonoAudioPreview;
}

//==============================================================================
const std::vector<double> RenderEngine::getRMSFrames()
{
    return rmsFrames;
}

//==============================================================================
bool RenderEngine::writeToWav(const std::string& path)
{
    const auto size = processedMonoAudioPreview.size();
    if (size == 0)
        return false;

    maxiRecorder recorder;
    recorder.setup (path);
    recorder.startRecording();
    const double* data = processedMonoAudioPreview.data();
    recorder.passData (data, size);
    recorder.stopRecording();
    recorder.saveToWav();
    return true;
}
