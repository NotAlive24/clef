# Clef Vault 🔐 (Still Underdevelopment)
<p align="left">
  <img src="https://img.shields.io/github/v/release/NotAlive24/clef?color=blue&label=Version&style=flat" alt="Version">
  <img src="https://img.shields.io/github/repo-size/NotAlive24/clef?color=green&label=Size&style=flat" alt="Size">
  <img src="https://img.shields.io/github/languages/top/NotAlive24/clef?color=cyan&style=flat" alt="Language">
  <img src="https://img.shields.io/github/license/NotAlive24/clef?color=orange&style=flat" alt="License">
  <img src="https://img.shields.io/github/last-commit/NotAlive24/clef?color=purple&style=flat" alt="Last Commit">
  <img src="https://img.shields.io/github/issues/NotAlive24/clef?color=red&style=flat" alt="Issues">
</p>

A locally-hosted, military-grade cryptographic password manager built entirely in C++. 

No cloud, no subscriptions, no third-party servers. Just you, your master password, and a mathematically secure vault living directly on your hard drive.

## ✨ What's New in v2.0: Core Architecture Update
Version 2.0 transitions Clef Vault from a static encryption script into a fully interactive, memory-safe cryptographic management engine.
*   **Data Lifecycle Management:** Introduced an interactive CLI interface allowing users to dynamically read, append, and safely delete specific credential entries during a single active session.
*   **Cryptographic Integrity Checks:** Engineered fail-safes into the `Decryption` engine to detect corrupted, empty, or tampered `.vault.clef` files prior to initiating key derivation, preventing fatal system aborts (`SIGABRT`).
*   **Secure Memory Allocation:** Overhauled the binary extraction pipeline to utilize strictly sized dynamic arrays, ensuring 100% garbage collection (`delete[]`) even during failed decryption attempts.
*   **In-Memory Stream Parsing:** Implemented `istringstream` logic for targeted credential deletion, ensuring that master vault modifications are handled entirely in RAM before overwriting the binary payload.
*   **Input Buffer Sanitization:** Resolved terminal buffer overflows by strategically flushing input streams, ensuring clean standard input (`stdin`) reading across varied application loops.

## 🚀 What It Does
Clef Vault allows you to store credentials for different applications securely. It takes your plaintext passwords, encrypts them using state-of-the-art cryptography, and packs them into a single binary file (`.vault.clef`). When you want your passwords back, you provide your master password, and the vault unpacks them straight into your terminal.

## 🧠 Under the Hood (The Cryptography)
This isn't just a Base64 encoding toy; this uses `libsodium` to provide genuine, modern cryptographic security.

*   **Key Derivation (Argon2):** When you type your master password, the program doesn't just use it as-is. It generates a 16-byte random **Salt** and runs it through `crypto_pwhash`. This acts as a mathematical marathon, intentionally consuming memory and CPU to derive a 32-byte key. This makes brute-force attacks by hackers extremely slow and expensive.
*   **Encryption (XSalsa20-Poly1305):** We use a 24-byte random **Nonce** (a number used only once) combined with the derived key to lock the data. XSalsa20 scrambles the text, and Poly1305 stamps it with a cryptographic "wax seal" (MAC). 
*   **Tamper-Proofing:** If a single bit in `.vault.clef` is changed by a hacker (or a corrupted hard drive), the Poly1305 seal breaks, and the program will refuse to open it.

### The Payload Structure
When the program saves to the hard drive, it packs the binary data into a single string in this exact order:

`[ 16-byte Salt ] + [ 24-byte Nonce ] + [ Ciphertext + MAC ]`

To decrypt, the program dynamically slices this payload back into its three original pieces, rebuilds the master key using the extracted salt, and unlocks the ciphertext.

## ⚙️ The Workflow
1.  **Boot & Check:** The program checks if `.vault.clef` exists. If not, it prompts you to create one and set a master password.
2.  **Decrypt to RAM:** If the vault exists, it reads the *raw binary* into memory, prompts for the master password, and decrypts the contents.
3.  **Manage (In-Memory):** A built-in menu allows you to **(A)dd** new passwords or **(D)elete** existing ones by searching for the app name. All changes happen purely in RAM.
4.  **Lock & Save:** Upon exit, the program generates a *brand new* Salt and Nonce, encrypts the entire updated block of text, overwrites the old `.vault.clef`, and wipes the plaintext from RAM.

## 🛠️ How to Compile & Run
Because this project relies on `libsodium`, you must link the library during compilation.

```bash
# Compile the code
g++ main.cpp -o vault -lsodium

# Run the executable
./vault
```
