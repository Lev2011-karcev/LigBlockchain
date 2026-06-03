#include "Transaction.h"
#include "picosha2.h"
#include <cstring>

// Метод перевода полей транзакции в единый массив байт для подписи
std::vector<uint8_t> Transaction::serializeForSigning() const {
    std::vector<uint8_t> buffer;

    buffer.insert(buffer.end(), sender.begin(), sender.end());
    buffer.insert(buffer.end(), receiver.begin(), receiver.end());

    uint8_t amountBytes[8];
    std::memcpy(amountBytes, &amount, 8);
    buffer.insert(buffer.end(), amountBytes, amountBytes + 8);

    uint8_t nonceBytes[8];
    std::memcpy(nonceBytes, &nonce, 8);
    buffer.insert(buffer.end(), nonceBytes, nonceBytes + 8);

    // Поля подписи r, s сюда НЕ добввляем так как подпись создается НАД этими байтами!
    return buffer;
}
std::vector<uint8_t> Transaction::calculateHash() const {
    std::vector<uint8_t> txBytes = serializeForSigning();
    
    //хэш с помощью picosha2
    std::vector<uint8_t> hash(32);
    picosha2::hash256(txBytes.begin(), txBytes.end(), hash.begin(), hash.end());
    
    return hash;
}
