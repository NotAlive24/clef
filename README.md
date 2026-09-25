# Clef Vault 🔐

<p align="left">
  <img src="https://img.shields.io/github/v/release/NotAlive24/clef?color=blue&label=Version&style=flat" alt="Version">
  <img src="https://img.shields.io/github/repo-size/NotAlive24/clef?color=green&label=Size&style=flat" alt="Size">
  <img src="https://img.shields.io/github/languages/top/NotAlive24/clef?color=cyan&style=flat" alt="Language">
  <img src="https://img.shields.io/github/license/NotAlive24/clef?color=orange&style=flat" alt="License">
</p>

An offline, CLI-based credential vault built in C++17 utilizing `libsodium` for authenticated symmetric encryption, explicit memory zeroization, and secure key derivation. No cloud, no subscriptions, no third-party servers—just a mathematically secure vault living directly on your local file system.

---

## 🛡️ Technical Specifications

| Security Domain | Implementation | Technical Detail |
| :--- | :--- | :--- |
| **Key Derivation (KDF)** | Argon2id (`crypto_pwhash`) | Derives a 256-bit secret key using CPU/RAM-hard parameters (`OPSLIMIT_INTERACTIVE`, `MEMLIMIT_INTERACTIVE`) with a unique 16-byte random salt to prevent precomputation and rainbow table attacks. |
| **Authenticated Encryption** | XSalsa20-Poly1305 (`crypto_secretbox_easy`) | Combines XSalsa20 stream cipher encryption with a Poly1305 Message Authentication Code (MAC) using a unique 24-byte random nonce per save cycle. |
| **Memory Hygiene** | Explicit RAM Zeroization | Derived keys, dynamic buffers, and plaintexts are zeroed out via `sodium_memzero()` prior to function returns or program termination to mitigate process memory dump leaks. |
| **Access Controls** | Strict File Permissions | Creates `.vault.clef` via POSIX `open()` with `S_IRUSR \| S_IWUSR` (`0600`), enforcing user-only read/write access at the local OS level. |

---

## 🗄️ Vault File Layout

The encrypted output file (`.vault.clef`) stores data in a strict binary layout. 

```text
+-----------------------+------------------------+---------------------------------------+
| Salt (16 Bytes)       | Nonce (24 Bytes)       | Ciphertext + Poly1305 MAC (Variable)  |
+-----------------------+------------------------+---------------------------------------+
| 0x00               0x0F | 0x10              0x27 | 0x28                              ... |
+-----------------------+------------------------+---------------------------------------+

```

**Integrity Checks:** The minimum valid vault payload size is 56 bytes (`16-byte Salt` + `24-byte Nonce` + `16-byte MAC`). Files smaller than this threshold are rejected immediately to prevent integer underflow vulnerabilities during decryption slice calculations.

---

## ⚙️ Prerequisites & Dependencies

* C++17 compatible compiler (`g++` 8+ or `clang++` 7+)
* `libsodium` development library

### Installing Libsodium

* **Debian / Ubuntu / Kali:**
```bash
sudo apt update && sudo apt install build-essential libsodium-dev

```


* **Arch Linux:**
```bash
sudo pacman -S gcc libsodium

```


* **macOS (Homebrew):**
```bash
brew install libsodium

```



---

## 🛠️ Compilation

Clone the repository and compile `main.cpp`, ensuring you link against `libsodium`:

```bash
git clone [https://github.com/NotAlive24/clef.git](https://github.com/NotAlive24/clef.git)
cd clef
g++ -std=c++17 main.cpp -lsodium -o clef

```

---

## 🚀 Usage

Run the executable:

```bash
./clef

```

### The Workflow

1. **Boot & Check:** The program checks if `.vault.clef` exists. If not, it prompts you to create one and set a strong master password.
2. **Decrypt to RAM:** If the vault exists, it reads the raw binary into memory, prompts for the master password, and decrypts the contents.
3. **Manage (In-Memory):**
* `(V)iew`: Displays all decrypted credentials currently stored in active RAM.
* `(A)dd`: Appends a new application entry (App Name, Username, Password) to RAM.
* `(D)elete`: Removes a specific application entry from RAM.


4. **Lock & Save:** Selecting `(S)ave & Exit` generates a brand new Salt and Nonce, encrypts the entire updated block of text, overwrites the old `.vault.clef`, and securely wipes the plaintext from RAM.

---

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](https://www.google.com/search?q=LICENSE&utm_source=gemini) file for details.
