# Clef Vault 🔐
<p align="left">
  <img src="https://img.shields.io/github/v/release/NotAlive24/clef?color=blue&label=Version&style=flat" alt="Version">
  <img src="https://img.shields.io/github/repo-size/NotAlive24/clef?color=green&label=Size&style=flat" alt="Size">
  <img src="https://img.shields.io/github/languages/top/NotAlive24/clef?color=cyan&style=flat" alt="Language">
  <img src="https://img.shields.io/github/license/NotAlive24/clef?color=orange&style=flat" alt="License">
  <img src="https://img.shields.io/github/last-commit/NotAlive24/clef?color=purple&style=flat" alt="Last Commit">
  <img src="https://img.shields.io/github/issues/NotAlive24/clef?color=red&style=flat" alt="Issues">
</p>

An offline, CLI-based credential vault built in C++17 utilizing `libsodium` for authenticated symmetric encryption, memory zeroization, and key derivation.

---

## Technical Specifications

| Security Domain | Implementation | Technical Detail |
| :--- | :--- | :--- |
| **Key Derivation (KDF)** | Argon2id (`crypto_pwhash`) | Derives a 256-bit secret key using CPU/RAM-hard parameters (`OPSLIMIT_INTERACTIVE`, `MEMLIMIT_INTERACTIVE`) with a unique 16-byte random salt to prevent precomputation and rainbow table attacks. |
| **Authenticated Encryption** | XSalsa20-Poly1305 (`crypto_secretbox_easy`) | Combines XSalsa20 stream cipher encryption with a Poly1305 Message Authentication Code (MAC) using a unique 24-byte random nonce per save cycle. |
| **Memory Hygiene** | Explicit RAM Zeroization | Derived keys, dynamic buffers, and plaintexts are zeroed out via `sodium_memzero()` prior to function returns or program termination. |
| **Access Controls** | Strict File Permissions | Creates `.vault.clef` via POSIX `open()` with `S_IRUSR \| S_IWUSR` (`0600`), enforcing user-only read/write access at the system level. |

---

## Vault File Layout

The encrypted output file (`.vault.clef`) stores data in a binary layout:

```text
+-----------------------+------------------------+---------------------------------------+
| Salt (16 Bytes)       | Nonce (24 Bytes)       | Ciphertext + Poly1305 MAC (Variable)  |
+-----------------------+------------------------+---------------------------------------+
| 0x00               0x0F | 0x10              0x27 | 0x28                              ... |
+-----------------------+------------------------+---------------------------------------+
