#include <cstdio>
char* readBinaryFile(const char& path, int& size)
{
    FILE* file = fopen(&path, "rb");
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    char* data = new char[size];
    fseek(file, 0, SEEK_SET);
    size_t bytesRead = fread(data, 1, size, file);
    if (bytesRead != size) {
        delete[] data;
        data = nullptr;
    };
    fclose(file);
    return data;
}
