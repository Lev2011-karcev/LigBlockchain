#pragma once
#include <string>
#include <vector>

using namespace std;

struct SchnorrSignature {
    vector<uint8_t> r;
    vector<uint8_t> s;
};

struct Transaction{
    string sender;
    string receiver;
    uint64_t amount;
    uint64_t nonce;
    SchnorrSignature sign;
    std::vector<uint8_t> serializeForSigning() const;
    std::vector<uint8_t> calculateHash() const;
};