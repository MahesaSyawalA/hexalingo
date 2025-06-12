#include <iostream>
#include <string>
#include <regex>
#include "json.hpp"
#include <fstream>
#include <iomanip>
using json = nlohmann::json;
using namespace std;

struct Session {
    string username;
    string role;
    bool isLoggedIn = false;
};

Session currentSession;

struct UserProfile {
    json profileJson;
};

struct ProfileNode {
    UserProfile profile;
    ProfileNode* next;
};

int nextProfileId = 1;
ProfileNode* profileHead = nullptr;

bool isAdmin() {
    return currentSession.isLoggedIn && currentSession.role == "admin";
}

ProfileNode* cariProfileByUsername(const string& username) {
    ProfileNode* current = profileHead;
    while (current) {
        if (current->profile.profileJson["username"] == username) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

string inputTidakKosong(const string& prompt) {
    string input;
    do {
        cout << prompt;
        getline(cin, input);
        if (input.empty()) cout << "Input tidak boleh kosong.\n";
    } while (input.empty());
    return input;
}

string inputEmailValid(const string& prompt) {
    regex emailRegex("^[\\w.-]+@[\\w.-]+\\.\\w+$");
    string input;
    do {
        input = inputTidakKosong(prompt);
        if (!regex_match(input, emailRegex)) cout << "Format email tidak valid.\n";
    } while (!regex_match(input, emailRegex));
    return input;
}

string inputPasswordValid(const string& prompt) {
    string input;
    do {
        input = inputTidakKosong(prompt);
        if (input.length() < 8) cout << "Password minimal 8 karakter.\n";
    } while (input.length() < 8);
    return input;
}

string inputPhoneValid(const string& prompt) {
    regex phoneRegex("^[0-9]+$");
    string input;
    do {
        input = inputTidakKosong(prompt);
        if (!regex_match(input, phoneRegex)) cout << "Nomor telepon harus berupa angka saja.\n";
    } while (!regex_match(input, phoneRegex));
    return input;
}

string inputRoleValid(const string& prompt) {
    string input;
    do {
        input = inputTidakKosong(prompt);
        if (input != "user" && input != "admin") cout << "Role hanya boleh 'user' atau 'admin'.\n";
    } while (input != "user" && input != "admin");
    return input;
}

string inputUsernameUnik() {
    string username;
    do {
        username = inputTidakKosong("Username     : ");
        if (cariProfileByUsername(username)) cout << "Username sudah digunakan.\n";
    } while (cariProfileByUsername(username));
    return username;
}

string inputUsernameAda() {
    string username;
    do {
        username = inputTidakKosong("Masukkan username: ");
        if (!cariProfileByUsername(username)) cout << "Username tidak ditemukan.\n";
    } while (!cariProfileByUsername(username));
    return username;
}

void loadProfilesFromJson(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) return;
    json data;
    file >> data;
    file.close();

    if (data.contains("session") && !data["session"].is_null()) {
        currentSession.username = data["session"]["username"];
        currentSession.role = data["session"]["role"];
        currentSession.isLoggedIn = true;
    }

    int maxId = 0;
    for (const auto& profileJson : data["users"]) {
        UserProfile p{profileJson};
        ProfileNode* newNode = new ProfileNode{p, nullptr};
        if (!profileHead) profileHead = newNode;
        else {
            ProfileNode* current = profileHead;
            while (current->next) current = current->next;
            current->next = newNode;
        }
        if (profileJson.contains("id")) maxId = max(maxId, profileJson["id"].get<int>());
    }
    nextProfileId = maxId + 1;
}

void saveProfilesToJson(const string& filename) {
    json data;
    ifstream inFile(filename);
    if (inFile.is_open()) inFile >> data;
    inFile.close();

    data["session"] = currentSession.isLoggedIn ? json{{"username", currentSession.username}, {"role", currentSession.role}} : nullptr;
    data["users"] = json::array();

    for (ProfileNode* current = profileHead; current; current = current->next)
        data["users"].push_back(current->profile.profileJson);

    ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << data.dump(4);
        outFile.close();
    }
}

void tambahProfile() {
    cout << "\n== Tambah Profile ==\n";
    string username = inputUsernameUnik();
    string nama = inputTidakKosong("Nama Lengkap : ");
    string email = inputEmailValid("Email        : ");
    string password = inputPasswordValid("Password     : ");
    string phone = inputPhoneValid("No. Telepon  : ");
    string role = inputRoleValid("Role (user/admin): ");

    json newProfile = {
        {"id", nextProfileId++},
        {"username", username},
        {"name", nama},
        {"email", email},
        {"role", role},
        {"password", password},
        {"phone", phone},
        {"mata_pelajaran", json::array()}
    };

    ProfileNode* newNode = new ProfileNode{{newProfile}, nullptr};
    if (!profileHead) profileHead = newNode;
    else {
        ProfileNode* current = profileHead;
        while (current->next) current = current->next;
        current->next = newNode;
    }
    cout << "Profile berhasil ditambahkan!\n";
}

void hapusProfile() {
    cout << "\n== Hapus Profile ==\n";
    string username = inputUsernameAda();
    ProfileNode* current = profileHead;
    ProfileNode* prev = nullptr;

    while (current) {
        if (current->profile.profileJson["username"] == username) {
            if (prev) prev->next = current->next;
            else profileHead = current->next;
            delete current;
            cout << "Profile berhasil dihapus.\n";
            return;
        }
        prev = current;
        current = current->next;
    }
}

void editProfile() {
    cout << "\n== Edit Profile ==\n";
    string username = inputUsernameAda();
    ProfileNode* current = cariProfileByUsername(username);

    current->profile.profileJson["name"] = inputTidakKosong("Nama Baru     : ");
    current->profile.profileJson["email"] = inputEmailValid("Email Baru    : ");
    current->profile.profileJson["phone"] = inputPhoneValid("No. Telepon   : ");
    current->profile.profileJson["password"] = inputPasswordValid("Password Baru : ");
    current->profile.profileJson["role"] = inputRoleValid("Role Baru (user/admin): ");

    cout << "Profile berhasil diperbarui!\n";
}

void lihatDetailProfile() {
    cout << "\n== Lihat Detail Profile ==\n";
    string username = inputUsernameAda();
    ProfileNode* current = cariProfileByUsername(username);

    cout << "\n=== Detail Profile ===\n";
    cout << "Username     : " << current->profile.profileJson["username"] << endl;
    cout << "Nama Lengkap : " << current->profile.profileJson["name"] << endl;
    cout << "Email        : " << current->profile.profileJson["email"] << endl;
    cout << "No. Telepon  : " << current->profile.profileJson["phone"] << endl;
    cout << "Role         : " << current->profile.profileJson["role"] << endl;
}

void tampilkanDaftarProfile() {
    if (!profileHead) {
        cout << "Tidak ada profile yang terdaftar.\n";
        return;
    }
    cout << "\nDAFTAR PROFILE TERDAFTAR\n\n";
    cout << "+----+----------------------+----------------------+----------------------+----------+\n";
    cout << "| ID | Username             | Nama Lengkap         | Email                | Role     |\n";
    cout << "+----+----------------------+----------------------+----------------------+----------+\n";

    int count = 0;
    for (ProfileNode* current = profileHead; current; current = current->next, ++count) {
        json p = current->profile.profileJson;
        cout << "| " << setw(2) << p["id"] << " | "
             << setw(20) << p["username"].get<string>().substr(0, 20) << " | "
             << setw(20) << p["name"].get<string>().substr(0, 20) << " | "
             << setw(20) << p["email"].get<string>().substr(0, 20) << " | "
             << setw(8) << p["role"] << " |\n";
    }
    cout << "+----+----------------------+----------------------+----------------------+----------+\n";
    cout << "Total profile: " << count << endl;
}

int mainProfile() {
    const string jsonFile = "database.json";
    loadProfilesFromJson(jsonFile);

    if (!isAdmin()) {
        lihatDetailProfile();
        cout << "\nTekan enter untuk kembali...";
        cin.ignore();
        // ==== TAMBAHAN: Tambah histori saat masuk menu ini (USER) ====
        tambahHistori("Melihat Profil");
        return 0;
    }

    int pilihan;
    do {
        cout << "\n=== MENU MANAJEMEN PROFILE (ADMIN) ===\n";
        cout << "1. Tambah Profile\n";
        cout << "2. Hapus Profile\n";
        cout << "3. Lihat Detail Profile\n";
        cout << "4. Tampilkan Daftar Profile\n";
        cout << "5. Edit Profile\n";
        cout << "6. Kembali ke Menu Utama\n";
        cout << "Pilih opsi (1-6): ";
        cin >> pilihan;
        cin.ignore();

        switch (pilihan) {
            case 1: tambahProfile(); break;
            case 2: hapusProfile(); break;
            case 3: lihatDetailProfile(); break;
            case 4: tampilkanDaftarProfile(); break;
            case 5: editProfile(); break;
            case 6: cout << "Kembali ke menu utama...\n"; break;
            default: cout << "Pilihan tidak valid.\n";
        }

    } while (pilihan != 6 && isAdmin());

    return 0;
}
