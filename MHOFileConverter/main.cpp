#include <string>
#include <iostream>

#include "Utils.h"
#include "MhoEncryption.h"
#include "CryXmlbReader.h"
#include "CryXmlbWriter.h"

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "Usage {} <filepath>\n", argv[0];
        return 1;
    }

    std::string filepath = argv[1];

    if (MhoEncryption::IsFileEncrypted(filepath)) {
		MhoEncryption::DecryptFile(filepath, filepath + ".xmlb");
		CryXMLB::ConvertCryXmlbToXml(filepath + ".xmlb", filepath + ".xml");
        return 0;
    }
    else if (IsFileCryXmlb(filepath)) {
		// Ask if we should convert CryXmlB to XML or encrypt it
		std::cout << "File is CryXmlB format. Convert it to (X)ML or (e)ncrypt it?" << std::endl;
		char choice;
		std::cin >> choice;
		if (choice == 'X' || choice == 'x') {
			CryXMLB::ConvertCryXmlbToXml(filepath, filepath + ".xml");
			std::cout << "Converted CryXmlB to XML successfully." << std::endl;
			return 0;
		}
        else if (choice == 'E' || choice == 'e') {
			MhoEncryption::EncryptFile(filepath, filepath + ".xmlbe");
			std::cout << "File encrypted." << std::endl;
			return 0;
		}
        else
        {
			std::cerr << "Invalid choice. Exiting.";
			return 2;
		}
    }
    else
    {
		// Assume the file is an XML file, convert it to CryXmlB and encrypt it
		CryXMLB::ConvertXmlToCryXmlb(filepath, filepath + ".xmlb");
		MhoEncryption::EncryptFile(filepath + ".xmlb", filepath + ".xmlbe");
		return 0;
	}
}