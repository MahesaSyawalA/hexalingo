#include <iostream>
#include <string>
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
    json profileJson; // Simpan seluruh atribut JSON
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

void loadProfilesFromJson(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Gagal membuka file JSON.\n";
        return;
    }

    json data;
    file >> data;
    file.close();

    // Load session jika ada
    if (data.contains("session") && !data["session"].is_null()) {
        currentSession.username = data["session"]["username"];
        currentSession.role = data["session"]["role"];
        currentSession.isLoggedIn = true;
    }

    int maxId = 0;

    for (const auto& profileJson : data["users"]) {
        UserProfile p;
        p.profileJson = profileJson;

        ProfileNode* newProfileNode = new ProfileNode{p, nullptr};
        if (!profileHead) profileHead = newProfileNode;
        else {
            ProfileNode* current = profileHead;
            while (current->next) current = current->next;
            current->next = newProfileNode;
        }

        if (profileJson.contains("id") && profileJson["id"].is_number_integer())
            maxId = max(maxId, profileJson["id"].get<int>());
    }

    nextProfileId = maxId + 1;
}


void saveProfilesToJson(const string& filename) {
    json data;
    ifstream inFile(filename);
    if (inFile.is_open()) {
        inFile >> data;
        inFile.close();
    } else {
        data = {
            {"users", json::array()},
            {"matakuliah", json::array()},
            {"tugas", json::array()},
            {"jawaban", json::array()},
            {"daftar_mata_pelajaran", json::array()},
            {"session", json::object()}
        };
    }

    // Simpan session jika user logged in
    if (currentSession.isLoggedIn) {
        data["session"] = {
            {"username", currentSession.username},
            {"role", currentSession.role}
        };
    } else {
        data["session"] = nullptr;
    }

    data["users"] = json::array();
    ProfileNode* current = profileHead;
    while (current) {
        data["users"].push_back(current->profile.profileJson);
        current = current->next;
    }

    ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << data.dump(4);
        outFile.close();
        cout << "📁 Data berhasil disimpan.\n";
    } else {
        cout << "❌ Gagal menyimpan ke file.\n";
    }
}

void tampilkanDaftarProfile() {
    if (!profileHead) {
        cout << "📭 Tidak ada profile yang terdaftar.\n";
        return;
    }

    cout << "\n📋 DAFTAR PROFILE TERDAFTAR\n\n";
    
    // Header tabel
    cout << "+----+----------------------+----------------------+----------------------+----------+\n";
    cout << "| " << left << setw(2) << "ID" << " | " 
         << setw(20) << "Username" << " | " 
         << setw(20) << "Nama Lengkap" << " | " 
         << setw(20) << "Email" << " | " 
         << setw(8) << "Role" << " |\n";
    cout << "+----+----------------------+----------------------+----------------------+----------+\n";
    
    // Isi tabel
    ProfileNode* current = profileHead;
    while (current) {
        json profile = current->profile.profileJson;
        cout << "| " << left << setw(3) << profile["id"] << " | " 
             << setw(20) << profile["username"].get<string>().substr(0, 20) << " | " 
             << setw(20) << profile["name"].get<string>().substr(0, 20) << " | " 
             << setw(20) << profile["email"].get<string>().substr(0, 20) << " | " 
             << setw(8) << profile["role"] << " |\n";
        current = current->next;
    }
    
    // Footer tabel
    cout << "+----+----------------------+----------------------+----------------------+----------+\n";
    
    // Hitung jumlah profile
    int count = 0;
    current = profileHead;
    while (current) {
        count++;
        current = current->next;
    }
    cout << "Total profile: " << count << endl;
}

void tambahProfileBaru(string username, string namaLengkap, string email, string role = "user") {
    ProfileNode* current = profileHead;
    while (current) {
        if (current->profile.profileJson["username"] == username) {
            cout << "⚠️  Username sudah terdaftar.\n";
            return;
        }
        current = current->next;
    }

    json newProfile = {
        {"id", nextProfileId++},
        {"username", username},
        {"name", namaLengkap},
        {"email", email},
        {"role", role},
        {"matakuliah", json::array()},
        {"password", ""},
        {"phone", ""}
    };

    ProfileNode* newProfileNode = new ProfileNode{{newProfile}, nullptr};

    if (!profileHead) profileHead = newProfileNode;
    else {
        current = profileHead;
        while (current->next) current = current->next;
        current->next = newProfileNode;
    }

    cout << "✅ Profile berhasil ditambahkan!\n";
}

void hapusProfile(string username) {
    ProfileNode* current = profileHead;
    ProfileNode* prev = nullptr;

    while (current) {
        if (current->profile.profileJson["username"] == username) {
            if (!prev) profileHead = current->next;
            else prev->next = current->next;

            delete current;
            cout << "🗑️  Profile berhasil dihapus.\n";
            return;
        }
        prev = current;
        current = current->next;
    }

    cout << "⚠️  Profile tidak ditemukan.\n";
}

void lihatDetailProfile(string username) {
    ProfileNode* current = profileHead;
    while (current) {
        if (current->profile.profileJson["username"] == username) {
            cout << "\n=== Detail Profile ===\n";
            cout << "Username     : " << current->profile.profileJson["username"] << endl;
            cout << "Nama Lengkap : " << current->profile.profileJson["name"] << endl;
            cout << "Email        : " << current->profile.profileJson["email"] << endl;
            cout << "Role         : " << current->profile.profileJson["role"] << endl;
            return;
        }
        current = current->next;
    }
    cout << "⚠️  Profile tidak ditemukan.\n";
}

void tampilkanProfileMenu() {
    if (isAdmin()) {
        cout << "\n=== MENU MANAJEMEN PROFILE (ADMIN) ===\n";
        cout << "1. Tambah Profile\n";
        cout << "2. Hapus Profile\n";
        cout << "3. Lihat Detail Profile\n";
        cout << "4. Tampilkan Daftar Profile\n";
        cout << "5. Kembali ke Menu Utama\n";
        cout << "Pilih opsi (1-5): ";
    } else {
        // Untuk non-admin, langsung tampilkan detail profile mereka
        lihatDetailProfile(currentSession.username);
        cout << "\nTekan enter untuk kembali...";
        cin.ignore();
        return;
    }
}


int mainProfile() {
    const string jsonFile = "database.json";
    loadProfilesFromJson(jsonFile);

    // Jika bukan admin, langsung tampilkan profile dan kembali
    if (!isAdmin()) {
        lihatDetailProfile(currentSession.username);
        cout << "\nTekan enter untuk kembali...";
        cin.ignore();
        return 0;
    }

    // Hanya admin yang bisa mengakses menu manajemen profile
    int pilihan;
    string username, namaLengkap, email;

    do {
        tampilkanProfileMenu();
        
        // Jika bukan admin, fungsi akan kembali setelah menampilkan profile
        if (!isAdmin()) break;
        
        cin >> pilihan;
        cin.ignore();

        switch (pilihan) {
            case 1:
                cout << "\n== Tambah Profile ==\n";
                cout << "Username     : ";
                getline(cin, username);
                cout << "Nama Lengkap : ";
                getline(cin, namaLengkap);
                cout << "Email        : ";
                getline(cin, email);
                tambahProfileBaru(username, namaLengkap, email);
                saveProfilesToJson(jsonFile);
                break;

            case 2:
                cout << "\n== Hapus Profile ==\n";
                cout << "Masukkan username yang ingin dihapus: ";
                getline(cin, username);
                hapusProfile(username);
                saveProfilesToJson(jsonFile);
                break;

            case 3:
                cout << "\n== Lihat Detail Profile ==\n";
                cout << "Masukkan username yang ingin dilihat: ";
                getline(cin, username);
                lihatDetailProfile(username);
                break;
                
            case 4:
                tampilkanDaftarProfile();
                break;

            case 5:
                cout << "Kembali ke menu utama...\n";
                break;

            default:
                cout << "❌ Pilihan tidak valid. Silakan pilih 1-5.\n";
        }

    } while (pilihan != 5 && isAdmin());

    return 0;
}