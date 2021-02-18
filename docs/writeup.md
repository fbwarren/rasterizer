# <center> **CS 184: Computer Graphics and Imaging, Spring 2021** <!-- omit in toc -->

## <center> **Project 1: Rasterizer** <!-- omit in toc -->

## <center> **Frank Warren** </center> <!-- omit in toc -->

## **Contents** <!-- omit in toc -->

- [**Overview**](#overview)
- [**Section I: Rasterization**](#section-i-rasterization)
  - [**Part 1: Rasterizing single-color triangles**](#part-1-rasterizing-single-color-triangles)
    - [**Improving speed**](#improving-speed)
  - [**Part 2: Antialiasing triangles**](#part-2-antialiasing-triangles)
- [**Section II: Sampling**](#section-ii-sampling)
  - [**Part 3: Transforms**](#part-3-transforms)
  - [**Part 4: Barycentric coordinates**](#part-4-barycentric-coordinates)
  - [**Part 5: "Pixel sampling" for texture mapping**](#part-5-pixel-sampling-for-texture-mapping)
  - [**Part 6 "Level sampling" with mipmaps for texture mapping**](#part-6-level-sampling-with-mipmaps-for-texture-mapping)
- [**Section III: Art Competition**](#section-iii-art-competition)

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

#### **Improving speed**

There are a few ways of improving the speed of the rasterization.  
It's not very efficient to sample every single pixel: consider the case of a very small triangle that's rasterized for a very large amount of pixels.  
The simplest way to cut down on the number of samples is to simply find a rectangle that the triangle is bounded by, and only sample pixels in the rectangle.  
Intuitively, we know that all the points inside a triangle won't have $x$/$y$ values less than/greater than the minimum/maximum $x$/$y$ values of its vertices. So, we can define a rectangle by the minimum and maximum coordinate values of the triangle, and only sample from that rectangle.  

### **Part 2: Antialiasing triangles**

Jagged edges on polygons are the result of [aliasing](https://en.wikipedia.org/wiki/aliasing). In order to smooth these "jaggies", we need to implement some form of anti-aliasing.  
If you are trying to represent some image on a screen, the solution from task 1 was to just take a sample of each pixel and if the pixel was inside the polygon, it was displayed. How can we display a more accurate image without simply adding more pixels?  
Well, even if you don't have more pixels, you can still sample more! If you take more than one sample per pixel, you can average them to have a more accurate value for your pixels! This is called [supersampling](https://en.wikipedia.org/wiki/Supersampling).


## **Section II: Sampling**

### **Part 3: Transforms**

### **Part 4: Barycentric coordinates**

### **Part 5: "Pixel sampling" for texture mapping**

### **Part 6 "Level sampling" with mipmaps for texture mapping**

## **Section III: Art Competition**
