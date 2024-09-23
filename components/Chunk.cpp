#include "Chunk.h"
#include <fstream>
#include <filesystem>
#include <algorithm>

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

std::vector<uint8_t> Chunk::getData() const {
    std::vector<uint8_t> result(CHUNK_SIZE * CHUNK_SIZE * 3 + 10 + 4);
    std::vector<uint8_t> compressedData;
    std::vector<std::pair<uint16_t, uint16_t>> compressedPos;

    uint32_t lastColor = (data[0][0].r << 16) | (data[0][0].g << 8) | data[0][0].b;
    uint16_t t = 1;
    uint16_t compBytes = 3;

    for (int y = 0; y < CHUNK_SIZE; ++y) {
        for (int x = 0; x < CHUNK_SIZE; ++x) {
            if (x == 0 && y == 0) continue;

            const RGB& color = data[x][y];
            uint32_t currentColor = (color.r << 16) | (color.g << 8) | color.b;
            compBytes += 3;

            if (currentColor == lastColor) {
                ++t;
            } else {
                if (t >= 3) {
                    compBytes -= t * 3 + 3;
                    compressedPos.push_back({compBytes, t});
                    compBytes += 5 + 3;
                }
                lastColor = currentColor;
                t = 1;
            }
        }
    }

    if (t >= 3) {
        compBytes -= t * 3;
        compressedPos.push_back({compBytes, t});
        compBytes += 5;
    }

    uint16_t totalAreas = compressedPos.size();
    uint16_t s = CHUNK_SIZE * CHUNK_SIZE * 3;

    result[0] = 2; // protocol.server.chunkLoad
    *reinterpret_cast<int32_t*>(&result[1]) = x;
    *reinterpret_cast<int32_t*>(&result[5]) = y;
    result[9] = isProtected() ? 1 : 0;

    size_t curr = 10;

    *reinterpret_cast<uint16_t*>(&result[curr]) = s;
    curr += 2;

    *reinterpret_cast<uint16_t*>(&result[curr]) = totalAreas;
    curr += 2;

    for (const auto& point : compressedPos) {
        *reinterpret_cast<uint16_t*>(&result[curr]) = point.first;
        curr += 2;
    }

    size_t di = 0;
    size_t ci = 0;
    for (const auto& point : compressedPos) {
        while (ci < point.first) {
            result[curr + ci++] = reinterpret_cast<const uint8_t*>(&data[0][0])[di++];
        }
        *reinterpret_cast<uint16_t*>(&result[curr + ci]) = point.second;
        ci += 2;
        result[curr + ci++] = reinterpret_cast<const uint8_t*>(&data[0][0])[di++];
        result[curr + ci++] = reinterpret_cast<const uint8_t*>(&data[0][0])[di++];
        result[curr + ci++] = reinterpret_cast<const uint8_t*>(&data[0][0])[di++];
        di += point.second * 3 - 3;
    }
    while (di < s) {
        result[curr + ci++] = reinterpret_cast<const uint8_t*>(&data[0][0])[di++];
    }

    size_t size = compBytes + totalAreas * 2 + 10 + 2 + 2;
    result.resize(size);
    return result;
}