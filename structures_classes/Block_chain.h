#ifndef BLOCK_CHAIN_H
#define BLOCK_CHAIN_H

#include <vector>
#include "Block.h"
#include "Transaction.h"

using namespace std;

class BlockChain{
private:
    vector<Block> chain;
    vector<Transaction> mempool;
    size_t currentDifficulty = 2;
    const uint32_t BLOCK_GENERATION_INTERVAL = 10;

public: 
    BlockChain();

    void addTransaction(const Transaction& tx);
    void minePendingTransactions(const vector<uint8_t>& minerRewardAddress);
    bool isValidChain() const;
    void adjustDifficulty();
    const Block& getLatestBlock() const;

};

#endif