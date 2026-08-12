#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sodium.h>
#include <iomanip>
#include <string>
#include <sstream>
using namespace std;

class Decryption{
public:
    string dec(string file_data,  string master_password){
        if (file_data.length() < 40) {
            cerr << "Error: Vault file is corrupted or empty!\n";
            return "ERROR";
        }
        unsigned char salt[16];
        for(int i = 0; i<16; i++){
            salt[i] = (unsigned char)file_data[i];
        }
        unsigned char nonce[24];
        for(int i = 0; i<24; i++){
            nonce[i] = (unsigned char)file_data[i+16];
        }
        size_t ciper_len = file_data.length()-40;
        unsigned char* ciper = new unsigned char[ciper_len];
        for(int i = 0; i<ciper_len; i++){
            ciper[i] = (unsigned char)file_data[i+40];
        }
        unsigned char key[32];
        if (crypto_pwhash(key, sizeof key, master_password.c_str(), master_password.length(), salt, crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE, crypto_pwhash_ALG_DEFAULT) != 0) {
            cerr << "Error: Out of memory during key derivation.\n";
            return "ERROR";
        }
        size_t plain_len = ciper_len - crypto_secretbox_MACBYTES;
        unsigned char* plain_mem = new unsigned char[plain_len];
        int pass_check = crypto_secretbox_open_easy(plain_mem, ciper, ciper_len, nonce, key);
        if (pass_check == -1){
            cerr << "Incorrect Password";
            delete[] ciper;
            delete[] plain_mem;
            return "ERROR";
        }
        string decrypted_text((char*)plain_mem, plain_len);
        delete[] ciper;
        delete[] plain_mem;
        return decrypted_text;
    }
};

class Encryption{
public:
    string enc(string text, string password){
        unsigned char salt[16];
        randombytes_buf(salt, sizeof(salt));

        unsigned char key[32];
        unsigned long long key_len = sizeof(key);
        if (crypto_pwhash(key, sizeof key, 
                      password.c_str(), password.length(), 
                      salt, 
                      crypto_pwhash_OPSLIMIT_INTERACTIVE, 
                      crypto_pwhash_MEMLIMIT_INTERACTIVE, 
                      crypto_pwhash_ALG_DEFAULT) != 0) {
        cerr << "Error: Out of memory during key derivation.\n";
        return "ERROR";
        }
        unsigned char nonce[24];
        randombytes_buf(nonce, sizeof(nonce));
        size_t enc_len = text.length() + crypto_secretbox_MACBYTES;
        unsigned char* encrypted_text = new unsigned char[enc_len];
        crypto_secretbox_easy(encrypted_text, (const unsigned char*)text.c_str(), text.length(), nonce, key);
        string payload = "";
        payload.append((const char*)salt, sizeof(salt));
        payload.append((const char*)nonce, sizeof(nonce));
        payload.append((const char*)encrypted_text, enc_len);
        delete[] encrypted_text;
        return payload;
    }
};

class CheckCreate {
public:
    // Checking file existance
    int check() {
        ifstream vault(".vault.clef");
        if (!vault.is_open()) {
            vault.close();
            cerr << "File Not Found\n";
            cout << "------------------------------";
            cout << "\n";
            return 404;
        } else {
            cout << "vault found, Do you want to use it? (y/n): ";
            char open;
            cin >> open;
            if (open == 'y' || open == 'Y') {
                return 1;
            } else if (open == 'n' || open == 'N') {
                exit(1);
            } else {
                cerr << "Invaild input";
                exit(1);
            }
        }
        vault.close();
        return 0;
    }
    // Creating file
    int create() {
        ofstream vault(".vault.clef");
        if (vault) {
            cout << "\nNew file is created successfully\n";
            vault.close();
            return 0;
        } else {
            cerr << "\nFile was not created, possible problems could be lack of "
                    "permission to write";
            exit(1);
        }
        return 0;
    }
};

class Read {
public:
    string Readvault(string master_password) {
        Decryption dec;
        ifstream vault(".vault.clef", ios::binary | ios::ate);
        streamsize size = vault.tellg();
        vault.seekg(0, ios::beg);
        string raw_payload(size, '\0');
        vault.read(&raw_payload[0], size);
        return dec.dec(raw_payload, master_password);
    }
};

class Write {
public:
    void WriteRam(string& master_vault_data ,string app_name, string username, string password) {
        string added_string = app_name + "\t" + username + "\t" + password + "\n";
        master_vault_data += added_string;
    }

    void WriteFile(string final_payload){
        ofstream vault(".vault.clef", ios::binary);
        vault.write(final_payload.data(), final_payload.size());
        vault.close();
    }

    void WRAM(string& master_vault_data){
        Write wt;
        string appName, uname, passwd;
        // Write to RAM
        while (true){
            cout << "Enter the application name (EXIT to exit): ";
            getline(cin, appName);
            if (appName == ""){
                continue;
            }
            if (appName == "EXIT"){
                break;
            }
            cout << "Enter the username: ";
            getline(cin, uname);
            if (uname == ""){
                continue;
            }
            cout << "Enter the password: ";
            getline(cin, passwd);
            if (passwd == ""){
                continue;
            }
            wt.WriteRam(master_vault_data, appName, uname, passwd);
        }
    }
};

class Password {
public:
    string password(){
        cout << "Enter a super strong password: ";
        string passwd;
        cin >> passwd;
        cin.ignore();
        return passwd;
    }
};

class Delete{
public:
    void removeLine(string& master_vault_data, string target_app){
        string updated_vault;
        bool found = false;
        string line;
        istringstream stream(master_vault_data);
        while (getline(stream, line)){
            if (line.find(target_app + "\t") == 0){
                found = true;
                cout << "Deleted successfully!\n";
            }else{
                updated_vault += line + "\n";
            }
        }
        master_vault_data = updated_vault;
        if (!found) { cout << "App not found!\n"; }
    }
};

int main() {
    // Sodium initialization
    if (sodium_init() < 0){
        cout << "Initialization failed";
    }
    // Objects
    string password;
    CheckCreate CC;
    Read rd;
    Write wt;
    Delete dt;
    Password pd;
    Encryption encrypt;
    Decryption decrypt;

    // Variables
    int check = CC.check();
    char choice = 'y';
    string master_vault_data = "", appName;
    string master_password;

    // Logic for phase 1
    if (check == 404){
        cout << "Vault not found, Do you want to create a new vault? (Y/n): ";
        cin >> choice;
        master_password = pd.password();
        if(choice == 'y' || choice == 'Y'){
            CC.create();
        }else if(choice == 'n' || choice == 'N'){
            cout << "Exiting....";
            exit(1);
        }else {
            cout << "Invaild syntax...";
            exit(1);
        }
    }else if(check == 1){
        cout << "Enter your master password to open the vault: ";
        cin >> master_password;
        master_vault_data = rd.Readvault(master_password);
        if (master_vault_data == "ERROR") {
            exit(1);
        }
        cout << "===================================\n";
        cout << master_vault_data;
        cout << "===================================\n";
    }
    while (true){
        cout << "Options: (A)dd new password, (D)elete a password, (S)ave and Exit: ";
        cin >> choice;
        cin.ignore();
        if (choice == 'A' || choice == 'a'){
            wt.WRAM(master_vault_data);
        }
        else if (choice == 'D' || choice == 'd'){
            cout << "Which app username and password should be deleted: ";
            getline(cin, appName);
            dt.removeLine(master_vault_data, appName);
        }
        else
            break;
    }

    // Passing the master data and the master password to encrypt
    string encrypted_text = encrypt.enc(master_vault_data, master_password);

    // Writing the encrypted_text to File
    wt.WriteFile(encrypted_text);

    return 0;
}