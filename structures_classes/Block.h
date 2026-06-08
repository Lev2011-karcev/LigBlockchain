#ifndef BLOCK_H
#define BLOCK_H

#include <vector>
#include <cstdint>
#include "Transaction.h"

class Block {
public:
    uint64_t index;
    uint64_t timestamp;
    uint64_t nonce;
    std::vector<uint8_t> previousBlockHash;
    std::vector<Transaction> transactions;
    std::vector<uint8_t> blockHash;

    // Конструктор, чтобы можно было удобно создавать объект
    Block(uint64_t idx, uint64_t ts, std::vector<uint8_t> prevHash, std::vector<Transaction> txs, std::vector<uint8_t> hash)
        : index(idx), timestamp(ts),nonce(0), previousBlockHash(prevHash), transactions(txs), blockHash(hash) {}

    std::vector<uint8_t> serializeBlock() const;
    std::vector<uint8_t> calculateBlockHash() const;
    bool mineBlock(size_t difficult);
    // Делаем функцию СТАТИЧЕСКОЙ, чтобы вызывать её везде для создания новых блоков
    static Block newBlock(uint64_t index, uint64_t timestamp, std::vector<uint8_t> previousBlockHash, std::vector<Transaction> transactions, size_t difficult);
};

#endif
