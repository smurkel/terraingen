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
N = 512
L = 2 # width of map in km
H = 1 # highest point - lowest point on map in km
scaling = False
scaleRmin = 0.6 # inner radius of scaling cone
scaleRmax = L / 2 * np.sqrt(2) # outer radius of scaling cone
scaleMagnitude =  H # height offset scaling factor between center (scaleMagnitude + 1) * height and edge1 * height
octaves = 12
persistence = 0.32
lacunarity = 2.5
seed = randrange(500)
## Grid distortion
XZnoise = 0.0


Map = np.zeros((N, N, 3))
Scaling = np.zeros((N, N))
for x in range(0, N):
    for y in range(0, N):
        #Map[x, y, 0] = (x + (y % 2) / 2 + (uniform(0, 1) - 0.5) * XZnoise) * L / N
        Map[x, y, 0] = (x + (uniform(0, 1) - 0.5) * XZnoise) * L / N
        Map[x, y, 2] = (y + (uniform(0, 1) - 0.5) * XZnoise) * L / N
        if scaling:
            distFromCenter = np.sqrt(((Map[x, y, 0] - L/2)**2 + (Map[x, y, 2] - L/2)**2))
            distFromCenter = (1 - (max(min(distFromCenter, scaleRmax), scaleRmin) - scaleRmin) / (scaleRmax - scaleRmin)) * scaleMagnitude + 1
            Scaling[x, y] = distFromCenter
            Map[x, y, 1] = distFromCenter #+ pnoise2(Map[x, y, 0], Map[x, y, 2], octaves=octaves, persistence=persistence, lacunarity=lacunarity, base=seed)
        else:
            Map[x, y, 1] = pnoise2(Map[x, y, 0], Map[x, y, 2], octaves=octaves, persistence=persistence, lacunarity=lacunarity, base=seed)
        
Map[:,:,1] = normalize(Map[:,:,1]) * H
plt.imshow(Map[:,:,1])


def ExportObj():
    sleep(1)
    dir_path = os.path.dirname(os.path.realpath(__file__)).replace("\\","/")
    timestamp = strftime("%Y-%m-%d %Hh%Mm%Ss", gmtime())
    file = open(dir_path+"/"+timestamp+"_seed"+str(seed)+".obj", "w")
    file.write("o Island \n")
    for i in range(0, N):
        for j in range(0, N):
            x = Map[i, j, 0]
            y = Map[i, j, 1]
            z = Map[i, j, 2]
            file.write("v {:.3f} {:.3f} {:.3f} \n".format(x, y, z))
    for i in range(0, N-1):
        for j in range(0, N-1):
            selfIdx = 1 + i*N + j
            if (j%2)==0:
                file.write("f {}// {}// {}//\n".format(selfIdx, selfIdx + N, selfIdx + 1))
                file.write("f {}// {}// {}//\n".format(selfIdx + 1, selfIdx + N, selfIdx + 1 + N))
            else:
                file.write("f {}// {}// {}//\n".format(selfIdx, selfIdx + N, selfIdx + 1 + N))
                file.write("f {}// {}// {}//\n".format(selfIdx, selfIdx + N + 1, selfIdx + 1 ))

def circleIndicesXY(R):
    idx = list()
    w = list()
    X = np.asarray(range(-R, R+1))**2
    R2 = R**2
    
    for x in range(0, 2*R + 1):
        for y in range(0, 2*R + 1):
            if (X[x] + X[y]) <= R2:
                idx.append([x - R, y - R])
                w.append(R - np.sqrt(X[x] + X[y]))
    return np.asarray(idx), np.asarray(w)/np.sum(w)

#%
RADIUS = 5
cROI, weights = circleIndicesXY(RADIUS)
class droplet:
    def __init__(self, hmap, position, direction, velocity, volume, inertia):
        self.pos = np.asarray(position)
        self.dir = np.asarray(direction)
        self.vel = velocity
        self.vol = volume
        self.inr = inertia
        self.h   = self.localHeight(hmap)
        self.mapSize = len(hmap[:,:])
        # SIMULATION PARAMETERS
        self.sed = 0
        self.MINSLOPE = 0.0125
        self.CAPACITY = 1.0
        self.EROSION = 0.6
        self.GRAVITY = 10.0
        self.EVAPORATION = 0.05
        self.DEPOSITION = 0.05
        self.STEPS = 0
        self.MAXSTEPS = 128
        self.RADIUS = RADIUS
        
    def localGradient(self, hmap):
        x = int(self.pos[0])
        y = int(self.pos[1])
        u = self.pos[0]-x
        v = self.pos[1]-y
        
        tl = hmap[x,y]
        tr = hmap[x+1,y]
        bl = hmap[x,y+1]
        br = hmap[x+1,y+1]
        
        
        return np.asarray([(tr-tl)*(1-v) + (br-bl)* v, (bl-tl)*(1-u) + (br-tr)*u])
    
    def localHeight(self, hmap):
        x = self.pos[0]
        y = self.pos[1]
    
        x0 = np.floor(x).astype(int)
        x1 = x0 + 1
        y0 = np.floor(y).astype(int)
        y1 = y0 + 1
    
        x0 = np.clip(x0, 0, hmap.shape[1]-1);
        x1 = np.clip(x1, 0, hmap.shape[1]-1);
        y0 = np.clip(y0, 0, hmap.shape[0]-1);
        y1 = np.clip(y1, 0, hmap.shape[0]-1);
    
        Ia = hmap[ y0, x0 ]
        Ib = hmap[ y1, x0 ]
        Ic = hmap[ y0, x1 ]
        Id = hmap[ y1, x1 ]
    
        wa = (x1-x) * (y1-y)
        wb = (x1-x) * (y-y0)
        wc = (x-x0) * (y1-y)
        wd = (x-x0) * (y-y0)
    
        return wa*Ia + wb*Ib + wc*Ic + wd*Id
    
    def deposit(self, toDrop, oldPos, hmap):
        x = oldPos[0]
        y = oldPos[1]
        x0 = int(x)
        x1 = x0 + 1
        y0 = int(y)
        y1 = y0 + 1
        
        x0 = np.clip(x0, 0, hmap.shape[1]-1);
        x1 = np.clip(x1, 0, hmap.shape[1]-1);
        y0 = np.clip(y0, 0, hmap.shape[0]-1);
        y1 = np.clip(y1, 0, hmap.shape[0]-1);
        
        # wa = (x1-x) * (y1-y)
        # wb = (x1-x) * (y-y0)
        # wc = (x-x0) * (y1-y)
        # wd = (x-x0) * (y-y0)
        
        hmap[ y0, x0 ] += toDrop/4
        hmap[ y1, x0 ] += toDrop/4
        hmap[ y0, x1 ] += toDrop/4
        hmap[ y1, x1 ] += toDrop/4
        return hmap
    
    def erode(self, toTake, oldPos, hmap):
        x = np.round(oldPos[0])
        y = np.round(oldPos[1])
        for i in range(0, len(cROI)):
            X = cROI[i][0]
            Y = cROI[i][1]
            W = weights[i]
            try:
                hmap[x + X, y + Y] -= W * toTake
            except:
                pass
        return hmap

    def update(self, hmap):
        if self.pos[0] < 1 or self.pos[0] > self.mapSize - 1 or self.pos[1] < 1 or self.pos[1] > self.mapSize - 1:
            return hmap, False
        
        gradient = self.localGradient(hmap)
        self.dir = self.dir * self.inr - gradient * (1- self.inr)
        length = np.sqrt(np.sum(self.dir**2))
        if length == 0:
            self.dir = randUnitVec()
        else:
            self.dir /= length
            
        oldPos = self.pos
        self.pos += self.dir
        
        
        prevH = self.h
        self.h = self.localHeight(hmap)
        hDiff = self.h - prevH
        if hDiff > 0:
            toDrop = min(hDiff, self.sed)
            hmap = self.deposit(toDrop, oldPos, hmap)
            self.sed -= toDrop
        else:
            self.CAPACITY = max(-hDiff, self.MINSLOPE) * self.vel * self.vol * self.CAPACITY
            if self.CAPACITY > self.sed:
                # take
                toTake = min((self.CAPACITY-self.sed)*self.EROSION, -hDiff)
                hmap = self.erode(toTake, oldPos, hmap)
                self.sed += toTake
            else:
                # drop
                toDrop = (self.sed - self.CAPACITY) * self.DEPOSITION
                hmap = self.deposit(toDrop, oldPos, hmap)
                self.sed -= toDrop
                
            
        self.vel = np.sqrt(max(self.vel**2 + hDiff*self.GRAVITY, 0))
        self.vol = self.vol * (1 - self.EVAPORATION)
        self.STEPS += 1
        if self.STEPS >= self.MAXSTEPS:
            return hmap, False
        return hmap, True

def randUnitVec():
    _th = np.pi * uniform(0, 2)
    x = np.cos(_th)
    y = np.sin(_th)
    return np.asarray([x, y])

def ErodeMap(hmap, nDroplets, velocity, volume, inertia):
    copyMap = copy.deepcopy(hmap)
    N = len(hmap)
    for i in range(0, nDroplets):
        if i%100 == 0:
            print(i)
        x = uniform(0, 1) * N
        y = uniform(0, 1) * N
        _pos = np.asarray([x, y])
        _dir = randUnitVec()
        D = droplet(copyMap, position = _pos, direction = _dir, velocity=velocity, volume=volume, inertia=inertia)
        running = True
        while running:
            copyMap, running = D.update(copyMap) 
    return copyMap
#ExportObj()
changeMap = np.zeros((N, N))
#%%

newMap = ErodeMap(Map[:,:,1], 1000, velocity=1, volume=10, inertia=0.6)
changeMap += (Map[:,:,1] - newMap)
#plt.subplot(1,3,1)
#plt.imshow(Map[:,:,1], cmap = 'gray')
plt.subplot(1,2,1)
plt.imshow(newMap, cmap = 'gray' )
plt.subplot(1,2,2)
plt.imshow(-changeMap,cmap = 'gray')
Map[:,:,1] = newMap
ExportObj()

#%%

x = list()
y = list()
for i in range(0, 100):
    _x = uniform(-0.5, 0.5);
    _y = uniform(-0.5, 0.5);
    l = np.sqrt(_x**2 + _y**2)
    plt.plot(_x, _y, marker='o')