```text
  _      _       ____  _            _       _           _       
 | |    (_)     |  _ \| |          | |     | |         (_)      
 | |     _  __ _| |_) | | ___   ___| | ____| |__   __ _ _ _ __  
 | |    | |/ _` |  _ <| |/ _ \ / __| |/ / _` '_ \ / _` | | '_ \ 
 | |____| | (_| | |_) | | (_) | (__|   < (_| | | | (_| | | | | |
 |______|_|\__, |____/|_|\___/ \___|_|\_\__,_|_| |_|\__,_|_|_| |_|
            __/ |                                               
           |___/  Custom C++ PoW Protocol                     

🚀 Overview
A high-performance, lightweight Blockchain Core Engine written in C++17 from scratch.
Developed by Lev Kartsev, 15 years old.

✨ Key Features
 Custom Cryptography: Low-level implementation of Schnorr Signature Scheme (Signing & Verification) utilizing tweaks on the ⁠secp256k1⁠ elliptic curve.
 Production Curve: Built on Bitcoin's robust libsecp256k1.
 PoW Consensus: Standard Proof-of-Work (SHA-256) with dynamic difficulty adjustment based on target block time.
 Memory Management: Clean OOP architecture using modern C++ zero-copy principles (references, move semantics) to minimize memory overhead during transaction validation.
🛠 Tech Stack
 Language: C++17
 Crypto Library: ⁠libsecp256k1⁠
 Core Concepts: Elliptic Curves, PoW, Merkle Proofs (in perspective), OOP Design.

📜 Architecture
 ⁠NewKeys.cpp⁠: Secp256k1 context, key pair generation, public key serialization.
 ⁠Transaction.cpp⁠: Signing & Verification using custom Schnorr.
 ⁠Block.cpp⁠: Hashing, dynamic mining (PoW), and serialization.
 ⁠Block_chain.cpp⁠: Ledger management, mempool, dynamic difficulty adjustment, and chain validation.


