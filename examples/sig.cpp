// signature C++ example

#include <chrono>
#include <iostream>
#include <string>

// liboqs C++ wrapper
#include "oqs_cpp.hpp"

int main() {
    std::cout << std::boolalpha;
    std::cout << "liboqs version: " << oqs::oqs_version() << '\n';
    std::cout << "liboqs-cpp version: " << oqs::oqs_cpp_version() << '\n';
    std::cout << "Enabled signatures:\n" << oqs::Sigs::get_enabled_sigs();

    oqs::bytes message = "This is the message to sign"_bytes;
    std::string sig_name = "Dilithium2";
    oqs::Signature signer{sig_name};
    std::cout << "\n\nSignature details:\n" << signer.get_details();

    oqs::Timer<std::chrono::microseconds> t;
    oqs::bytes signer_public_key = signer.generate_keypair();
    t.toc();
    std::cout << "\n\nSigner public key:\n" << oqs::hex_chop(signer_public_key);
    std::cout << "\n\nIt took " << t << " microsecs to generate the key pair";

    t.tic();
    oqs::bytes signature = signer.sign(message);
    t.toc();
    std::cout << "\nIt took " << t << " microsecs to sign the message";
    std::cout << "\n\nSignature:\n" << oqs::hex_chop(signature);

    oqs::Signature verifier{sig_name};
    bool is_valid = verifier.verify(message, signature, signer_public_key);
    std::cout << "\n\nValid signature? " << is_valid << '\n';

    return is_valid ? oqs::OQS_STATUS::OQS_SUCCESS : oqs::OQS_STATUS::OQS_ERROR;
}
