# <center> **CS 184: Computer Graphics and Imaging, Spring 2021** <!-- omit in toc -->

## <center> **Project 1: Rasterizer** <!-- omit in toc -->

## <center> **Frank Warren** </center> <!-- omit in toc -->

## **Contents** <!-- omit in toc -->

- [**Overview**](#overview)
- [**Section I: Rasterization**](#section-i-rasterization)
  - [**Part 1: Rasterizing single-color triangles**](#part-1-rasterizing-single-color-triangles)
  - [**Part 2: Antialiasing triangles**](#part-2-antialiasing-triangles)
- [**Section II: Sampling**](#section-ii-sampling)
  - [**Part 3: Transforms**](#part-3-transforms)
  - [**Part 4: Barycentric coordinates**](#part-4-barycentric-coordinates)
  - [**Part 5: "Pixel sampling" for texture mapping**](#part-5-pixel-sampling-for-texture-mapping)
    - [**Nearest Neighbor**](#nearest-neighbor)
    - [**Bilinear**](#bilinear)
  - [**Part 6 "Level sampling" with mipmaps for texture mapping**](#part-6-level-sampling-with-mipmaps-for-texture-mapping)

## **Overview**

The purpose of this project was to implement a simple rasterizer that can handle simplified [Scalable Vector Graphics (SVG)](https://en.wikipedia.org/wiki/Scalable_Vector_Graphics) files.  
The bulk of the work in this project is to implement several features such as [supersampling](https://en.wikipedia.org/wiki/Supersampling), [matrix transforms](https://en.wikipedia.org/wiki/Transformation_matrix#Examples_in_2_dimensions), [barycentric coordinates](https://en.wikipedia.org/wiki/Barycentric_coordinate_system), pixel sampling, and [level sampling with mipmaps](https://en.wikipedia.org/wiki/Mipmap).  
I completed this project alone.

## **Section I: Rasterization**
A [rasterizer](https://en.wikipedia.org/wiki/Rasterisation) is a program that an image described in vector graphics and converts it into a series of pixels that can be displayed on a screen.  
After this project is built, the executable for the rasterizer can be ran with:  ```./draw [path to svg file/folder to render```  
When the executable is run, a couple things happen:  

1. An **SVGParser** (```svgparser.h/cpp```) reads the input ```.svg``` files.
2. An **OpenGL viewer** containing a **DrawRend** (```drawrend.h/cpp```) renderer is launched.
3. The viewer waits for mouse/keyboard input in an infinite loop.
4. ```DrawRend::redraw()``` performs high-level drawing work through the various **SVGElement** child classes, which pass their low-level shape rasterization data to appropraite methods in the **Rasterizer** class.



### **Part 1: Rasterizing single-color triangles**

The first task in the assignment was to implement triangle rasterization.  
The simplest way to do this is to simply check whether or not each pixel is inside or outside of the triangle. This is called sampling.  
In this project, each ```(x,y)``` pixel's sample point is in its center: ```(x+.5, y+.5)```. This isn't that important right now, it's just the convention that was chosen.  
Another convention that is followed in this project is that a sample that is directly on a triangle boundary will be drawn by the rasterizer.  
Luckily, a formula for determining if a point is inside a triangle is already well known:  
$$ \text{Consider some triangle defined by the points } $$
$$ A = (x_0, y_0) \\ B = (x_1, y_1) \\ C = (x_2, y_2) $$ 
$$ \text{A point } P = (x,y) \text{ is inside the triangle if the vectors} $$
$$ \vec{AB} \times \vec{AP},  \vec{AB} \times \vec{AP} \text{ and } \vec{AB} \times \vec{AP}$$
$$ \text{all point in the same direction relative to the plane.}$$
$$ \text{(when the z-component of each vector has the same sign)} $$

The intuition for this comes from the right hand rule. Notice that the z-component of the cross products only have to have the same sign; an intuitive way to think of this is that if you traverse a triangle in a counterclockwise direction, a point inside the triangle will appear to be to the left of the edges. If you traverse a triangle the otherway, an inner point will appear to be to the right of the edges.

#### **Improving speed**  <!-- omit in toc -->

There are a few ways of improving the speed of the rasterization.  
It's not very efficient to sample every single pixel: consider the case of a very small triangle that's rasterized for a very large amount of pixels.  
The simplest way to cut down on the number of samples is to simply find a rectangle that the triangle is bounded by, and only sample pixels in the rectangle.  
Intuitively, we know that all the points inside a triangle won't have $x$/$y$ values less than/greater than the minimum/maximum $x$/$y$ values of its vertices. So, we can define a rectangle by the minimum and maximum coordinate values of the triangle, and only sample from that rectangle.  

### **Part 2: Antialiasing triangles**

Jagged edges on polygons are the result of [aliasing](https://en.wikipedia.org/wiki/aliasing). In order to smooth these "jaggies", we need to implement some form of anti-aliasing.  
If you are trying to represent some image on a screen, the solution from task 1 was to just take a sample of each pixel and if the pixel was inside the polygon, it was displayed. How can we display a more accurate image without simply adding more pixels?  
Well, even if you don't have more pixels, you can still sample more! If you take more than one sample per pixel, you can average them to have a more accurate value for your pixels! This is called [supersampling](https://en.wikipedia.org/wiki/Supersampling).  

#### **Implementation** <!-- omit in toc -->

In this project, super smapling was achieved through by rasterizing a higher resolution image to a (larger) sample buffer, then downsampling from the sample buffer to the display buffer. The sample buffer is a vector that holds `width * height * sample_rate` Color objects. The frame buffer holds `3 * width * height` RGB values, since each pixel has a red, green, and blue value. Basically, when any `rasterize` function is called, it will sample points determined by the `sample_rate` and populate the sample buffer with the samples. Then, `resolve_to_framebuffer` is called to downsample from `sample_buffer` to the `frame_buffer` by averaging the `sample_rate` number of supersamples.

Here, you can see how taking more samples and averaging the results helps remove aliasing.

| ![](./images/task2a.png) | ![](./images/task2b.png) | ![](./images/task2c.png) |
|:--:|:--:|:--:|
| *1x sampling* | *4x supersampling* | *16x supersampling* |

## **Section II: Sampling**

### **Part 3: Transforms**

Part of this assignment was to implement matrix transforms. These included rotations, translations, and scaling. I used these transformations to edit an SVG image of a robot to make him more human. I gave him a tie, some slacks, and a hat. Also, he's waving at you!  

| <img src="./images/robot.jpg" width=500> | ![](./images/robot.svg) |
|:--:|:--:|
| *Before* | *After* |  
  

### **Part 4: Barycentric coordinates**

[Barycentric coordinates](https://en.wikipedia.org/wiki/Barycentric_coordinate_system) are coordinates that are defined in relation to the vertices of a triangle.  
Basically, the idea is that you specify a location based on how far it is from each vertex. If you give each vertex some weight or value, you can think of barycentric coordinates as representing the "contribution" each vertex has to the overall value of the point. Formally, it's defined like this:  

$$
\text{For a triangle } ABC, \\
$$
$$
\text{$P = (\alpha, \beta, \gamma)$} \\
\alpha + \beta + \gamma = 1 \\
P = \alpha A + \beta B + \gamma C
$$

The pictures below give a visual idea of how points on a triangle can be thought of as the sum of each vertex's "contribution.  
Both the shapes have a clear red, green, or blue region. Using barycentric coordinates, the surface of the shapes were able to be colored by a process called linear interpolation, which resulted in a smooth transition between the colors. 

| <img src="./images/interptri.png"> | ![](./images/interpcirc.png) |
|:--:|:--:|
| *Color interpolated triangle* | *Color interpolated circle* |  

### **Part 5: "Pixel sampling" for texture mapping**

So far in the project, we have been drawing shapes and coloring them with colors specified either in source code, or in the SVG files. However, what if we drew the inside of a house? Would it make sense to draw and define the color values of all the walls, or all the floor tiles?  No! All the walls usually look like each other, and all the floor tiles look like each other. So, it's easier and more efficient to have a general image of what those things are supposed to look like, and then **sample** from those images! This is called **texture mapping**!  

There's various ways to actually implement texture mapping. The two ways were were asked to implement texture mapping was *nearest neighbor* and *bilinear*.

#### **Nearest Neighbor**

Afer determining the texture coordinates $(u,v)$ by barycentric coordinates, we just scale the coordinates by the texture image dimensions, then round to the nearest pixel in the texture.

#### **Bilinear**

Bilinear filtering basically takes the four closest pixels to the $(u,v)$ and determines a weighted average based on the position of $(u,v)$ relative to the four pixels.

| <img src="./images/nearest1.png"> | ![](./images/nearest16.png) |
|:--:|:--:|
| *Nearest neighbor, 1x* | *Nearest neighbor, 16x* |  

| <img src="./images/bi1.png"> | ![](./images/bi16.png) |
|:--:|:--:|
| *Bilinear, 1x* | *Bilinear, 16x* |

It's a bit subtl, but you can see that there is less noise when using bilinear filtering. Fine features like hair, or feathers in this case, blend more nicely. Bilinear filtering generally helps smooth out high frequency details.

### **Part 6 "Level sampling" with mipmaps for texture mapping**



