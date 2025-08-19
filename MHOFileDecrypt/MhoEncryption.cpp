#include "MhoEncryption.h"
#include "Utils.h"
namespace MhoEncryption {

    bool _EncryptBuffer(uint8_t* data, uint32_t size)
    {
        if (!data || size == 0)
            return false;

        uint8_t temp[129];

        while (size >= 129) {
            // First 64 bytes with table_0
            for (int i = 0; i < 64; i++)
                temp[i] = data[i + 65] ^ MH_FILE_ENCRYPT_TABLE_0[i];

            // Next 65 bytes with table_1
            for (int i = 0; i < 65; i++)
                temp[i + 64] = data[i] ^ MH_FILE_ENCRYPT_TABLE_1[i];

            memcpy(data, temp, 129);
            data += 129;
            size -= 129;
        }

        // Handle leftovers (<129 bytes)
        if (size > 0) {
            if (size <= 65) {
                for (uint32_t i = 0; i < size; i++)
                    temp[i] = data[i] ^ MH_FILE_ENCRYPT_TABLE_1[i];
            }
            else {
                uint32_t first_part = size - 65;

                for (uint32_t i = 0; i < first_part; i++)
                    temp[i] = data[i + 65] ^ MH_FILE_ENCRYPT_TABLE_0[i];

                for (uint32_t i = 0; i < 65; i++)
                    temp[i + first_part] = data[i] ^ MH_FILE_ENCRYPT_TABLE_1[i];
            }
            memcpy(data, temp, size);
        }

        return true;
    }

    bool _DecryptBuffer(uint8_t* data, uint32_t size)
    {
        if (!data || size == 0)
            return false;

        uint8_t temp[129];

        while (size >= 129) {
            // First 64 bytes with table_0
            for (int i = 0; i < 64; i++)
                temp[i + 65] = data[i] ^ MH_FILE_ENCRYPT_TABLE_0[i];

            // Next 65 bytes with table_1
            for (int i = 0; i < 65; i++)
                temp[i] = data[i + 64] ^ MH_FILE_ENCRYPT_TABLE_1[i];

            memcpy(data, temp, 129);
            data += 129;
            size -= 129;
        }

        // Handle leftover (<129 bytes)
        if (size > 0) {
            if (size <= 65) {
                for (uint32_t i = 0; i < size; i++)
                    temp[i] = data[i] ^ MH_FILE_ENCRYPT_TABLE_1[i];
            }
            else {
                uint32_t first_part = size - 65;
                for (uint32_t i = 0; i < first_part; i++)
                    temp[i + 65] = data[i] ^ MH_FILE_ENCRYPT_TABLE_0[i];

                for (uint32_t i = 0; i < 65; i++)
                    temp[i] = data[i + first_part] ^ MH_FILE_ENCRYPT_TABLE_1[i];
            }
            memcpy(data, temp, size);
        }

        return true;
    }

    bool IsFileEncrypted(const std::string& filepath) {
        std::vector<uint8_t> fileContents = ReadFileContents(filepath);
        // Check magic header
        return fileContents.size() >= std::size(kMagic) && std::equal(std::begin(kMagic), std::end(kMagic), fileContents.begin());
    }

    bool EncryptFile(const std::string& filepath, const std::string& output) {

        auto buffer = ReadFileContents(filepath);

        // Prepend file magic:
        buffer.insert(buffer.begin(), std::begin(kMagic), std::end(kMagic));

        if (!_EncryptBuffer(buffer.data() + 4, buffer.size() - 4)) {
            std::cerr << "Failed to encrypt file.";
            return false;
        }

        return SaveToFile(output, buffer);
    }

    bool DecryptFile(const std::string& filepath, const std::string& output) {
        std::vector<uint8_t> buffer = ReadFileContents(filepath);

        // Remove magic header
        buffer.erase(buffer.begin(), buffer.begin() + 4);

        // Decrypt
        std::cout << "Attempting to decrypt: " << filepath << std::endl;
        _DecryptBuffer(buffer.data(), buffer.size());

		return SaveToFile(output, buffer);
    }
}