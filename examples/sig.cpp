// signature C++ example

#include <iostream>

// liboqs C++ wrapper
#include "oqs_cpp.h"

int main() {
    std::cout << "Supported signatures:\n" << oqs::Sigs::get_supported_sigs();
    std::cout << "\n\nEnabled signatures:\n" << oqs::Sigs::get_enabled_sigs();

    oqs::bytes message = "This is the message to sign"_bytes;

    oqs::Signature signer{"DEFAULT"};
    std::cout << "\n\nSignature details:\n" << signer.get_details();
    oqs::bytes signer_public_key = signer.generate_keypair();
    oqs::bytes signature = signer.sign(message);

    oqs::Signature verifier{"DEFAULT"};
    bool is_valid =
        verifier.verify(message, signature, signer_public_key);

    std::cout << "\n\nSignature:\n" << signature;

    std::cout << std::boolalpha;
    std::cout << "\n\nValid signature? " << is_valid << '\n';
    std::cout << std::noboolalpha;
}
