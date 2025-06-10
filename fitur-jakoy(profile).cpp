#include <iostream>
#include <string>
using namespace std;

struct User {
    string username;
    string namaLengkap;
    string email;
};

struct Node {
    User user;
    Node* next;
};

Node* head = nullptr;

void tambahUserBaru(string username, string namaLengkap, string email) {
    // cek duplikat
    Node* current = head;
    while (current != nullptr) {
        if (current->user.username == username) {
            cout << "⚠️  Username sudah terdaftar.\n";
            return;
        }
        current = current->next;
    }

    Node* newNode = new Node{{username, namaLengkap, email}, nullptr};

    if (head == nullptr) {
        head = newNode;
    } else {
        current = head;
        while (current->next != nullptr) {
            current = current->next;
        }
        current->next = newNode;
    }

    cout << "✅ User berhasil ditambahkan!\n";
}

void hapusUser(string username) {
    Node* current = head;
    Node* prev = nullptr;

    while (current != nullptr) {
        if (current->user.username == username) {
            if (prev == nullptr) {
                head = current->next;
            } else {
                prev->next = current->next;
            }
            delete current;
            cout << "🗑️  User berhasil dihapus.\n";
            return;
        }
        prev = current;
        current = current->next;
    }

    cout << "⚠️  User tidak ditemukan.\n";
}

void lihatProfil(string username) {
    Node* current = head;
    while (current != nullptr) {
        if (current->user.username == username) {
            cout << "\n=== Profil User ===\n";
            cout << "Username     : " << current->user.username << endl;
            cout << "Nama Lengkap : " << current->user.namaLengkap << endl;
            cout << "Email        : " << current->user.email << endl;
            return;
        }
        current = current->next;
    }
    cout << "⚠️  User tidak ditemukan.\n";
}

void tampilkanMenu() {
    cout << "\n=== MENU MANAJEMEN USER ===\n";
    cout << "1. Tambah User\n";
    cout << "2. Hapus User\n";
    cout << "3. Lihat Profil User\n";
    cout << "4. Keluar\n";
    cout << "Pilih opsi (1-4): ";
}


// MAIN MENU Disini ya
int main() {
    int pilihan;
    string username, namaLengkap, email;

    do {
        tampilkanMenu();
        cin >> pilihan;
        cin.ignore();

        switch (pilihan) {
            case 1:
                cout << "\n== Tambah User ==\n";
                cout << "Username     : ";
                getline(cin, username);
                cout << "Nama Lengkap : ";
                getline(cin, namaLengkap);
                cout << "Email        : ";
                getline(cin, email);
                tambahUserBaru(username, namaLengkap, email);
                break;

            case 2:
                cout << "\n== Hapus User ==\n";
                cout << "Masukkan username yang ingin dihapus: ";
                getline(cin, username);
                hapusUser(username);
                break;

            case 3:
                cout << "\n== Lihat Profil ==\n";
                cout << "Masukkan username yang ingin dilihat: ";
                getline(cin, username);
                lihatProfil(username);
                break;

            case 4:
                cout << "👋 Keluar dari program. Sampai jumpa!\n";
                break;

            default:
                cout << "❌ Pilihan tidak valid. Silakan pilih 1-4.\n";
        }

    } while (pilihan != 4);

    return 0;
}