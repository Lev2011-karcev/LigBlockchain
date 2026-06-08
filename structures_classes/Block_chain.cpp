#include "Block_chain.h"

#include <iostream>

BlockChain::BlockChain() {

}



void BlockChain::addTransaction(const Transaction& tx){
    if (tx.verifyTransactions()){
        mempool.push_back(tx);
    }
}
void BlockChain::adjustDifficulty() {
    if (chain.size() == 0 || chain.size() % 5 != 0) return;

    const uint64_t BLOCK_GENERATION_INTERVAL = 10;
    uint64_t expectedTime = 5 * BLOCK_GENERATION_INTERVAL;
    uint64_t latestBlockTime = chain.back().timestamp;
    uint64_t oldBlockTime = chain[chain.size() - 5].timestamp;

    uint64_t actualTime = latestBlockTime - oldBlockTime;

    if (actualTime < expectedTime) {
        currentDifficulty++;
        std::cout << "[Difficulty Adjustment] Сеть слишком быстрая. Сложность увеличена до: " << currentDifficulty << "\n";
    } 
    else if (actualTime > expectedTime) {
        if (currentDifficulty > 1) {
            currentDifficulty--;
            std::cout << "[Difficulty Adjustment] Сеть слишком медленная. Сложность уменьшена до: " << currentDifficulty << "\n";
        }
    }
}
void BlockChain::minePendingTransactions(const vector<uint8_t>& minerRewardAddress) {
    adjustDifficulty();
    Transaction rewardTx;
    rewardTx.sender = {}; 
    rewardTx.receiver = minerRewardAddress;
    rewardTx.amount = 50;
    rewardTx.nonce = 0;

    std::vector<Transaction> blockTransactions = { rewardTx };

    blockTransactions.insert(blockTransactions.end(), mempool.begin(), mempool.end());
    Block newB = Block::newBlock(
        chain.size(),
        std::time(nullptr),
        getLatestBlock().blockHash,
        blockTransactions,
        currentDifficulty
    );
    chain.push_back(newB);
    mempool.clear();
}

bool BlockChain::isValidChain() const {
    bool valid = true;

    for (int i = 1; i < chain.size();i++){
        if (chain[i].previousBlockHash != chain[i-1].calculateBlockHash()){
            valid = false;
            break;
        }
        if (chain[i].blockHash != chain[i].calculateBlockHash()){
            valid = false;
            break;
        }
        bool tr = true;
        for (Transaction tx : chain[i].transactions){
            if (!tx.verifyTransactions()){
                tr = false;
                break;
            }
        }
        if (!tr){
            valid = false;
            break;
        }
    }
    return valid;
}

