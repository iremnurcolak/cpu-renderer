#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "colors.h"
#include "line_renderer.h"
#include "triangle_renderer.h"
#include "obj_renderer.h"
#include "tgaimage.h"

namespace
{
constexpr int framebufferWidth = 800;
constexpr int framebufferHeight = 800;

struct Vertex
{
    float x = 0.0F;
    float y = 0.0F;
    float z = 0.0F;
};

struct ScreenPoint
{
    int x;
    int y;
    double z;
};

using FaceDrawer = void (*)(const std::vector<ScreenPoint>&, TGAImage&, TGAImage&);

/**
 * @brief Draw a closed polygon boundary in white.
 * @pre points must contain at least three vertices.
 * @param[in] points Ordered projected face vertices.
 * @param[in,out] framebuffer Image receiving the face boundary.
 * @param[in,out] zbuffer Unused depth image; wireframe mode does not write depth.
 */
void drawWireframeFace(const std::vector<ScreenPoint>& points, TGAImage& framebuffer,
                       [[maybe_unused]] TGAImage& zbuffer)
{
    const ScreenPoint& first = points.front();
    ScreenPoint previous = first;
    for(std::size_t i = 1; i < points.size(); ++i)
    {
        const ScreenPoint& point = points[i];
        drawLineInterpolated(previous.x, previous.y, point.x, point.y, framebuffer, white);
        previous = point;
    }
    drawLineInterpolated(previous.x, previous.y, first.x, first.y, framebuffer, white);
}

/**
 * @brief Triangulate a face as a fan and fill each triangle with a random RGB color.
 * @pre points must describe a convex polygon with at least three vertices.
 * @note Back-facing triangles and triangles with signed area below 1 are skipped.
 * @param[in] points Ordered projected face vertices.
 * @param[in,out] framebuffer Image receiving the filled triangles.
 * @param[in,out] zbuffer Grayscale image receiving interpolated depth without visibility testing.
 */
void drawFilledFace(const std::vector<ScreenPoint>& points, TGAImage& framebuffer, TGAImage& zbuffer)
{
    static thread_local std::mt19937 randomEngine{std::random_device{}()};
    std::uniform_int_distribution<int> channel(0, 255);
    const ScreenPoint& first = points.front();
    for(std::size_t i = 1; i + 1 < points.size(); ++i)
    {
        const ScreenPoint& b = points[i];
        const ScreenPoint& c = points[i + 1];
        const TGAColor color = {
            static_cast<std::uint8_t>(channel(randomEngine)),
            static_cast<std::uint8_t>(channel(randomEngine)),
            static_cast<std::uint8_t>(channel(randomEngine)),
            255};
        drawTriangleWithBoundingBox(first.x, first.y, b.x, b.y, c.x, c.y,
            first.z, b.z, c.z, color, color, color, framebuffer, zbuffer);
    }
}

/**
 * @brief Read an OBJ model, project its faces, and save the rendered image.
 * @note Supports slash-separated and negative vertex indices.
 * @note Projects x/y from [-1, 1] onto an 800 x 800 framebuffer.
 * @return 0 on success; 1 if the input cannot be opened or output cannot be saved.
 * @param[in] objPath Explicit OBJ path or filename looked up under obj/.
 * @param[in] outputPath Destination TGA file path.
 * @param[in] drawFace Callback used to draw each valid projected face.
 * @param[in] zbufferPath Depth TGA output path; empty disables depth output.
 */
int renderObj(std::string_view objPath, std::string_view outputPath, FaceDrawer drawFace,
              std::string_view zbufferPath = {})
{
    std::filesystem::path modelPath{std::string(objPath)};
    if(!modelPath.has_parent_path())
    {
        const std::filesystem::path modelRoot{"obj"};
        const std::filesystem::path directPath = modelRoot / modelPath;
        const std::filesystem::path nestedPath = modelRoot / modelPath.stem() / modelPath;
        modelPath = std::filesystem::exists(directPath) ? directPath : nestedPath;
    }
    std::ifstream objFile{modelPath};
    if(!objFile)
    {
        std::cerr << "Failed to open OBJ file: " << modelPath.string() << '\n';
        return 1;
    }

    TGAImage framebuffer(framebufferWidth, framebufferHeight, TGAImage::RGB);
    TGAImage zbuffer = zbufferPath.empty() ? TGAImage{}
        : TGAImage(framebufferWidth, framebufferHeight, TGAImage::GRAYSCALE);
    std::vector<Vertex> vertices;
    std::size_t faceCount = 0;

    std::string line;
    while(std::getline(objFile, line))
    {
        if(line.empty())
        {
            continue;
        }

        const bool isVertex = line.size() > 1 && line.front() == 'v'
            && std::isspace(static_cast<unsigned char>(line[1]));
        const bool isFace = line.size() > 1 && line.front() == 'f'
            && std::isspace(static_cast<unsigned char>(line[1]));

        if(isVertex)
        {
            std::istringstream lineStream(line);
            char recordType = '\0';
            Vertex vertex;

            if(lineStream >> recordType >> vertex.x >> vertex.y >> vertex.z)
            {
                vertices.push_back(vertex);
            }
        }
        else if(isFace)
        {
            std::istringstream lineStream(line);
            char recordType = '\0';
            std::vector<std::size_t> faceVertexIndices;
            std::string element;

            lineStream >> recordType;
            while(lineStream >> element)
            {
                const std::size_t slashPosition = element.find('/');
                const std::string vertexIndexText = element.substr(0, slashPosition);

                std::istringstream indexStream(vertexIndexText);
                int objVertexIndex = 0;
                if(!(indexStream >> objVertexIndex) || objVertexIndex == 0)
                {
                    faceVertexIndices.clear();
                    break;
                }

                const int vertexIndex = objVertexIndex > 0
                    ? objVertexIndex - 1
                    : static_cast<int>(vertices.size()) + objVertexIndex;

                if(vertexIndex < 0
                    || vertexIndex >= static_cast<int>(vertices.size()))
                {
                    faceVertexIndices.clear();
                    break;
                }

                faceVertexIndices.push_back(static_cast<std::size_t>(vertexIndex));
            }

            if(faceVertexIndices.size() >= 3)
            {
                ++faceCount;

                constexpr float xScale = (framebufferWidth - 1) * 0.5F;
                constexpr float yScale = (framebufferHeight - 1) * 0.5F;
                std::vector<ScreenPoint> points;
                points.reserve(faceVertexIndices.size());
                for(const std::size_t index : faceVertexIndices)
                {
                    const Vertex& vertex = vertices[index];
                    points.push_back({
                        static_cast<int>((vertex.x + 1.0F) * xScale),
                        static_cast<int>((vertex.y + 1.0F) * yScale),
                        (static_cast<double>(vertex.z) + 1.0) * 127.5});
                }
                drawFace(points, framebuffer, zbuffer);
            }
        }
    }

    std::cout << "Loaded " << vertices.size() << " vertices and "
              << faceCount << " faces\n";

    const std::string outputFile(outputPath);
    if(!framebuffer.write_tga_file(outputFile))
    {
        std::cerr << "Failed to write " << outputPath << '\n';
        return 1;
    }

    if(!zbufferPath.empty() && !zbuffer.write_tga_file(std::string(zbufferPath)))
    {
        std::cerr << "Failed to write " << zbufferPath << '\n';
        return 1;
    }

    return 0;
}

}

/**
 * @brief Render OBJ face boundaries as white line segments.
 * @note Output is an 800 x 800 TGA image.
 * @note Model x/y coordinates map from [-1, 1] without automatic fitting.
 * @return 0 on success; 1 if the input cannot be opened or output cannot be saved.
 * @param[in] objPath OBJ path or filename resolved under obj/.
 * @param[in] outputPath Destination TGA file path.
 */
int ObjRenderer::renderWireframe(std::string_view objPath, std::string_view outputPath) const
{
    return renderObj(objPath, outputPath, drawWireframeFace);
}

/**
 * @brief Render OBJ faces as randomly colored filled triangles.
 * @pre Polygonal faces must be convex for fan triangulation.
 * @note Back-facing triangles and triangles with signed area below 1 are skipped.
 * @note A separate grayscale image maps model z from [-1, 1] to [0, 255].
 * @note No depth testing, lighting, or textures are applied.
 * @note Model x/y coordinates map from [-1, 1] without automatic fitting.
 * @return 0 on success; 1 if the input cannot be opened or output cannot be saved.
 * @param[in] objPath OBJ path or filename resolved under obj/.
 * @param[in] outputPath Destination TGA file path.
 * @param[in] zbufferPath Destination grayscale depth TGA; empty disables depth output.
 */
int ObjRenderer::renderFilled(std::string_view objPath, std::string_view outputPath,
                              std::string_view zbufferPath) const
{
    return renderObj(objPath, outputPath, drawFilledFace, zbufferPath);
}
