#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "line_renderer.h"
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

}

int ObjRenderer::render(
    const std::string_view objPath,
    const std::string_view outputPath) const
{
    std::filesystem::path modelPath{std::string(objPath)};
    if(!modelPath.has_parent_path())
    {
        modelPath = std::filesystem::path("assets") / "models" / modelPath;
    }
    std::ifstream objFile{modelPath};
    if(!objFile)
    {
        std::cerr << "Failed to open OBJ file: " << modelPath.string() << '\n';
        return 1;
    }

    TGAImage framebuffer(framebufferWidth, framebufferHeight, TGAImage::RGB);
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

                constexpr TGAColor white = {255, 255, 255, 255};
                constexpr float xScale = (framebufferWidth - 1) * 0.5F;
                constexpr float yScale = (framebufferHeight - 1) * 0.5F;

                const Vertex& firstVertex = vertices[faceVertexIndices.front()];
                const int firstX = static_cast<int>((firstVertex.x + 1.0F) * xScale);
                const int firstY = static_cast<int>((firstVertex.y + 1.0F) * yScale);
                int previousX = firstX;
                int previousY = firstY;

                for(std::size_t i = 1; i < faceVertexIndices.size(); ++i)
                {
                    const Vertex& vertex = vertices[faceVertexIndices[i]];
                    const int x = static_cast<int>((vertex.x + 1.0F) * xScale);
                    const int y = static_cast<int>((vertex.y + 1.0F) * yScale);

                    drawLineInterpolated(
                        previousX, previousY, x, y, framebuffer, white);
                    previousX = x;
                    previousY = y;
                }

                drawLineInterpolated(
                    previousX, previousY, firstX, firstY, framebuffer, white);
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

    return 0;
}
