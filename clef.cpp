#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <sodium.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

constexpr size_t SALT_LEN = crypto_pwhash_SALTBYTES;
constexpr size_t NONCE_LEN = crypto_secretbox_NONCEBYTES;
constexpr size_t MAC_LEN = crypto_secretbox_MACBYTES;
constexpr size_t MIN_PAYLOAD_LEN = SALT_LEN + NONCE_LEN + MAC_LEN;
constexpr char VAULT_PATH[] = ".vault.clef";

void secure_zero_string(std::string& str) {
    if (!str.empty()) {
        sodium_memzero(str.data(), str.capacity());
        str.clear();
    }
}

namespace VaultSecurity {

    bool init() {
        if (sodium_init() < 0) {
            std::cerr << "[!] Error: Cryptographic library (libsodium) failed to initialize.\n";
            return false;
        }
        return true;
    }

    bool create_secure_file() {
        int fd = open(VAULT_PATH, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            std::cerr << "[!] Error: Failed to create vault file with secure permissions.\n";
            return false;
        }
        close(fd);
        return true;
    }

    bool file_exists() {
        struct stat buffer;
        return (stat(VAULT_PATH, &buffer) == 0);
    }

    bool derive_key(uint8_t* key, const std::string& password, const uint8_t* salt) {
        if (crypto_pwhash(key, crypto_secretbox_KEYBYTES,
                         password.c_str(), password.length(),
                         salt,
                         crypto_pwhash_OPSLIMIT_INTERACTIVE,
                         crypto_pwhash_MEMLIMIT_INTERACTIVE,
                         crypto_pwhash_ALG_DEFAULT) != 0) {
            std::cerr << "[!] Error: Key derivation failed (Out of memory).\n";
            sodium_memzero(key, crypto_secretbox_KEYBYTES);
            return false;
        }
        return true;
    }

    bool encrypt(const std::string& plaintext, const std::string& password, std::vector<uint8_t>& payload_out) {
        uint8_t salt[SALT_LEN];
        uint8_t nonce[NONCE_LEN];
        uint8_t key[crypto_secretbox_KEYBYTES];

        randombytes_buf(salt, sizeof(salt));
        randombytes_buf(nonce, sizeof(nonce));

        if (!derive_key(key, password, salt)) {
            return false;
        }

        size_t cipher_len = plaintext.length() + MAC_LEN;
        std::vector<uint8_t> ciphertext(cipher_len);

        crypto_secretbox_easy(
            ciphertext.data(),
            reinterpret_cast<const unsigned char*>(plaintext.c_str()),
            plaintext.length(),
            nonce,
            key
        );

        sodium_memzero(key, sizeof(key));

        payload_out.clear();
        payload_out.insert(payload_out.end(), salt, salt + SALT_LEN);
        payload_out.insert(payload_out.end(), nonce, nonce + NONCE_LEN);
        payload_out.insert(payload_out.end(), ciphertext.begin(), ciphertext.end());

        return true;
    }

    bool decrypt(const std::vector<uint8_t>& payload, const std::string& password, std::string& plaintext_out) {
        if (payload.size() < MIN_PAYLOAD_LEN) {
            std::cerr << "[!] Error: Corrupted or invalid vault payload length.\n";
            return false;
        }

        const uint8_t* salt = payload.data();
        const uint8_t* nonce = payload.data() + SALT_LEN;
        const uint8_t* ciphertext = payload.data() + SALT_LEN + NONCE_LEN;
        size_t cipher_len = payload.size() - (SALT_LEN + NONCE_LEN);

        uint8_t key[crypto_secretbox_KEYBYTES];
        if (!derive_key(key, password, salt)) {
            return false;
        }

        size_t plain_len = cipher_len - MAC_LEN;
        std::vector<uint8_t> plain_buf(plain_len);

        int res = crypto_secretbox_open_easy(
            plain_buf.data(),
            ciphertext,
            cipher_len,
            nonce,
            key
        );

        sodium_memzero(key, sizeof(key));

        if (res != 0) {
            std::cerr << "[!] Error: Authentication failed. Incorrect master password or tampered vault.\n";
            sodium_memzero(plain_buf.data(), plain_buf.size());
            return false;
        }

        plaintext_out.assign(reinterpret_cast<char*>(plain_buf.data()), plain_len);
        sodium_memzero(plain_buf.data(), plain_buf.size());
        return true;
    }
}

namespace VaultIO {

    std::vector<uint8_t> read_binary_file() {
        std::ifstream file(VAULT_PATH, std::ios::binary | std::ios::ate);
        if (!file.is_open()) return {};

        std::streamsize size = file.tellg();
        if (size <= 0) return {};

        file.seekg(0, std::ios::beg);
        std::vector<uint8_t> buffer(size);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            return {};
        }
        return buffer;
    }

    bool write_binary_file(const std::vector<uint8_t>& data) {
        std::ofstream file(VAULT_PATH, std::ios::binary | std::ios::trunc);
        if (!file.is_open()) return false;

        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        return file.good();
    }

    std::string get_line(const std::string& prompt) {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);
        return line;
    }
}

int main() {
    if (!VaultSecurity::init()) {
        return 1;
    }

    std::string master_password;
    std::string vault_data;

    if (!VaultSecurity::file_exists()) {
        std::cout << "No vault found. Create a new vault? (y/n): ";
        std::string ans;
        std::getline(std::cin, ans);
        if (ans != "y" && ans != "Y") return 0;

        master_password = VaultIO::get_line("Set a strong master password: ");
        if (master_password.empty()) {
            std::cerr << "[!] Password cannot be empty.\n";
            return 1;
        }

        if (!VaultSecurity::create_secure_file()) {
            return 1;
        }
    } else {
        master_password = VaultIO::get_line("Enter master password: ");
        std::vector<uint8_t> payload = VaultIO::read_binary_file();

        if (!VaultSecurity::decrypt(payload, master_password, vault_data)) {
            secure_zero_string(master_password);
            return 1;
        }
        std::cout << "\n[+] Vault Unlocked Successfully.\n";
    }

    while (true) {
        std::cout << "\nOptions: (V)iew, (A)dd, (D)elete, (S)ave & Exit: ";
        std::string choice;
        std::getline(std::cin, choice);

        if (choice == "V" || choice == "v") {
            std::cout << "\n--- VAULT CONTENTS ---\n";
            std::cout << (vault_data.empty() ? "[Vault is empty]\n" : vault_data);
            std::cout << "----------------------\n";
        } 
        else if (choice == "A" || choice == "a") {
            std::string app = VaultIO::get_line("App Name: ");
            std::string user = VaultIO::get_line("Username: ");
            std::string pass = VaultIO::get_line("Password: ");
            
            if (!app.empty() && !user.empty() && !pass.empty()) {
                vault_data += app + "\t" + user + "\t" + pass + "\n";
                std::cout << "[+] Entry added in RAM.\n";
            }
            secure_zero_string(pass);
        } 
        else if (choice == "D" || choice == "d") {
            std::string target = VaultIO::get_line("App to delete: ");
            std::istringstream stream(vault_data);
            std::string line, updated_data;
            bool found = false;

            while (std::getline(stream, line)) {
                if (line.rfind(target + "\t", 0) == 0) {
                    found = true;
                } else {
                    updated_data += line + "\n";
                }
            }
            vault_data = updated_data;
            std::cout << (found ? "[+] Deleted successfully.\n" : "[!] App not found.\n");
        } 
        else if (choice == "S" || choice == "s") {
            std::vector<uint8_t> encrypted_payload;
            if (VaultSecurity::encrypt(vault_data, master_password, encrypted_payload)) {
                if (VaultIO::write_binary_file(encrypted_payload)) {
                    std::cout << "[+] Vault encrypted and saved to disk.\n";
                }
            }
            break;
        }
    }

    secure_zero_string(vault_data);
    secure_zero_string(master_password);

    return 0;
}
