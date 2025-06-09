#include <iostream>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

struct Node {
    string name;
    map<string, shared_ptr<Node>> children;

    Node(const string& n) : name(n) {}
};

void insert(shared_ptr<Node> root, const vector<string>& path) {
    shared_ptr<Node> current = root;
    for (const string& part : path) {
        if (current->children.find(part) == current->children.end()) {
            current->children[part] = make_shared<Node>(part);
        }
        current = current->children[part];
    }
}

json nodeToJson(shared_ptr<Node> node) {
    json j;
    for (auto& childPair : node->children) {
        j[childPair.first] = nodeToJson(childPair.second);
    }
    return j;
}

shared_ptr<Node> jsonToNode(const json& j, const string& nodeName = "") {
    auto node = make_shared<Node>(nodeName);
    for (auto it = j.begin(); it != j.end(); ++it) {
        node->children[it.key()] = jsonToNode(it.value(), it.key());
    }
    return node;
}

// Visualisasi tree dengan garis cabang
void displayTreePretty(shared_ptr<Node> node, string prefix = "", bool isLast = true) {
    if (!node->name.empty()) {
        cout << prefix;
        if (isLast) {
            cout << "+-- ";
            prefix += "    ";
        } else {
            cout << "|-- ";
            prefix += "|   ";
        }
        cout << node->name << "\n";
    }

    auto it = node->children.begin();
    while (it != node->children.end()) {
        auto nextIt = it;
        ++nextIt;
        bool lastChild = (nextIt == node->children.end());
        displayTreePretty(it->second, prefix, lastChild);
        it = nextIt;
    }
}

int main() {
    shared_ptr<Node> root = make_shared<Node>("");

    ifstream inFile("materi.json");
    if (inFile.is_open()) {
        json j;
        inFile >> j;
        inFile.close();

        root = jsonToNode(j);

        cout << "Materi dari file materi.json:\n";
        displayTreePretty(root);
    } else {
        cout << "File materi.json tidak ditemukan. Silakan input materi baru.\n";
        cout << "Input data materi (ketik 'selesai' untuk berhenti)\n";
    }

    while (true) {
        vector<string> path;
        cout << "\nMasukkan judul mata kuliah (atau 'selesai'): ";
        string input;
        getline(cin, input);
        if (input == "selesai") break;
        path.push_back(input);

        while (true) {
            cout << "Masukkan subjudul untuk '" << path.back() << "' (kosong untuk selesai): ";
            getline(cin, input);
            if (input.empty()) break;
            path.push_back(input);
        }

        insert(root, path);
        cout << "\nMateri berhasil ditambahkan. Visualisasi materi terkini:\n";
        displayTreePretty(root);
    }


    json j = nodeToJson(root);
    ofstream outFile("materi.json");
    outFile << j.dump(4);
    outFile.close();

    cout << "\nData materi tersimpan ke file materi.json\n";

    return 0;
}
