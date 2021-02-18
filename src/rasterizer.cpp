#include "rasterizer.h"

using namespace std;

namespace CGL {

    RasterizerImp::RasterizerImp(PixelSampleMethod psm, LevelSampleMethod lsm,
                                 size_t width, size_t height,
                                 unsigned int sample_rate) {
        this->psm = psm;
        this->lsm = lsm;
        this->width = width;
        this->height = height;
        this->sample_rate = sample_rate;

        sample_buffer.resize(width * height * sample_rate, Color::White);
    }

    // Used by rasterize_point and rasterize_line
    void RasterizerImp::fill_pixel(size_t x, size_t y, Color c) {
        // TODO: Task 2: You might need to this function to fix points and lines (such as the black rectangle border in test4.svg)
        // NOTE: You are not required to implement proper supersampling for points and lines
        // It is sufficient to use the same color for all supersamples of a pixel for points and lines (not triangles)
        sample_buffer[y * width + x] = c;
    }

    // Rasterize a point: simple example to help you start familiarizing
    // yourself with the starter code.
    //
    void RasterizerImp::rasterize_point(float x, float y, Color color) {
        // fill in the nearest pixel
        int sx = (int)floor(x);
        int sy = (int)floor(y);
        // check bounds
        if (sx < 0 || sx >= width) return;
        if (sy < 0 || sy >= height) return;

        fill_pixel(sx, sy, color);
        return;
    }

    // Rasterize a line.
    void RasterizerImp::rasterize_line(float x0, float y0,
                                       float x1, float y1,
                                       Color color) {
        if (x0 > x1) {
            swap(x0, x1); swap(y0, y1);
        }

        float pt[] = { x0,y0 };
        float m = (y1 - y0) / (x1 - x0);
        float dpt[] = { 1,m };
        int steep = abs(m) > 1;
        if (steep) {
            dpt[0] = x1 == x0 ? 0 : 1 / abs(m);
            dpt[1] = x1 == x0 ? (y1 - y0) / abs(y1 - y0) : m / abs(m);
        }

        while (floor(pt[0]) <= floor(x1) && abs(pt[1] - y0) <= abs(y1 - y0)) {
            rasterize_point(pt[0], pt[1], color);
            pt[0] += dpt[0]; pt[1] += dpt[1];
        }
    }

    // Rasterize a triangle.
    void RasterizerImp::rasterize_triangle(float x0, float y0,
                                           float x1, float y1,
                                           float x2, float y2,
                                           Color color) {
        // TODO: Task 1: Implement basic triangle rasterization here, no supersampling
        // We only want to sample pixels that are inside of the rectangle that the triangle is bounded by
        float xmin, xmax, ymin, ymax;
        xmin = floor(min({x0, x1, x2}));
        xmax = ceil(max({x0, x1, x2}));
        ymin = floor(min({y0, y1, y2}));
        ymax = ceil(max({y0, y1, y2}));

        // Use the line equation for each sample
        for (int x = xmin; x < xmax; x++) {
            for (int y = ymin; y < ymax; y++) {
                float l0 = lineEquation(x, y, x0, y0, x1, y1);
                float l1 = lineEquation(x, y, x1, y1, x2, y2);
                float l2 = lineEquation(x, y, x2, y2, x0, y0);

                // If the line equation result is + for all lines or - for all lines, then we know that the
                // sample point is inside (bounded by) a triangle
                if (l0 >= 0.0 && l1 >= 0.0 && l2 >= 0.0 || l0 <= 0.0 && l1 <= 0.0 && l2 <= 0.0)
                    { sample_buffer[y * width + x] = color; }
            }
        }
        // TODO: Task 2: Update to implement super-sampled rasterization
        return;
    }

    void RasterizerImp::rasterize_interpolated_color_triangle(float x0, float y0, Color c0,
                                                              float x1, float y1, Color c1,
                                                              float x2, float y2, Color c2)
    {
        // TODO: Task 4: Rasterize the triangle, calculating barycentric coordinates and using them to interpolate vertex colors across the triangle
        // Hint: You can reuse code from rasterize_triangle
        float xmin, xmax, ymin, ymax;
        float bCoords[3];
        xmin = floor(min({x0, x1, x2}));
        xmax = ceil(max({x0, x1, x2}));
        ymin = floor(min({y0, y1, y2}));
        ymax = ceil(max({y0, y1, y2}));

        // Use the line equation for each sample
        for (int x = xmin; x < xmax; x++) {
            for (int y = ymin; y < ymax; y++) {
                fill_n(bCoords, 3, 0);
                barycentricCoord(x, y, x0, y0, x1, y1, x2, y2, bCoords);
                float l0 = lineEquation(x, y, x0, y0, x1, y1);
                float l1 = lineEquation(x, y, x1, y1, x2, y2);
                float l2 = lineEquation(x, y, x2, y2, x0, y0);
                // If the line equation result is + for all lines or - for all lines, then we know that the
                // sample point is inside (bounded by) a triangle
                if (l0 >= 0.0 && l1 >= 0.0 && l2 >= 0.0 || l0 <= 0.0 && l1 <= 0.0 && l2 <= 0.0) {
                    sample_buffer[y * width + x] = (bCoords[0] * c0) + (bCoords[1] * c1) + (bCoords[2] * c2);
                }
            }
        }
    }

    void RasterizerImp::rasterize_textured_triangle(float x0, float y0, float u0, float v0,
                                                    float x1, float y1, float u1, float v1,
                                                    float x2, float y2, float u2, float v2,
                                                    Texture& tex)
    {
        // TODO: Task 5: Fill in the SampleParams struct and pass it to the tex.sample function.
        // TODO: Task 6: Set the correct barycentric differentials in the SampleParams struct.
        // Hint: You can reuse code from rasterize_triangle/rasterize_interpolated_color_triangle




    }

    void RasterizerImp::set_sample_rate(unsigned int rate) {
        // TODO: Task 2: You may want to update this function for supersampling support

        this->sample_rate = rate;
        this->sample_buffer.resize(width * height * sample_rate, Color::White);
    }

    void RasterizerImp::set_framebuffer_target(unsigned char* rgb_framebuffer,
                                               size_t width, size_t height)
    {
        // TODO: Task 2: You may want to update this function for supersampling support
        this->width = width;
        this->height = height;
        this->rgb_framebuffer_target = rgb_framebuffer;

        this->sample_buffer.resize(width * height * sample_rate, Color::White);
    }

    void RasterizerImp::clear_buffers() {
        std::fill(rgb_framebuffer_target, rgb_framebuffer_target + 3 * width * height, 255);
        std::fill(sample_buffer.begin(), sample_buffer.end(), Color::White);
    }


    // This function is called at the end of rasterizing all elements of the
    // SVG file.  If you use a supersample buffer to rasterize SVG elements
    // for antialising, you could use this call to fill the target framebuffer
    // pixels from the supersample buffer data.
    //
    void RasterizerImp::resolve_to_framebuffer() {
        // TODO: Task 2: You will likely want to update this function for supersampling support
        int rate = sqrt(sample_rate);
        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < height; ++y) {
                for (int sx = 0; sx < rate; ++sx) {
                    for (int sy = 0; sy < rate; ++sy) {
                        Color col = sample_buffer[y*rate*rate*width + x*rate + sx + sy*rate*width] * (1.0/sample_rate);
                        for (int k = 0; k < 3; ++k) {
                            // Add each (weighted) supersample to the pixel it belongs to
                            this->rgb_framebuffer_target[3 * (y * width + x) + k] = (&col.r)[k] * 255;
                        }
                    }
                }
            }
        }
    }

    // Line equation helper
    // Finds the magnitude of a normal formed between a point (x, y) and a line formed by the other args.
    float RasterizerImp::lineEquation(int x, int y, float x0, float y0, float x1, float y1) {
        return -(x-x0 + 0.5) * (y1-y0) + (y-y0 + 0.5) * (x1-x0);
    }

    // Barycentric coordinate helper
    // Doesn't assume that the point is in your triangle!
    // The coordinates will go into the coords array
    void RasterizerImp::barycentricCoord(int x, int y, float x0, float y0, float x1, float y1, float x2, float y2, float *coords) {
        float l1 = lineEquation(x, y, x1, y1, x2, y2);
        float l2 = lineEquation(x, y, x2, y2, x0, y0);
        coords[0] = (l1 / lineEquation(x0, y0, x1, y1, x2, y2));
        coords[1] = (l2 / lineEquation(x1, y1, x2, y2, x0, y0));
        coords[2] = 1 - coords[0] - coords[1];
    }

    Rasterizer::~Rasterizer() { }
}// CGL
