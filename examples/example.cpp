#include <iostream>

// libows C++ wrapper
#include "oqs_cpp.h"

int main() {
    std::cout << "Supported KEMs:\n";
    for (auto&& elem : oqs::KEMs::get_supported_KEMs())
        std::cout << elem << ' ';
    std::cout << "\n\n";

    std::cout << "Enabled KEMs:\n";
    for (auto&& elem : oqs::KEMs::get_enabled_KEMs())
        std::cout << elem << ' ';
    std::cout << "\n\n";

    std::cout << std::boolalpha;
    std::cout << oqs::KEMs::get_enabled_KEMs().size() << '\n';
    std::cout << oqs::KEMs::get_supported_KEMs().size() << '\n';
    std::cout << oqs::KEMs::get_KEM_name(22) << '\n';
    std::cout << std::noboolalpha;
}
