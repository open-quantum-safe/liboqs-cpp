// key encapsulation C++ example

#include <chrono>
#include <iostream>
#include <string>
#include <tuple>

// liboqs C++ wrapper
#include "oqs_cpp.h"

int main() {
    std::cout << std::boolalpha;
    std::cout << "Enabled KEMs:\n" << oqs::KEMs::get_enabled_KEMs();

    std::string kem_name = "Kyber512";
    oqs::KeyEncapsulation client{kem_name};
    std::cout << "\n\nKEM details:\n" << client.get_details();

    oqs::Timer<std::chrono::milliseconds> t;
    oqs::bytes client_public_key = client.generate_keypair();
    t.toc();
    std::cout << "\n\nClient public key:\n" << oqs::hex_chop(client_public_key);
    std::cout << "\n\nIt took " << t << " millisecs to generate the key pair";

    oqs::KeyEncapsulation server{kem_name};
    oqs::bytes ciphertext, shared_secret_server;
    t.tic();
    std::tie(ciphertext, shared_secret_server) =
        server.encap_secret(client_public_key);
    t.toc();
    std::cout << "\nIt took " << t << " millisecs to encapsulate the secret";

    t.tic();
    oqs::bytes shared_secret_client = client.decap_secret(ciphertext);
    t.toc();
    std::cout << "\nIt took " << t << " millisecs to decapsulate the secret";

    std::cout << "\n\nClient shared secret:\n"
              << oqs::hex_chop(shared_secret_client);
    std::cout << "\n\nServer shared secret:\n"
              << oqs::hex_chop(shared_secret_server);
    bool is_valid = (shared_secret_client == shared_secret_server);
    std::cout << "\n\nShared secrets coincide? " << is_valid << '\n';

    return is_valid ? oqs::OQS_STATUS::OQS_SUCCESS : oqs::OQS_STATUS::OQS_ERROR;
}
