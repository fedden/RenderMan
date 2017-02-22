import renderman as rm

engine = rm.RenderEngine(44100, 512, 512)

engine.load_plugin("/home/tollie/Development/vsts/dexed/Builds/Linux/build/Dexed.so")

p = engine.get_patch()

newPatch = zip(*p)

newTuple = (0.489,) * len(p)

newPatch[1] = newTuple

print newPatch

z = zip(newPatch[0], newPatch[1])

print z

engine.set_patch(z)

print engine.get_patch()

print
print
print
print
print
print
print
print

engine.render_patch(40, 255, 0.2, 0.5)

print len(engine.get_mfcc_features())

print engine.get_plugin_parameter_size()

print engine.get_plugin_parameters_description()

engine.override_plugin_parameter(0, 1.0)
