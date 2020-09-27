import numpy as np

def vec3(string):
    i1 = line.find(" ") + 1
    i2 = line.find(" ", i1+1)
    i3 = line.find(" ", i2+1)
    try:
        i4 = line.find(" ",i3+1)
    except:
        i4 = -1
    vec = [float(line[i1:i2]), float(line[i2:i3]), float(line[i3:i4])]
    return vec

def DistanceVector(R, r):
    vec = np.zeros(len(R))
    for i in range(0, len(vec)):
        vec[i] = np.sqrt(np.sum((R[i] - r) ** 2))
    return vec

objFile = "C:/Users/Mart/Desktop/dev/Hazel/Sandbox/assets/models/Raft.obj"
f = open(objFile)
PROBES = list()
_tag = float('nan')
# order of probe parameters: X, Y, Z, density, Nx, Ny, Nz, Volume/Area
_V = [1.0, 20.0, 1.0] # floater, dragger, sinker volume
_D = [0.3, 0.0, 1.1] # density


currentType = -1

lineNum = 0
for line in f:
    if line.find("#floaters") != -1:
        currentType = 0
        print("Floaters found")
        _tag = np.nanmin([_tag, lineNum])
    elif line.find("#draggers") != -1:
        currentType = 1
        print("Draggers found")
        _tag = np.nanmin([_tag, lineNum])
    elif line.find("#sinkers") != -1:
        currentType = 2
        print("Sinkers found")
        _tag = np.nanmin([_tag, lineNum])
    if ((line[:2] == "v ") or line[:3]=="fl ") and not (currentType == -1):
        _vec = vec3(line)
        _vec.insert(0, currentType)
        PROBES.append(_vec)
    lineNum += 1

# find center of mass
    
COM = np.asarray([0.0,0.0,0.0])    
M = 0
Vtot = 0
for p in PROBES:
    X = np.asarray(p[1:])
    m = _V[p[0]] * _D[p[0]]
    COM += X * m
    M += m
    if p[0]!=1:
        Vtot += _V[p[0]]
COM /= M

print("\nCenter of mass at: ({:.2f}, {:.2f}, {:.2f}) meter in x, y, z.".format(COM[0], COM[1], COM[2]))    
print("Total mass: {:.2f} kg".format(M))
print("Net density: {:.2f} kg/m3".format(M / Vtot))

# CALCULATE MOMENT OF INERTIA
I = np.asarray([[0.0,0.0,0.0],[0.0,0.0,0.0],[0.0,0.0,0.0]])
i = np.asarray([[1.0, 0.0, 0.0],[0.0, 1.0, 0.0], [0.0, 0.0, 1.0]])

ix = 0;
for p in PROBES:
    X = [p[1:] - COM]
    m = _V[p[0]] * _D[p[0]]
    I += m * ((np.dot(X[0], X[0]))*i - np.transpose(X) * X)
    ix += 1

print("Moment of inertia matrix:\n")
for l in I:
    print("\t{:.4f}\t{:.4f}\t{:.4f}".format(l[0], l[1], l[2]))
       
# FIND DRAGGER NORMALS
draggers = list()
for p in PROBES:
    if p[0] == 1:
        draggers.append(np.asarray(p[1:]))


draggersFinal = list()
while len(draggers) >= 2:
    d = draggers[0]
    D = draggers[1:]
    R = DistanceVector(D, d)
    ix = np.argmin(R)
    normal = (D[ix] - d)
    normal = normal / np.sqrt(np.sum((normal**2)))
    # set right direction for the normal
    if np.sqrt(np.sum((D[ix]**2))) < np.sqrt(np.sum((d**2))):
        normal = -normal
    final = [d[0], d[1], d[2], _D[1], normal[0], normal[1], normal[2], _V[1]]
    draggersFinal.append(final)
    draggers.pop(ix+1)
    draggers.pop(0)
    
## Write to file
with open(objFile, 'r') as obj:
    lines = obj.read().splitlines()
del(obj)

newFile = lines[:int(_tag)]
newFile.append("#floaters")
for p in PROBES:
    if p[0] == 0 or p[0] == 2:
        _line = "fl {:.8f} {:.8f} {:.8f} {:.8f} {:.8f} {:.8f} {:.8f} {:.8f}".format(p[1]-COM[0], p[2]-COM[1], p[3]-COM[2], _D[p[0]], 2.0, 2.0, 2.0, _V[p[0]])
        newFile.append(_line)
        
for p in draggersFinal:
    _line = "fl {:.8f} {:.8f} {:.8f} {:.8f} {:.8f} {:.8f} {:.8f} {:.8f}".format(p[0]-COM[0], p[1]-COM[1], p[2]-COM[2], p[3], p[4], p[5], p[6], p[7])
    newFile.append(_line)

ix = 0
for line in newFile:
    # offset all XYZ by -COM
    if line[:2] == "v ":
        temp = line.split(" ")
        x = [float(temp[1]), float(temp[2]), float(temp[3])]
        temp = "v {:.6f} {:.6f} {:.6f}".format(x[0]-COM[0], x[1]-COM[1], x[2]-COM[2])
        newFile[ix] = temp
    ix += 1

f = open(objFile[0:objFile.rfind(".")] + 'Valid.obj', 'w').write('\n'.join(newFile))
del(f)
    

print("\nglm::mat3 I = glm::mat3({:.4f},{:.4f},{:.4f},{:.4f},{:.4f},{:.4f},{:.4f},{:.4f},{:.4f});".format(I[0][0],I[1][0],I[2][0], I[0][1],I[1][1],I[2][1], I[0][2],I[1][2],I[2][2]))
print("float M = {:.4f};\n\n".format(M))

print("5 x higher + zero diagonals:")
I *= 5
I[0, 1] = I[0, 2] = I[1, 0] = I[1, 2] = I[2, 0] = I[2, 1] = 0
M *= 5
print("\nglm::mat3 I = glm::mat3({:.4f},{:.4f},{:.4f},{:.4f},{:.4f},{:.4f},{:.4f},{:.4f},{:.4f});".format(I[0][0],I[1][0],I[2][0], I[0][1],I[1][1],I[2][1], I[0][2],I[1][2],I[2][2]))
print("float M = {:.4f};".format(M))

#%% CHECK Center Of Mass
fopcom = np.asarray([0.0, 0.0, 0.0])
MM = 0.0
for line in newFile:
    if line[:3] == "fl ":
        temp = line.split(" ")
        x = np.asarray([float(temp[1]), float(temp[2]), float(temp[3])])
        d = float(temp[4])
        V = float(temp[8])
        fopcom += x * d * V
        MM += d * V
fopcom /= MM

I = np.asarray([[0.0,0.0,0.0],[0.0,0.0,0.0],[0.0,0.0,0.0]])
i = np.asarray([[1.0, 0.0, 0.0],[0.0, 1.0, 0.0], [0.0, 0.0, 1.0]])    
for line in newFile:
    if line[:3] == "fl ":
        temp = line.split(" ")
        x = [[float(temp[1]), float(temp[2]), float(temp[3])]]
        d = float(temp[4])
        V = float(temp[8])
        I += d * V * ((np.dot(x[0], x[0]))*i - np.transpose(x) * x)
print("COM: {:.6f} {:.6f} {:.6f}".format(fopcom[0], fopcom[1], fopcom[2]))
for j in I:
    print("{:.6f}\t{:.6f}\t{:.6f}".format(j[0], j[1], j[2]))
#%% VISUALIZE DRAGGERS
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
plt.close("all")
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
for p in draggersFinal:
    X = np.asarray([p[0], p[1], p[2]])
    N = X + 0.02 * np.asarray([p[4], p[5], p[6]])
    ax.scatter(X[0], X[1], X[2], marker = 'o', color = [0,0,0])
    ax.scatter(N[0], N[1], N[2], marker = 'o', color = [0,0.5,0])