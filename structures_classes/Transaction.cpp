#include "Transaction.h"
#include "picosha2.h"
#include <cstring>
#include "bt_secp256k1.h" 
#include <array>
#include <algorithm>


// Метод перевода полей транзакции в единый массив байт для подписи 
std::vector<uint8_t> Transaction::serializeForSigning() const {
    std::vector<uint8_t> buffer;
    buffer.reserve(sender.size() + receiver.size() + 16);

    buffer.insert(buffer.end(), sender.begin(), sender.end());
    buffer.insert(buffer.end(), receiver.begin(), receiver.end());

    for (int i = 0; i < 8; ++i) {
        buffer.push_back(static_cast<uint8_t>((amount >> (i * 8)) & 0xFF));
    }
    for (int i = 0; i < 8; ++i) {
        buffer.push_back(static_cast<uint8_t>((nonce >> (i * 8)) & 0xFF));
    }
    // Поля подписи r, s сюда НЕ добввляем так как подпись создается НАД этими байтами!
    return buffer;
}

std::vector<uint8_t> Transaction::calculateHash() const {
    std::vector<uint8_t> txBytes = serializeForSigning();
    std::vector<uint8_t> hash(32);
    picosha2::hash256(txBytes.begin(), txBytes.end(), hash.begin(), hash.end());
    return hash;
}

bool Transaction::signTransaction(const std::vector<uint8_t>& secretKey) {
    if (secretKey.size() != 32) return false;

    const secp256k1_context* ctx = secp256k1_context_static;

    if (secp256k1_ec_seckey_verify(ctx, secretKey.data()) != 1) return false;

    std::vector<uint8_t> txHash = calculateHash();

    std::array<uint8_t, 32> k;
    unsigned int counter = 0;
    
    std::vector<uint8_t> k_seed = txHash;
    k_seed.insert(k_seed.end(), secretKey.begin(), secretKey.end());
    
    bool kValid = false;
    while (!kValid) {
        picosha2::hash256(k_seed.begin(), k_seed.end(), k.begin(), k.end());
        if (secp256k1_ec_seckey_verify(ctx, k.data()) == 1) {
            kValid = true;
        } else {
            k_seed.push_back(static_cast<uint8_t>(counter++));
        }
    }

    secp256k1_pubkey R_point;
    if (secp256k1_ec_pubkey_create(ctx, &R_point, k.data()) != 1) return false;

    std::array<uint8_t, 33> serializedR;
    size_t len = serializedR.size();
    secp256k1_ec_pubkey_serialize(ctx, serializedR.data(), &len, &R_point, SECP256K1_EC_COMPRESSED);

    std::array<uint8_t, 32> buf_d;
    std::copy(secretKey.begin(), secretKey.end(), buf_d.begin());

    if (serializedR[0] == 0x03) {
        secp256k1_ec_seckey_negate(ctx, k.data());
        secp256k1_ec_seckey_negate(ctx, buf_d.data());
    }

    std::array<uint8_t, 32> x;
    std::copy(serializedR.begin() + 1, serializedR.end(), x.begin());

    std::vector<uint8_t> e_buf;
    e_buf.reserve(x.size() + sender.size() + txHash.size());
    e_buf.insert(e_buf.end(), x.begin(), x.end());
    e_buf.insert(e_buf.end(), sender.begin(), sender.end());
    e_buf.insert(e_buf.end(), txHash.begin(), txHash.end());

    std::array<uint8_t, 32> e;
    picosha2::hash256(e_buf.begin(), e_buf.end(), e.begin(), e.end());

    if (secp256k1_ec_privkey_tweak_mul(ctx, buf_d.data(), e.data()) != 1) return false;
    if (secp256k1_ec_seckey_tweak_add(ctx, k.data(), buf_d.data()) != 1) return false;

    this->sign.clear();
    this->sign.reserve(64);
    this->sign.insert(this->sign.end(), x.begin(), x.end());
    this->sign.insert(this->sign.end(), k.begin(), k.end());

    return true;
}

bool Transaction::verifyTransactions() const {
    if (this->sign.size() != 64) return false;
    if (this->sender.size() != 33) return false;

    const secp256k1_context* ctx = secp256k1_context_static;

    std::array<uint8_t, 32> x;
    std::array<uint8_t, 32> s;
    std::copy(this->sign.begin(), this->sign.begin() + 32, x.begin());
    std::copy(this->sign.begin() + 32, this->sign.end(), s.begin());

    secp256k1_pubkey P_point;
    if (secp256k1_ec_pubkey_parse(ctx, &P_point, this->sender.data(), this->sender.size()) != 1) {
        return false;
    }

    std::vector<uint8_t> txHash = calculateHash();

    std::vector<uint8_t> e_buf;
    e_buf.reserve(x.size() + sender.size() + txHash.size());
    e_buf.insert(e_buf.end(), x.begin(), x.end());
    e_buf.insert(e_buf.end(), sender.begin(), sender.end());
    e_buf.insert(e_buf.end(), txHash.begin(), txHash.end());

    std::array<uint8_t, 32> e;
    picosha2::hash256(e_buf.begin(), e_buf.end(), e.begin(), e.end());

    std::array<uint8_t, 32> e_neg = e;
    if (secp256k1_ec_seckey_negate(ctx, e_neg.data()) != 1) return false;

    if (secp256k1_ec_pubkey_tweak_mul(ctx, &P_point, e_neg.data()) != 1) return false;
    if (secp256k1_ec_pubkey_tweak_add(ctx, &P_point, s.data()) != 1) return false;

    std::array<uint8_t, 33> serializedR;
    size_t len = serializedR.size();
    if (secp256k1_ec_pubkey_serialize(ctx, serializedR.data(), &len, &P_point, SECP256K1_EC_COMPRESSED) != 1) {
        return false;
    }

    return std::equal(x.begin(), x.end(), serializedR.begin() + 1);
}

/*
это для работыс точками 
secp256k1_pubkey R_point;
// Берет 32 байта скаляра 'k' и превращает в структуру точки 'R_point' тут генератор 
secp256k1_ec_pubkey_create(ctx, &R_point, k.data());
// Изменяет структуру точки P_point, умножая ее на 32 байта скаляра 'e'
secp256k1_ec_pubkey_tweak_mul(ctx, &P_point, e.data());
// Изменяет структуру точки P_point, прибавляя к ней (s * G)
secp256k1_ec_pubkey_tweak_add(ctx, &P_point, s.data());
со Скадярами 
// Изменяет первый аргумент, умножая его на второй
int result = secp256k1_ec_seckey_tweak_mul(ctx, scalarA.data(), scalarB.data());
// Изменяет первый аргумент, прибавляя к нему второй
int result = secp256k1_ec_seckey_tweak_add(ctx, scalarA.data(), scalarB.data());
// Инвертирует скаляр на месте
int result = secp256k1_ec_seckey_negate(ctx, scalarA.data());
Сериализация 
их точки в байты: 
    vector<uint8_t> serialized(33);
size_t len = 33;
secp256k1_ec_pubkey_serialize(ctx, serialized.data(), &len, &point, SECP256K1_EC_COMPRESSED);
// serialized[0] - префикс знака (0x02 или 0x03)
// serialized[1...32] - чистая координата X (32 байта)
Из байт в структуру точки 
    secp256k1_pubkey point;
    // Принимает только полный вариант (33 байта с префиксом 0x02/0x03)
    secp256k1_ec_pubkey_parse(ctx, &point, serialized_bytes.data(), 33);

*/