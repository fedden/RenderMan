```
       ______               _          ___  ___
       | ___ \             | |         |  \/  |
       | |_/ /___ _ __   __| | ___ _ __| .  . | __ _ _ __
       |    // _ \ '_ \ / _` |/ _ \ '__| |\/| |/ _` | '_ \
       | |\ \  __/ | | | (_| |  __/ |  | |  | | (_| | | | |
       \_| \_\___|_| |_|\__,_|\___|_|  \_|  |_/\__,_|_| |_|

* *  Command Line VST Audio, Features and Parameter Renderer  * *
```

| `build` |
|:-------:|
| [![Build Status](https://travis-ci.org/fedden/RenderMan.svg?branch=master)](https://travis-ci.org/fedden/RenderMan) |

# RenderMan

Renderman is a command line VST host written in C++ with Python bindings using [JUCE](https://github.com/julianstorer/JUCE) and [Maximilian](https://github.com/micknoise/Maximilian) libraries for the backend. It is designed with ease of use in mind to extract audio and features from VST plugins. It has a fast growing list of features, including setting, getting parameters from synthesiers, setting whole patches, getting random patches, obtaining MFCCS, FFT, audio data and much more.

A usage example in the form of an IPython notebook can be found [here](http://doc.gold.ac.uk/~lfedd001/renderman.html).

Here are some quick gifs demonstrating a miniscule amount of the availble features, go towards the bottom of the README to see the full API:

#### Loading Plugins

Here we load a plugin. On Linux it's an .so file, for MacOS it will be a .vst or .au.
![](https://github.com/fedden/RenderMan/raw/master/Documentation/loading_plugin.gif)

#### Getting Parameter Descriptions

We can obtain the available parameters that are used and that can be modified by doing the following.
![](https://github.com/fedden/RenderMan/raw/master/Documentation/plugin_description.gif)

#### Getting a Randomised Patch For a Synthesiser

We can easily get a randomised for a given synth by using the PatchGenerator class.
![](https://github.com/fedden/RenderMan/raw/master/Documentation/print_random_patch.gif)

#### Plotting Rendered Audio Frames

We can plot the output audio frames easily by using matplotlib.
![](https://github.com/fedden/RenderMan/raw/master/Documentation/plotting_audio_frames.gif)

## Building / Installation

### MacOS

If you haven't already, [get brew](https://brew.sh/). The last time I checked the command to install was simply this:
```
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```

Next, get the boost headers.
```
brew install boost-python
```

You can also install boost manually if for some reason you don't want to use brew, [see here.](http://www.boost.org/doc/libs/1_50_0/doc/html/quickbook/install.html)

Now just open the Xcode project in the Builds directory and build it! There is a bug in the JUCE projucer app which means the generated shared object will be suffixed with a dylib. This means python wont be able to import the module. Until this bug is fixed, change directory into the Builds/MacOSX/build/<Debug/Release> (depending on your Xcode scheme,) and run:
```
mv librenderman.so.dylib librenderman.so
```

### Linux

Firstly, you will need the boost library (specifically the python headers) for this code to compile.

Ubuntu:
```
sudo apt-get install libboost-all-dev
```

Arch:
```
sudo pacman -Ss boost
```

Fedora:
```
sudo yum install boost-devel
```

If your distribution's package manager doesn't have boost, [you can get the headers from here.](http://www.boost.org/doc/libs/1_47_0/more/getting_started/unix-variants.html)

Juce itself has a list of dependancies for Linux; it's a very big library - if you don't know it you should definitely take some time out to check it out! Depending on your distribution and setup you may already have some / all of the following libraries. If you are on Ubuntu, the following commands will install your dependancies. Find the respective packages for other distros using google please!

```
sudo apt-get -y install llvm
sudo apt-get -y install clang
sudo apt-get -y install libfreetype6-dev
sudo apt-get -y install libx11-dev
sudo apt-get -y install libxinerama-dev
sudo apt-get -y install libxrandr-dev
sudo apt-get -y install libxcursor-dev
sudo apt-get -y install mesa-common-dev
sudo apt-get -y install libasound2-dev
sudo apt-get -y install freeglut3-dev
sudo apt-get -y install libxcomposite-dev
sudo apt-get -y install libcurl4-gnutls-dev
```

Well done! You've made it this far! Should you still have problems, which is always a possibility with Linux, a good place to start is the JUCE forums, particularly [here](https://forum.juce.com/t/juce-4-2-1-setup-on-apt-based-linux-ubuntu-16-04-lts-mint-elementary-os-freya/17164) and [here](https://forum.juce.com/t/list-of-juce-dependencies-under-linux/15121). Feel free to drop me a note with an error and I'll happily scratch my head over it but you may get better results in the forums!

So to now build the library for Linux, change to the right directory and run make:
```
cd Builds/LinuxMakefile/
make
```

### Windows

Windows isn't ready yet, but is in my to-do list. Patches, errors and notes are very welcome here!

## Does It Work?

Change directory to where the .so file is and run:
```
python
```
Once in interactive mode, run:
```
import librenderman as rm
```
If this doesn't spit out errors, congratulations! Enjoy RenderMan. :)

_Note you'll need to have the .so binary in the same directory as the Python project or where you call the interative Python shell from. To use it system wide it will need to be added to the PYTHONPATH environment variable. Soon I'll get distutils sorted so there is an easier installation method._

## API

##### class RenderEngine
The constructor takes three arguments, the [sample rate](http://wiki.audacityteam.org/wiki/Sample_Rates), the buffer size and fft size. Good defualt values if you don't really care that much are 44100, 512, 512 repsectively.
```
__init__(int sample_rate,
         int buffer_size,
         int fft_size)
```
Supply a full path to the plugin to this function to load the vst. It will return true if there is a successful loading.
```
bool load_plugin(string plugin_path)
```
We can set a synth's patch by taking a list of tuples and set the parameters at the int index to the float value. The PatchGenerator class can generate random patches with ease for a given synth.
```
void set_patch(list_of_tuples(int, float) patch)
```
Get the current patch.
```
list_of_tuples(int, float) get_patch()
```  
Take a midi note (middle C is 40,) a velocity (0 - 127,) and the note length and recording / rendering length and create the features to be extracted!
```
void render_patch(int   midi_note_pitch,
                  int   midi_note_velocity,
                  float note_length_seconds,
                  float render_length_seconds)
```
Get MFCC features as a list of lists. The first length will be dictated by fft size divided by four, and the second length with be 13, which is the amount of coefficients.
```
list_of_lists get_mfcc_frames()
```   
Get the int amount of parameters for the loaded plugin.
```
int get_plugin_parameter_size()
```  
Get a description of each parameter.
```
string get_plugin_parameters_description()
```
Override a parameter to always be the supplied value. The float is normalised (0 - 1).
```
override_plugin_parameter(int   index,
                          float value)
```  
Remove an overriden plugin parameter.
```
remove_overriden_plugin_parameter(int index)
```
Get a list of floats which is the audio from the rendering session.
```
list get_audio_frames()
```  
Write the current patch to a wav file at the specified relative or absolute path. This will overwrite existing files and is only a preview; it is mono and currently not quite loud enough.
```
void write_to_wav(string path)
```
Get a list of root mean squared frames derived from the audio samples. Each frame is a root mean squared of an amount of samples equal to the fft size divided by four.
```
list_of_floats get_rms_frames()
```

##### class PatchGenerator
This class is used to generate patches for a given engine.

The constructor takes an argument of a RenderEngine that has succesfully loaded a plugin.
```
__init__(RenderEngine engine)
```
We can obtain a random value (from a real uniform distribution) that for a parameter at a specified index.
```      
tuple(int, float) get_random_parameter(int index)
```
We can get a completely random patch sampled from PRNG in the same way above.
```      
list_of_tuples(int, float) get_random_patch()
```

## Contributors

I want to express my deep gratitude to [jgefele](https://github.com/jgefele). It is very touching that people want to use this code let alone contribute to it - thanks!

## Context and Contact
This library is a work in progress for my final year project where I am using Neural Networks to generate synth patches to create desired sounds. For example, I wrote a toy VST and learnt synthesiser using a neural network to sound match target sounds [here](http://doc.gold.ac.uk/~lfedd001/tf_fm_synth_learn.html). Everything this library has facilitates that but I recognise there may be more applications so if there are any feature requests please drop me a line on here or leonfedden (at) gmail.com :)

Finally, doing something cool with this library? Let me know what you are up to! And if this code was useful for you then please kindly drop me a GitHub star so more developers will trust and use this code.

If you use this code for academic works, feel free to reference this code through the DOI here:
[![DOI](https://zenodo.org/badge/82790125.svg)](https://zenodo.org/badge/latestdoi/82790125)

Thanks for reading this far, you rock!

![](https://media.giphy.com/media/lr2bkSkxMWFCo/giphy.gif)
