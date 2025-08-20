#pragma once
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>

namespace CryXMLB {

	bool ConvertCryXmlbToXml(const std::string& filename, const std::string& output);

}