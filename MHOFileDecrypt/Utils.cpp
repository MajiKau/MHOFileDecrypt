#include "Utils.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

std::vector<uint8_t> ReadFileContents(const std::string& filepath)
{
    // Open file
    std::ifstream file_stream(filepath, std::ios::binary);
    if (!file_stream)
    {
        std::cout << "Failed to open file: " << filepath;
    }

    // Get file length
    file_stream.seekg(0, file_stream.end);
    uint64_t size = file_stream.tellg();
    file_stream.seekg(0, file_stream.beg);

    // Read data into vector
    std::vector<uint8_t> data;
    data.resize(size);

    file_stream.read(reinterpret_cast<char*>(data.data()), size);
    file_stream.close();

    return data;
}


bool SaveToFile(const std::string& filepath, const std::vector<uint8_t>& data) {
    std::ofstream out(filepath, std::ios::binary | std::ios::trunc);
    out.write(reinterpret_cast<const char*>(data.data()), data.size());
    return out.good();
}

bool IsFileCryXmlb(const std::string& filepath) {
    std::vector<uint8_t> buffer = ReadFileContents(filepath);
    return (std::string_view(reinterpret_cast<const char*>(buffer.data()), 7) == "CryXmlB");
}