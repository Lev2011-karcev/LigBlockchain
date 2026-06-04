#include "Transaction.h"
#include "picosha2.h"
#include <cstring>
#include "bt_secp256k1.h" 
#include <random>
#include <algorithm>

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

bool Transaction::signTransaction(const vector<uint8_t>& secretKey) {
    if (secretKey.size() != 32) return false;

    secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    
    vector<uint8_t> hash = calculateHash();
    vector<uint8_t> k(32);
    random_device rd;
    bool kValid = false;

    while (!kValid) {
        for (size_t i = 0; i < 8; ++i) {
            uint32_t chunk = rd();
            memcpy(&k[i * 4], &chunk, 4);
        }
        if (secp256k1_ec_seckey_verify(ctx, k.data()) == 1) {
            kValid = true;
        }
    }
    secp256k1_pubkey R_point;
    if (secp256k1_ec_pubkey_create(ctx, &R_point, k.data()) != 1) {
        secp256k1_context_destroy(ctx);
        return false;
    }

    vector<uint8_t> serialized(33);
    size_t len = 33;
    secp256k1_ec_pubkey_serialize(ctx, serialized.data(), &len, &R_point, SECP256K1_EC_COMPRESSED);
    if (serialized[0] == 0x03) {
        secp256k1_ec_seckey_negate(ctx, k.data());
        secp256k1_ec_pubkey_create(ctx, &R_point, k.data());
        secp256k1_ec_pubkey_serialize(ctx, serialized.data(), &len, &R_point, SECP256K1_EC_COMPRESSED);
    }

    vector<uint8_t> x(32);
    copy(serialized.begin() + 1, serialized.end(), x.begin());
 
    vector<uint8_t> e_buf = x;
    e_buf.insert(e_buf.end(), sender.begin(), sender.end());
    e_buf.insert(e_buf.end(), hash.begin(), hash.end());

    vector<uint8_t> e(32);
    picosha2::hash256(e_buf.begin(), e_buf.end(), e.begin(), e.end());
         
    vector<uint8_t> buf_d = secretKey;
    if (secp256k1_ec_privkey_tweak_mul(ctx, buf_d.data(), e.data()) != 1) {
        secp256k1_context_destroy(ctx);
        return false;
    } 
    vector<uint8_t> s = k; 
    if (secp256k1_ec_seckey_tweak_add(ctx, s.data(), buf_d.data()) != 1) {
        secp256k1_context_destroy(ctx);
        return false;
    }

    // Сохраняем готовую 64-байтную подпись
    this->sign.clear();
    this->sign.insert(this->sign.end(), x.begin(), x.end());
    this->sign.insert(this->sign.end(), s.begin(), s.end());

    secp256k1_context_destroy(ctx);
    return true;
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