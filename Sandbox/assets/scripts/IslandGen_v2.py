# -*- coding: utf-8 -*-
"""
Created on Mon Sep 21 15:33:35 2020

@author: Mart
"""

from noise import pnoise2, snoise2
from random import randrange, uniform
import matplotlib.pyplot as plt
import numpy as np
import os 
from time import gmtime, strftime, sleep
import copy
def normalize(array):
    array -= np.amin(array)
    array /= np.amax(array)
    return array
# Map parameters
## Height
N = 64
L = 2 # width of map in km
H = 1 # highest point - lowest point on map in km
octaves = 12
persistence = 0.32
lacunarity = 2.5
seed = randrange(10)
## Grid distortion
XZnoise = 0.0


heightMap = np.zeros((N, N, 3))
Scaling = np.zeros((N, N))
for x in range(0, N):
    for z in range(0, N):
        heightMap[x, z, 0] = pnoise2(x*L/N, z*L/N, octaves=octaves, persistence=persistence, lacunarity=lacunarity, base=seed)
        heightMap[x, z, 1] = max(0.0, 0.50 * pnoise2(x*2*L/N, z*2*L/N, octaves=octaves, persistence=persistence, lacunarity=lacunarity, base=seed))
        heightMap[x, z, 2] = max(0.0, 0.25 * pnoise2(x*4*L/N, z*4*L/N, octaves=octaves, persistence=persistence, lacunarity=lacunarity, base=seed))
        
del x, z

def plotHeightMap(rgb):
    plt.subplot(1,3,1)
    plt.imshow(heightMap[:,:,0], cmap = "gray")
    plt.subplot(1,3,2)
    plt.imshow(heightMap[:,:,1], cmap = "gray")
    plt.subplot(1,3,3)
    plt.imshow(heightMap[:,:,2], cmap = "gray")

def plotHeightMapTotal(rgb):
    temp = heightMap[:, :, 0] + heightMap[:, :, 1] + heightMap[:, :, 2]
    plt.imshow(temp, cmap = "gray")
#%%
    
def localGradient(position):
    x = int(np.floor(position[0]))
    z = int(np.floor(position[1]))
    u = position[0] - x
    v = position[1] - z
    tl = sum(heightMap[x, z, :])
    tr = sum(heightMap[x + 1, z, :])
    bl = sum(heightMap[x, z + 1, :])
    br = sum(heightMap[x + 1, z + 1, :])
    return np.asarray([(1 - v) * (tr - tl) + v * (br - bl), (1 - u) * (bl - tl) + u * (br - tr)])

def length(vec):
    return np.sqrt(sum(np.square(vec)))

VOLUME = 10.0
INERTIA = 0.01
MAXSTEPS = 4
plotHeightMapTotal(heightMap)

def droplet(position):
    position = position * 1.0
    direction = np.asarray([0.0, 0.0])
    STEPS = 0
    while STEPS<MAXSTEPS:
        gradient = localGradient(position)
        direction = INERTIA * direction + gradient * (1 - INERTIA)
        _direction_len = length(direction)
        if _direction_len == 0.0:
            _theta = randrange(1000)/1000 * 2 * np.pi
            direction = np.asarray([np.cos(_theta), np.sin(_theta)])
            
        direction = direction / length(direction)
        position += direction
        if ((position[0] < 0) | (position[1] < 0) | (position[0] > N-1) | (position[1] > N-1)):
            break
        plt.plot(position[0], position[1], marker = 'o', markersize = 1, color = [STEPS / MAXSTEPS, (1 - STEPS / MAXSTEPS), 0.0])
        STEPS+=1

_bin = 2;
for i in range(0, int((N-_bin) / _bin)):
    for j in range(0, int((N-_bin) / _bin)):
        print(i, j)
        droplet(np.asarray([i*1.0*_bin, j*1.0*_bin]))
    
        
plt.show()