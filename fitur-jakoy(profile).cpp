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

void tambahUser(string username, string namaLengkap, string email) {
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