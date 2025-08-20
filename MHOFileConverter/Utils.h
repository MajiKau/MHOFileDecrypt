#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

std::vector<uint8_t> ReadFileContents(const std::string& filepath);

bool SaveToFile(const std::string& filepath, const std::vector<uint8_t>& data);

bool IsFileCryXmlb(const std::string& filepath);
