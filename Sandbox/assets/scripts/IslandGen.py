# -*- coding: utf-8 -*-
"""
Created on Mon Sep 21 13:29:46 2020

@author: Mart
"""


import noise
import numpy as np
from scipy.spatial import Voronoi, voronoi_plot_2d
import matplotlib.pyplot as plt
import PIL.ImageDraw as ImageDraw
import PIL.Image as Image

N = 250;
L = 50;

points = np.random.uniform(0, L, (N, 2))


def CentroidRelaxation(VoronoiDiagram, iterations):
    def RelaxIter(vor):
        polygons = list()
        for region in vor.regions:
            closedPoly = True
            if region == []:
                closedPoly = False
            for idx in region:
                if idx < 0:
                    closedPoly = False
                    break
            if closedPoly:
                polygons.append(region)
        
        centroids = list()
        for poly in polygons:
            Centroid = np.asarray([0.0, 0.0])
            VertexCount = 0
            for vertex in poly:
                VertexCount += 1
                Centroid += vor.vertices[vertex]
            Centroid /= VertexCount
            if Centroid[0] > 0 and Centroid[0] < L and Centroid[1] > 0 and Centroid[1] < L:
                centroids.append(Centroid)
        centroids = np.asarray(centroids)
        return Voronoi(centroids)
    
    
    for i in range(0, iterations):
        VoronoiDiagram = RelaxIter(VoronoiDiagram)    
        
    return VoronoiDiagram


vor = CentroidRelaxation(Voronoi(points), 2)
            
class Mesh:
    def __init__(self, voronoi):
        self.Centroids = voronoi.points
        self.Polygons = voronoi.regions[1:]
        i = 0
        while i < len(self.Polygons):
            if self.Polygons[i] == []:
                self.Polygons.pop(i)
            i += 1
        self.Vertices = voronoi.vertices
        self.p_Count = len(self.Polygons)
        self.p_Color = list()
        for i in range(0, self.p_Count):
            self.p_Color.append((0,0,0))
        self.v_Count = len(self.Vertices)
        self.v_Type = list()
        for i in range(0, self.v_Count):
            self.v_Type.append(0)
        
    def render(self):
        image = Image.new("RGB", (500, 500))
        draw = ImageDraw.Draw(image)
        pIdx = 0
        for poly in self.Polygons:
            points = list()
            for idx in poly:
                if not idx == -1:
                    points.append(tuple(self.Vertices[idx]*10))
            draw.polygon(tuple(points), fill=self.p_Color[pIdx], outline = (255, 255, 255))
            pIdx += 1
        return image
    
    def setWater(self):
        for poly in self.Polygons:
            edge = False
            for idx in poly:
                if idx < 0:
                    edge = True
            if edge:
                for idx in poly:
                    self.v_Color[idx] = (30, 80, 140)
mesh = Mesh(vor)
mesh.setWater()
img = mesh.render()
plt.imshow(img)