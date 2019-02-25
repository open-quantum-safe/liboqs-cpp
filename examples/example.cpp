#include <iostream>

// liboqs C++ wrapper
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

    oqs::KeyEncapsulation key_encapsulation{"DEFAULT"};
    std::cout << key_encapsulation.get_details() << "\n\n";

    // generate a public key
    auto public_key = key_encapsulation.generate_keypair(); // public key
    std::cout << "Public key: " << public_key << "\n\n";

    // encapsulated shared secret on Alice side
    auto encaps = key_encapsulation.encap_secret(public_key);
    auto encaps_shared_secret = encaps.second; // shared secret on Alice side

    // decapsulated shared secret on Bob side
    auto decaps_shared_secret = key_encapsulation.decap_secret(encaps.first);

    std::cout << "Shared secret on Alice side " << encaps_shared_secret << '\n';

    std::cout << "Shared secret on Bob side   " << decaps_shared_secret << '\n';

    // check Alice and Bob shared secrets coincide
    std::cout << "Share secret coincide? ";
    std::cout << std::boolalpha;
    std::cout << (encaps_shared_secret == decaps_shared_secret) << '\n';
    std::cout << std::noboolalpha;
}
