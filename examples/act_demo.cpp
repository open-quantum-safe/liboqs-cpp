// quantum-resistant digital signature in liboqs-cpp
#include <iostream>
#include "oqs_cpp.h"

int main() {
    std::cout << "Enabled signatures:\n" << oqs::Sigs::get_enabled_sigs();

    oqs::bytes message = "ACT March 2019 quantum computing workshop"_bytes;

    std::string sig_name = "DEFAULT";
    oqs::Signature signer{sig_name};
    std::cout << "\n\nSignature details:\n" << signer.get_details();

    oqs::bytes signer_public_key = signer.generate_keypair();
    std::cout << "\n\nSigner public key:\n" << oqs::hex_chop(signer_public_key);

    oqs::bytes signature = signer.sign(message);

    oqs::Signature verifier{sig_name};
    bool is_valid = verifier.verify(message, signature, signer_public_key);

    std::cout << "\n\nSignature:\n" << oqs::hex_chop(signature);

    std::cout << std::boolalpha;
    std::cout << "\n\nValid signature? " << is_valid << '\n';
    std::cout << std::noboolalpha;
}
