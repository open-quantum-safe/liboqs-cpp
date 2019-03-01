// key encapsulation C++ example

#include <iostream>
#include <tuple>

// liboqs C++ wrapper
#include "oqs_cpp.h"

int main() {
    std::cout << "Supported KEMs:\n" << oqs::KEMs::get_supported_KEMs();
    std::cout << "Supported KEMs:\n" << oqs::KEMs::get_supported_KEMs();
    std::cout << "\n\nEnabled KEMs:\n" << oqs::KEMs::get_enabled_KEMs();

    oqs::KeyEncapsulation client{"DEFAULT"};
    std::cout << "\n\nKEM details: \n" << client.get_details();
    oqs::bytes client_public_key = client.generate_keypair();
    std::cout << "\n\nClient public key:\n" << oqs::hex_chop(client_public_key);

    oqs::KeyEncapsulation server{"DEFAULT"};

    oqs::bytes ciphertext, shared_secret_server;
    std::tie(ciphertext, shared_secret_server) =
        server.encap_secret(client_public_key);

    oqs::bytes shared_secret_client = client.decap_secret(ciphertext);

    std::cout << "\n\nClient shared secret:\n" << shared_secret_client;
    std::cout << "\n\nServer shared secret:\n" << shared_secret_server;
    std::cout << "\n\nShared secrets coincide? ";
    std::cout << std::boolalpha;
    std::cout << (shared_secret_client == shared_secret_server) << '\n';
    std::cout << std::noboolalpha;
}
