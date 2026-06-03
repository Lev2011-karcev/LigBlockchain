#ifndef NEKEYS_H
#define NEWKEYS_H

#include <vector>

using namespace std;

vector<uint8_t> newSecretKey();
vector<uint8_t> newPublicKey(vector<uint8_t>& secretKey);

#endif