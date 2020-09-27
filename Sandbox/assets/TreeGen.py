# -*- coding: utf-8 -*-
"""
Created on Sat Sep 26 14:08:32 2020

@author: Mart
"""

import numpy as np
from random import uniform
import matplotlib.pyplot as plt
from time import sleep, strftime, gmtime
import os

# 1 plakje:
class Object:
    def __init__(self, Name):
        self.Name = Name
        self.Vertices = list()
        self.Faces = list()
    def addVertex(self, V):
        line = "v {:.3f} {:.3f} {:.3f}\n".format(V[0], V[2], V[1])
        self.Vertices.append(line)
        
    def addFace(self, F):
        line = "f {}// {}// {}//\n".format(F[0], F[1], F[2])
        self.Faces.append(line)
        
    def export(self):
        sleep(1)
        dir_path = os.path.dirname(os.path.realpath(__file__)).replace("\\","/")
        timestamp = strftime("%Y-%m-%d %Hh%Mm%Ss", gmtime())
        file = open(dir_path+"/"+timestamp+"-"+self.Name+".obj", "w")
        file.write("o "+self.Name+" \n")
        for line in self.Vertices:
            file.write(line)
        for line in self.Faces:
            file.write(line)
        

obj = Object("Tree")
# # # PER LAYER SETTINGS
nVertices = 8; # amount of vertices per layer polygon
vNoise = 0.05; # distortion in vertex position (from perfect n-polygon position)
zNoise = 0.02; # distortion in height of a vertex (added to layer height)
mainDeltaZ = 0.22; # distance between layer centers in Z
layerDeltaZ = 0.2; # distance from bottom to top face of a layer
rad = 1.0; # radius of the bottom face
cNoise = 0.1; # noise in center position of each layer
mainThinFac = 0.1 # radius by which layers shrink (XY) per layer
layerThinFac = 0.8 # factor by which bottom face of layer is smaller than top face

vIdx = 0
lIdx = 0

rad = 1.0 + uniform(-0.3, 0.3)
mainDeltaZ = 0.3
layerDeltaZ = mainDeltaZ + uniform(-0.05, 0.05)
cNoise = uniform(0.0, 0.2)
vNoise = uniform(0.1, 0.3)
zNoise = uniform(0.0, 0.05)
mainThinFac = uniform(0.07, 0.10)
top = uniform(0.09, 0.31)
nVertices = 6
# add layer by layer
_Cprev = [[0, 0, 0, 0], [0, 0, 0, 0]]
while (rad > top):
    B = list()
    T = list()
    C = list()
    lIdx += 1
    vIdx += 1
    _cNoiseVec = [uniform(-cNoise, cNoise), uniform(-cNoise, cNoise)]
    C.append([vIdx, _Cprev[0][1] + _cNoiseVec[0], _Cprev[0][2] + _cNoiseVec[1], lIdx * mainDeltaZ + uniform(-zNoise, zNoise)])
    vIdx += 1
    C.append([vIdx, _Cprev[1][1] + _cNoiseVec[0], _Cprev[0][2] + _cNoiseVec[1], lIdx * mainDeltaZ + uniform(-zNoise, zNoise) + layerDeltaZ])
    
    _Cprev = C
    rad -= mainThinFac
    offset = uniform(0, 2*np.pi)
    for v in range(0, nVertices):
            th = (v+1) * 2 * np.pi / nVertices + offset
            x = rad * (np.cos(th) + uniform(-vNoise, vNoise)) + C[0][1]
            y = rad * (np.sin(th) + uniform(-vNoise, vNoise)) + C[0][2]
            z = C[0][3] + uniform(-zNoise, zNoise) * rad
            vIdx = vIdx + 1
            B.append([vIdx, x, y, z])      
            th = (v + 1.5) * 2 * np.pi / nVertices + offset
            x = (np.cos(th) + uniform(-vNoise, vNoise)) * rad * layerThinFac + C[1][1]
            y = (np.sin(th) + uniform(-vNoise, vNoise)) * rad * layerThinFac + C[1][2]
            z = C[1][3] + uniform(-zNoise, zNoise) * rad * layerThinFac
            vIdx = vIdx + 1
            T.append([vIdx, x, y, z])    
    for vertex in C:
        obj.addVertex(vertex[1:])
    for i in range(len(B)):
        obj.addVertex(B[i][1:])
        obj.addVertex(T[i][1:])
        
    for f in range(0, nVertices):
        i0 = f
        i1 = (f+1) % nVertices
        F = list()
        F.append([C[0][0], B[i0][0], B[i1][0]])
        F.append([C[1][0], T[i0][0], T[i1][0]])
        F.append([B[i0][0], T[i0][0], B[i1][0]])
        F.append([B[i1][0], T[i0][0], T[i1][0]])
        for face in F:
            obj.addFace(face)
    
obj.export()
