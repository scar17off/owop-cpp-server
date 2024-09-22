#include "Chunk.h"
#include <fstream>
#include <filesystem>

Chunk::Chunk(int x, int y, const std::string& world) : x(x), y(y), world(world) {
    data.resize(CHUNK_SIZE, std::vector<RGB>(CHUNK_SIZE, RGB(255, 255, 255)));
}

RGB Chunk::getColor(int localX, int localY) const {
    if (localX >= 0 && localX < CHUNK_SIZE && localY >= 0 && localY < CHUNK_SIZE) {
        return data[localX][localY];
    }
    return RGB(0, 0, 0);
}

void Chunk::setColor(int localX, int localY, const RGB& color) {
    if (localX >= 0 && localX < CHUNK_SIZE && localY >= 0 && localY < CHUNK_SIZE) {
        data[localX][localY] = color;
    }
}

void Chunk::setRGB(const RGB& color) {
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            data[x][y] = color;
        }
    }
}

bool Chunk::operator==(const RGB& color) const {
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            if (data[x][y] != color) {
                return false;
            }
        }
    }
    return true;
}

bool Chunk::operator!=(const RGB& color) const {
    return !(*this == color);
}

bool Chunk::saveToFile() const {
    if (isFullyWhite()) {
        std::filesystem::remove(getFilePath());
        return true;
    }

    std::filesystem::create_directories("./worlds/" + world);

    std::ofstream file(getFilePath(), std::ios::binary);
    if (!file) {
        return false;
    }

    // Write chunk coordinates
    file.write(reinterpret_cast<const char*>(&x), sizeof(x));
    file.write(reinterpret_cast<const char*>(&y), sizeof(y));

    // Write chunk data
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            file.write(reinterpret_cast<const char*>(&data[x][y].r), sizeof(uint8_t));
            file.write(reinterpret_cast<const char*>(&data[x][y].g), sizeof(uint8_t));
            file.write(reinterpret_cast<const char*>(&data[x][y].b), sizeof(uint8_t));
        }
    }

    return true;
}

bool Chunk::loadFromFile() {
    std::ifstream file(getFilePath(), std::ios::binary);
    if (!file) {
        setRGB(RGB(255, 255, 255));
        return true;
    }

    file.read(reinterpret_cast<char*>(&x), sizeof(x));
    file.read(reinterpret_cast<char*>(&y), sizeof(y));

    // Read chunk data
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            file.read(reinterpret_cast<char*>(&data[x][y].r), sizeof(uint8_t));
            file.read(reinterpret_cast<char*>(&data[x][y].g), sizeof(uint8_t));
            file.read(reinterpret_cast<char*>(&data[x][y].b), sizeof(uint8_t));
        }
    }

    return true;
}

bool Chunk::isFullyWhite() const {
    const RGB white(255, 255, 255);
    return *this == white;
}

std::string Chunk::getFilePath() const {
    return "./worlds/" + world + "/chunk_" + std::to_string(x) + "_" + std::to_string(y) + ".dat";
}