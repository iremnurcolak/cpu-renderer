#include "triangle_renderer.h"
#include "line_renderer.h"
#include "colors.h"
void drawTriangleWithMyScanlineV1(int ax, int ay, int bx, int by, int cx, int cy,
    TGAImage& framebuffer,
    TGAColor color)
{
  // sort the vertices, a,b,c in ascending y order 
    if (ay>by) { std::swap(ax, bx); std::swap(ay, by); }
    if (ay>cy) { std::swap(ax, cx); std::swap(ay, cy); }
    if (by>cy) { std::swap(bx, cx); std::swap(by, cy); }
    
    for(int y = ay; y <= by; y++)
    {
        const float t1 = (float(y) - ay) / (float(by) - ay);
        float x1;
        if(t1 >= 0.0F && t1 <= 1.0F)
        {
            x1 = ax + t1 * (float(bx) - ax);
            // Kesişim: (x, a)
        }

        const float t2 = (float(y) - ay) / (float(cy) - ay);
        float x2;
        if(t2 >= 0.0F && t2 <= 1.0F)
        {
            x2 = ax + t2 * (float(cx) - ax);
            // Kesişim: (x, a)
        }

        drawLineInterpolated(x1, y, x2, y, framebuffer, color);
    }
    

    for(int y = by; y <= cy; y++)
    {
        const float t1 = (float(y) - by) / (float(cy) - by);
        float x1;
        if(t1 >= 0.0F && t1 <= 1.0F)
        {
            x1 = bx + t1 * (float(cx) - bx);
            // cross: (x, a)
        }

        const float t2 = (float(y) - ay) / (float(cy) - ay);
        float x2;
        if(t2 >= 0.0F && t2 <= 1.0F)
        {
            x2 = ax + t2 * (float(cx) - ax);
            // cross: (x, a)
        }

        drawLineInterpolated(x1, y, x2, y, framebuffer, color);
    }
}

void drawTriangleWithMyScanlineV2(int ax, int ay, int bx, int by, int cx, int cy,
    TGAImage& framebuffer,
    TGAColor color)
{
  // sort the vertices, a,b,c in ascending y order 
    if (ay>by) { std::swap(ax, bx); std::swap(ay, by); }
    if (ay>cy) { std::swap(ax, cx); std::swap(ay, cy); }
    if (by>cy) { std::swap(bx, cx); std::swap(by, cy); }

    if(ay != by)
    {
        for(int y = ay; y <= by; y++)
        {
            int x1 = ax + (((y - ay) * (bx - ax)) / (by - ay));
            // cross: (x1, a)

            int x2 = ax + (((y - ay) * (cx - ax)) / (cy - ay));
            // cross: (x2, a)

            //these two methods give different results
            //drawLineInterpolated(x1, y, x2, y, framebuffer, color);
            for (int x=std::min(x1,x2); x<std::max(x1,x2); x++)  // draw a horizontal line
                framebuffer.set(x, y, color);

        }
    }

    if(by != cy)
    {
        for(int y = by; y <= cy; y++)
        {
            int x1 = bx + (((y - by) * (cx - bx)) / (cy - by));
                // Kesişim: (x1, a)

            int x2 = ax + (((y - ay) * (cx - ax)) / (cy - ay));
                // Kesişim: (x2, a)

            //these two methods give different results
            //drawLineInterpolated(x1, y, x2, y, framebuffer, color);
            for (int x=std::min(x1,x2); x<std::max(x1,x2); x++)  // draw a horizontal line
                framebuffer.set(x, y, color);

        }

    }
}