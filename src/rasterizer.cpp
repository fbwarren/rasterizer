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
        
        // NOTE: You are not required to implement proper supersampling for points and lines
        // It is sufficient to use the same color for all supersamples of a pixel for points and lines (not triangles)
        int start = y * width * sample_rate + x * sqrt(sample_rate);
        for (int col = 0; col < sqrt(sample_rate); ++col) {
            for (int row = 0; row < sqrt(sample_rate); ++row) {
                sample_buffer[start + row*sample_rate*width + col] = c;
            }
        }
    }

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
        float xmin, xmax, ymin, ymax;
        int rate = sqrt(sample_rate);
        // scale the triangle
        x0 *= rate;
        x1 *= rate;
        x2 *= rate;
        y0 *= rate;
        y1 *= rate;
        y2 *= rate;
        // bounding box
        xmin = floor(min({x0, x1, x2}));
        xmax = ceil(max({x0, x1, x2}));
        ymin = floor(min({y0, y1, y2}));
        ymax = ceil(max({y0, y1, y2}));

        // Use the line equation for each sample
        for (int x = xmin; x < xmax; x++) {
            for (int y = ymin; y < ymax; y++) {
                if (x < 0 || y < 0 || x >= width * rate || y >= height * rate) continue; // bound check
                float l0 = lineEquation(x+0.5, y+0.5, x0, y0, x1, y1);
                float l1 = lineEquation(x+0.5, y+0.5, x1, y1, x2, y2);
                float l2 = lineEquation(x+0.5, y+0.5, x2, y2, x0, y0);
                // If the line equation result is + for all lines or - for all lines, then we know that the
                // sample point is inside (bounded by) a triangle
                if (l0 > 0.0 && l1 > 0.0 && l2 > 0.0 || l0 < 0.0 && l1 < 0.0 && l2 < 0.0)
                    { sample_buffer[y * width * rate + x] = color; }
            }
        }
        return;
    }

    void RasterizerImp::rasterize_interpolated_color_triangle(float x0, float y0, Color c0,
                                                              float x1, float y1, Color c1,
                                                              float x2, float y2, Color c2)
    {
        float xmin, xmax, ymin, ymax;
        float bCoords[3];
        int rate = sqrt(sample_rate);

        // Scale the triangle
        x0 *= rate;
        x1 *= rate;
        x2 *= rate;
        y0 *= rate;
        y1 *= rate;
        y2 *= rate;

        // Bounding box
        xmin = floor(min({x0, x1, x2}));
        xmax = ceil(max({x0, x1, x2}));
        ymin = floor(min({y0, y1, y2}));
        ymax = ceil(max({y0, y1, y2}));

        for (int x = xmin; x < xmax; x++) {
            for (int y = ymin; y < ymax; y++) {
                if (x < 0 || y < 0 || x >= width * rate || y >= height * rate) continue; // bound check
                fill_n(bCoords, 3, 0);
                barycentricCoord(x+0.5, y+0.5, x0, y0, x1, y1, x2, y2, bCoords);
                float l0 = lineEquation(x+0.5, y+0.5, x0, y0, x1, y1);
                float l1 = lineEquation(x+0.5, y+0.5, x1, y1, x2, y2);
                float l2 = lineEquation(x+0.5, y+0.5, x2, y2, x0, y0);
                if (l0 >= 0.0 && l1 >= 0.0 && l2 >= 0.0 || l0 <= 0.0 && l1 <= 0.0 && l2 <= 0.0) {
                    sample_buffer[y * width * rate + x] = (bCoords[0] * c0) + (bCoords[1] * c1) + (bCoords[2] * c2);
                }
            }
        }
    }

    void RasterizerImp::rasterize_textured_triangle(float x0, float y0, float u0, float v0,
                                                    float x1, float y1, float u1, float v1,
                                                    float x2, float y2, float u2, float v2,
                                                    Texture& tex)
    {
        int rate = sqrt(sample_rate);
        float xmin, xmax, ymin, ymax;
        float bCoords[3];
        SampleParams sample;
        sample.lsm = lsm;
        sample.psm = psm;

        // scale the triangle
        x0 *= rate;
        x1 *= rate;
        x2 *= rate;
        y0 *= rate;
        y1 *= rate;
        y2 *= rate;

        // bounding box
        xmin = floor(min({x0, x1, x2}));
        xmax = ceil(max({x0, x1, x2}));
        ymin = floor(min({y0, y1, y2}));
        ymax = ceil(max({y0, y1, y2}));

        for (int x = xmin; x < xmax; x++) {
            for (int y = ymin; y < ymax; y++) {
                if (x < 0 || y < 0 || x >= width * rate || y >= height * rate) continue; // bound check
                fill_n(bCoords, 3, 0);

                barycentricCoord(x+0.5, y+0.5, x0, y0, x1, y1, x2, y2, bCoords);
                sample.p_uv = Vector2D(u0, v0) * bCoords[0] + Vector2D(u1, v1) * bCoords[1] + Vector2D(u2, v2) * bCoords[2];
                barycentricCoord(x+1.5, y+0.5, x0, y0, x1, y1, x2, y2, bCoords);
                sample.p_dx_uv = Vector2D(u0, v0) * bCoords[0] + Vector2D(u1, v1) * bCoords[1] + Vector2D(u2, v2) * bCoords[2];
                barycentricCoord(x+0.5, y+1.5, x0, y0, x1, y1, x2, y2, bCoords);
                sample.p_dy_uv = Vector2D(u0, v0) * bCoords[0] + Vector2D(u1, v1) * bCoords[1] + Vector2D(u2, v2) * bCoords[2];

                float l0 = lineEquation(x+0.5, y+0.5, x0, y0, x1, y1);
                float l1 = lineEquation(x+0.5, y+0.5, x1, y1, x2, y2);
                float l2 = lineEquation(x+0.5, y+0.5, x2, y2, x0, y0);

                if (l0 >= 0.0 && l1 >= 0.0 && l2 >= 0.0 || l0 <= 0.0 && l1 <= 0.0 && l2 <= 0.0) {
                        sample_buffer[y * width * sqrt(sample_rate) + x] = tex.sample(sample);
                }
            }
        }
    }

    void RasterizerImp::set_sample_rate(unsigned int rate) {
        this->sample_rate = rate;
        this->sample_buffer.resize(width * height * sample_rate, Color::White);
    }

    void RasterizerImp::set_framebuffer_target(unsigned char* rgb_framebuffer,
                                               size_t width, size_t height)
    {
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
        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < height; ++y) {
                Color col = averagePixels(x, y);
                for (int k = 0; k < 3; ++k) {
                    // Add each (weighted) supersample to the pixel it belongs to
                    this->rgb_framebuffer_target[3 * (y * width + x) + k] = (&col.r)[k] * 255;
                }
            }
        }
    }

    // This function returns a color that is the average of the supersamples for the pixel (x,y)
    // The sample_buffer is in row order
    Color RasterizerImp::averagePixels(int x, int y){
        int start = y * width * sample_rate + x * sqrt(sample_rate);
        Color color = Color();
        for (int col = 0; col < sqrt(sample_rate); ++col) {
            for (int row = 0; row < sqrt(sample_rate); ++row) {
                color += sample_buffer[start + row * sqrt(sample_rate) * width + col] * (1.0 / sample_rate);
            }
        }
        return color;
    }

    // Line equation helper
    // Finds the magnitude of a normal formed between a point (x, y) and a line formed by the other args.
    float RasterizerImp::lineEquation(float x, float y, float x0, float y0, float x1, float y1) {
        return -(x-x0) * (y1-y0) + (y-y0) * (x1-x0);
    }

    // Barycentric coordinate helper
    // Doesn't assume that the point is in your triangle!
    // The coordinates will go into the coords array
    void RasterizerImp::barycentricCoord(float x, float y, float x0, float y0, float x1, float y1, float x2, float y2, float *coords) {
        float l1 = lineEquation(x, y, x1, y1, x2, y2);
        float l2 = lineEquation(x, y, x2, y2, x0, y0);
        coords[0] = (l1 / lineEquation(x0, y0, x1, y1, x2, y2));
        coords[1] = (l2 / lineEquation(x1, y1, x2, y2, x0, y0));
        coords[2] = 1 - coords[0] - coords[1];
    }

    Rasterizer::~Rasterizer() { }
}// CGL
