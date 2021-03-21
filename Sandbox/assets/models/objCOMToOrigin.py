import numpy as np

def vec3(string):
    i1 = line.find(" ") + 1
    i2 = line.find(" ", i1+1)
    i3 = line.find(" ", i2+1)
    try:
        i4 = line.find(" ",i3+1)
    except:
        i4 = -1
    vec = np.asarray([float(line[i1:i2]), float(line[i2:i3]), float(line[i3:i4])])
    return vec

def DistanceVector(R, r):
    vec = np.zeros(len(R))
    for i in range(0, len(vec)):
        vec[i] = np.sqrt(np.sum((R[i] - r) ** 2))
    return vec

objFile = "C:/Users/mart_/Desktop/dev/Hazel/Sandbox/assets/models/PyramidProbes.obj"
model = "C:/Users/mart_/Desktop/dev/Hazel/Sandbox/assets/models/Pyramid.obj"
f = open(objFile)
PROBES = list()
_tag = float('nan')
for line in f:
    if (line[:3] == "pr "):
        _vec = vec3(line)
        PROBES.append(_vec)

COM = np.asarray([0.0,0.0,0.0])    
M = 0
for p in PROBES:
    m = 1.0
    COM += p * m
    M += m
COM /= M

print("\nCenter of mass at: ({:.2f}, {:.2f}, {:.2f}) meter in x, y, z.".format(COM[0], COM[1], COM[2]))    
print("Total mass: {:.2f} kg".format(M))

