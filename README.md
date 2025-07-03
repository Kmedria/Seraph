Seraph - ReadMe

Seraph is a 3D Game Engine, that is being developed by me, using C++, Vulkan and other online resources such as videos from TheCherno, Javidx9 and 3Blue1Brown.

Seraph currently has:
- A Memory Arena (free list allocater that manages a block of memeory stored in one large array, managed by linked lists).
- A basic 3D renderer (Takes data in form of Vertices and indexes, and uses shaders + Vulkan to simulate a 3D view onto a 2D screen. The renderer requires all data to be in triangle format. )
- A basic Shader that shades based on normal proximity (if the face of the object is facing you, it is brighter, if it facing away, it is darker.)
- Basic Mesh + object structure to store/manipulate grouped vertices and indexes more easily.

Seraph Eventually needs:
- Scripting (For when the game engine can be used to create games.)
- Entity component system (To better manage and control entity data).
- Physics (To simulate better object interactions.)
- Better/more sophisticated shading (Looks nicer).
- Scenes (management of grouped entities, and the ability to save/load scenes.)
- the ability to edit while running the engine (will greatly assist in debugging).

Currently I am:
- Intergrating Imgui for window managemnet + in-run editing.

Inline-style: 
![alt text](https://github.com/Kmedria/Seraph/blob/main/shaded%20teapot..png "Seraph Engine shaded teapot")

The image above

Here's our logo (hover to see the title text):

Inline-style: 
![alt text](https://github.com/adam-p/markdown-here/raw/master/src/common/images/icon48.png "Logo Title Text 1")

Reference-style: 
![alt text][logo]

[logo]: https://github.com/adam-p/markdown-here/raw/master/src/common/images/icon48.png "Logo Title Text 2"
