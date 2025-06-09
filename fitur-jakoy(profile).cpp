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