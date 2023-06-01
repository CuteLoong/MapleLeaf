#include <iostream>
#include <filesystem>

int main(int argc, char** argv)
{
    std::filesystem::path path = "/Resources/Shader/tri1.vert";
    std::cout << std::filesystem::exists(path) << std::endl;

    return 0;
}