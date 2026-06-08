#include "Block.h"
#include "picosha2.h"
#include <cstring>
#include <iostream>
#include <stdexcept>

using namespace std;

std::vector<uint8_t> Block::serializeBlock() const {
    std::vector<uint8_t> buffer;
    
    uint8_t indexBytes[8];
    std::memcpy(indexBytes, &index, 8);
    buffer.insert(buffer.end(), indexBytes, indexBytes + 8);
    uint8_t timestampBytes[8];
    std::memcpy(timestampBytes, &timestamp, 8);
    buffer.insert(buffer.end(), timestampBytes, timestampBytes + 8);
    uint8_t nonceBytes[8];
    std::memcpy(nonceBytes, &nonce, 8);
    buffer.insert(buffer.end(), nonceBytes, nonceBytes+8);

    buffer.insert(buffer.end(), previousBlockHash.begin(), previousBlockHash.end());
    for (const Transaction& trans : transactions) { // используем  & чтобы не копировать память зря
        std::vector<uint8_t> txSign = trans.serializeForSigning();
        buffer.insert(buffer.end(), txSign.begin(), txSign.end());
    }

    return buffer;
}

std::vector<uint8_t> Block::calculateBlockHash() const {
    std::vector<uint8_t> txBytes = serializeBlock();
    std::vector<uint8_t> hash(32);
    picosha2::hash256(txBytes.begin(), txBytes.end(), hash.begin(), hash.end());
    return hash;
}

bool Block::mineBlock(size_t difficult){
    vector<uint8_t> target(difficult, 0);
    
    while(true){
        this->blockHash = calculateBlockHash();
        if (this->blockHash.size() < difficult){
            return false;
        }
        if (vector<uint8_t>(this->blockHash.begin(), this->blockHash.begin() + difficult) == target){
            return true;
        }
        this->nonce++;
    }
}

Block Block::newBlock(uint64_t index, uint64_t timestamp, std::vector<uint8_t> previousBlockHash, std::vector<Transaction> transactions, size_t difficult) {
    Block tempBlock(index, timestamp, previousBlockHash, transactions, std::vector<uint8_t>());
    tempBlock.blockHash = tempBlock.calculateBlockHash();
    if (!tempBlock.mineBlock(difficult)) {
        throw std::runtime_error("Mining failed: block hash size is smaller than difficulty!");
    }
    return tempBlock;
}
