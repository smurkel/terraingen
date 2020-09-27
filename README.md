# waterphysics
c++/OpenGL ocean simulation and rendering (+ rigid body interaction)

This project is me trying to learn c++ through writing an ocean simulator. Currently, the simulation of the ocean surface itself is done (based on 'Simulating ocean water' by Jerry Tessendorf, and various other online sources, amongst which 'Realtime GPGPU FFT water simulation' by Fynn-Jorin Flügge) - and I am now working on a rigid body physics simulator that interacts with the water surface.

An executable demo file can be found in the OceanSimDemo folder. To be able to run it, download the whole folder as a .zip, extract, and run Sandbox.exe. A window opens in which the simulation runs. Controls are:
- RMB/MMB & move mouse        move camera
- scroll MMB / touchpad       zoom in or out
- I/J/K/L                     increase wind velocity in the corresponding ijkl/wasd direction  
- Q/E                         in- decrease wave amplitude
- spacebar                    pause

The water surface is rendered by combining various water colour variables: an
ambient colour forms the base colour, and specular & diffuse reflections, and 
an emissive component add detail to the image. These colours can also be changed
from within the simulator.
