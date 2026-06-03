#pragma once
#include <string>
#include <vector>

using namespace std;


struct Transaction{
    vector<uint8_t> sender;
    vector<uint8_t> receiver;
    uint64_t amount;
    uint64_t nonce;
    vector<uint8_t> sign;
    std::vector<uint8_t> serializeForSigning() const;
    std::vector<uint8_t> calculateHash() const;
};