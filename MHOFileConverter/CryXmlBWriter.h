#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <fstream>
#include <functional>
#include <algorithm>

namespace CryXMLB {

	bool ConvertXmlToCryXmlb(const std::string& xmlPath, const std::string& output);

}