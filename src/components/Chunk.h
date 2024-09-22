#pragma once

#include <vector>
#include <string>
#include "RGB.h"

class Chunk {
public:
    static const int CHUNK_SIZE = 16;
    int x;
    int y;
    std::string world;
    std::vector<std::vector<RGB>> data;

    Chunk(int x, int y, const std::string& world);
    RGB getColor(int localX, int localY) const;
    void setColor(int localX, int localY, const RGB& color);
    void setRGB(const RGB& color);

    bool operator==(const RGB& color) const;
    bool operator!=(const RGB& color) const;

    bool saveToFile() const;
    bool loadFromFile();

    bool isFullyWhite() const;

private:
    std::string getFilePath() const;
};