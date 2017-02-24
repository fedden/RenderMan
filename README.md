```
       ______               _          ___  ___
       | ___ \             | |         |  \/  |
       | |_/ /___ _ __   __| | ___ _ __| .  . | __ _ _ __
       |    // _ \ '_ \ / _` |/ _ \ '__| |\/| |/ _` | '_ \
       | |\ \  __/ | | | (_| |  __/ |  | |  | | (_| | | | |
       \_| \_\___|_| |_|\__,_|\___|_|  \_|  |_/\__,_|_| |_|

* *  Command Line VST Audio, Features and Parameter Renderer  * *
```

# RenderMan

Renderman is a command line VST host written in C++ with Python bindings using [JUCE](https://github.com/julianstorer/JUCE) and [Maximilian](https://github.com/micknoise/Maximilian) libraries for the backend. It is designed with ease of use in mind to extract audio and features from VST plugins. It has a fast growing list of features, including setting, getting parameters from synthesiers, setting whole patches, getting random patches, obtaining MFCCS, FFT, audio data and much more.

A usage example in the form of an IPython notebook can be found [here](http://doc.gold.ac.uk/~lfedd001/renderman.html).

Here are some quick gifs demonstrating a miniscule amount of the availble features, go to the bottom of the read me to see the full API:

### Loading Plugins

Here we load a plugin. On Linux it's an .so file, for MacOS it will be a .vst or .au.
![](https://github.com/fedden/RenderMan/raw/master/Documentation/loading_plugin.gif)

### Getting Parameter Descriptions

We can obtain the available parameters that are used and that can be modified by doing the following.
![](https://github.com/fedden/RenderMan/raw/master/Documentation/plugin_description.gif)

### Getting a Randomised Patch For a Synthesiser

We can easily get a randomised for a given synth by using the PatchGenerator class.
![](https://github.com/fedden/RenderMan/raw/master/Documentation/print_random_patch.gif)

### Plotting Rendered Audio Frames

We can plot the output audio frames easily by using matplotlib.
![](https://github.com/fedden/RenderMan/raw/master/Documentation/plotting_audio_frames.gif)

## Building / Installation

First, obtain Steinberg's VST SDK. [Obtain it from here](https://www.steinberg.net/en/company/developers.html), last time I checked it was on 3.6.6.

We need to ensure a folder titled 'SDKs' exists at the home directory.
```
mkdir ~/SDKs
```

Ensure the VST3 SDK is moved to the folder "~/SDKs/" so that JUCE can find the VST SDK headers.
```
mv ~/Downloads/VST3\ SDK/ ~/SDKs/
```

### MacOS

If you haven't already, [get brew](https://brew.sh/). The last time I checked the command was this:
```
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```

Then get the boost headers.
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

So to now build the library for Linux, change directory to Builds/LinuxMakefile/ and run simply run make.

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
Get MFCC features as a list of lists. The first length will be dictated by fft size, smaller means a greater set of mfccs, and the second length with be 13, which is the amount of coefficients.
```
list_of_lists get_mfcc_features()
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
override_plugin_parameter(int index,
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

## Context and Contact
This library is a work in progress for my final year project where I am using Neural Networks to generate synth patches to create desired sounds. For example, I wrote a toy VST and learnt close parameters [here](http://doc.gold.ac.uk/~lfedd001/tf_fm_synth_learn.html). Everything this library has facilitates that but I recognise there may be more applications so if there are any feature requests please drop me a line on here or leonfedden<at>gmail.com :)

Finally, doing something cool with this library? Let me know what you are up to! And if this code was useful for you then please kindly drop me a GitHub star so more developers will trust and use this code.

Thanks for reading this far, you rock!

![](https://media.giphy.com/media/lr2bkSkxMWFCo/giphy.gif)
