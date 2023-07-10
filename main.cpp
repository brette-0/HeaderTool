#include <iostream>     //shell output
#include <fstream>      //file handling
#include <sstream>      //string streaming
#include <vector>
#include <string>       //string dataype handling

unsigned long calculateCRC32(const unsigned char* data, size_t length) {
    const unsigned long crcTable[256] = {
        // CRC-32 Lookup Table
        // ...
    };

    unsigned long crc = 0xFFFFFFFFUL;
    for (size_t i = 0; i < length; ++i) {
        crc = (crc >> 8) ^ crcTable[(crc ^ data[i]) & 0xFF];
    }
    return crc ^ 0xFFFFFFFFUL;
}

bool fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

std::string getHeaderFilename(const std::string& checksum) {
    return "./headers/" + checksum + ".bin";
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argv[1][0] == '\0') {
        std::cout << "No file specified." << std::endl;
        return 1;
    }

    std::ifstream file(argv[1], std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << argv[1] << std::endl;
        return 1;
    }

    std::vector<unsigned char> romData(std::istreambuf_iterator<char>(file), {});
    file.close();

    if (romData.size() & 0x10) {
        romData.erase(romData.begin(), romData.begin() + 16);
    }

    unsigned long checksum = calculateCRC32(romData.data(), romData.size());
    std::ostringstream checksumStream;
    checksumStream << std::hex << checksum;
    std::string checksumStr = checksumStream.str();

    std::string headerFilename = getHeaderFilename(checksumStr);
    if (!fileExists(headerFilename)) {
        std::cout << "ROM is unrecognized, likely a bad dump." << std::endl;
        return 1;
    }

    std::ifstream headerFile(headerFilename, std::ios::binary);
    if (!headerFile) {
        std::cerr << "Failed to open header file: " << headerFilename << std::endl;
        return 1;
    }

    std::ostringstream payloadStream;
    payloadStream << headerFile.rdbuf();
    std::string payload = payloadStream.str();
    headerFile.close();

    std::ofstream outputFile(payload.substr(16), std::ios::binary);
    if (!outputFile) {
        std::cerr << "Failed to open output file: " << payload.substr(16) << std::endl;
        return 1;
    }

    outputFile.write(payload.c_str(), 16);
    outputFile.write(reinterpret_cast<const char*>(romData.data()), romData.size());
    outputFile.close();

    return 0;
}