// signature C++ example

#include <iostream>

// liboqs C++ wrapper
#include "oqs_cpp.h"

int main() {
    std::cout << "Supported Sigs:\n";
    for (auto&& elem : oqs::Sigs::get_supported_Sigs())
        std::cout << elem << ' ';

    std::cout << "\n\nEnabled Sigs:\n";
    for (auto&& elem : oqs::Sigs::get_enabled_Sigs())
        std::cout << elem << ' ';

    oqs::Signature signer{"DEFAULT"};
    std::cout << "\n\nSignature details:\n" << signer.get_details();
    oqs::bytes signer_public_key = signer.generate_keypair();

    oqs::Signature verifier{"DEFAULT"};

    std::string message = "This is the message to sign";
    oqs::bytes message_bytes(message.begin(), message.end());
    oqs::bytes signature = signer.sign(message_bytes);

    bool is_valid =
        verifier.verify(message_bytes, signature, signer_public_key);

    std::cout << "\n\nSignature:\n" << signature;

    std::cout << std::boolalpha;
    std::cout << "\n\nValid signature? " << is_valid << '\n';
    std::cout << std::noboolalpha;
}
