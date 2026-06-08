#include "NewKeys.h"


#include <vector>
#include <cstdint>
#include <random>
#include "bt_secp256k1.h" 

using namespace std;



vector<uint8_t> newSecretKey(){
    secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_NONE);
    vector<uint8_t> secretKey(32);
    random_device rd;
    bool isValid = false;

    while (!isValid){
        for (size_t i = 0; i < 8; ++i){
            uint32_t random_chunk = rd();
            secretKey[i*4] = static_cast<uint8_t>(random_chunk & 0xFF);
            secretKey[i*4 + 1] = static_cast<uint8_t>((random_chunk >> 8) & 0xFF);
            secretKey[i*4 + 2] = static_cast<uint8_t>((random_chunk >> 16) & 0xFF);
            secretKey[i*4 + 3] = static_cast<uint8_t>((random_chunk >> 24) & 0xFF);
        
        }
        int result = secp256k1_ec_seckey_verify(ctx, secretKey.data());
        if (result == 1){
            isValid = true;
        }
    }
    secp256k1_context_destroy(ctx);
    return secretKey;
}

vector<uint8_t> newPublicKey(vector<uint8_t>& secretKey){
    secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    secp256k1_pubkey pubKey;
    if (secp256k1_ec_pubkey_create(ctx, &pubKey, secretKey.data()) != 1) {
        secp256k1_context_destroy(ctx);
        return {}; // Ошибка неверный приватный ключ
    }
    vector<uint8_t> serializedKey(33);
    size_t outLen = 33;
    secp256k1_ec_pubkey_serialize(ctx, serializedKey.data(), &outLen, &pubKey, SECP256K1_EC_COMPRESSED);
    secp256k1_context_destroy(ctx);
    
    return serializedKey;
}
