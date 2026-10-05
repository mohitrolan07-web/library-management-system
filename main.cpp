// Library Management System - console application in C++17
// Concepts used: classes, inheritance, polymorphism (virtual functions),
// STL containers (vector, map), smart pointers, file handling.

#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// ---------------------------------------------------------------- Items
class Item {
protected:
    int id;
    string title;
    bool issued = false;
    int issuedTo = -1;  // member id, -1 if available

public:
    Item(int id, string title) : id(id), title(std::move(title)) {}
    virtual ~Item() = default;

    int getId() const { return id; }
    const string& getTitle() const { return title; }
    bool isIssued() const { return issued; }
    int getIssuedTo() const { return issuedTo; }

    void issueTo(int memberId) { issued = true; issuedTo = memberId; }
    void giveBack() { issued = false; issuedTo = -1; }

    virtual string type() const = 0;      // "Book" or "Magazine"
    virtual string creator() const = 0;   // author / issue label
    virtual int loanDays() const = 0;     // allowed days before fine starts
};

class Book : public Item {
    string author;

public:
    Book(int id, string title, string author)
        : Item(id, std::move(title)), author(std::move(author)) {}
    string type() const override { return "Book"; }
    string creator() const override { return author; }
    int loanDays() const override { return 14; }
};

class Magazine : public Item {
    string issueLabel;

public:
    Magazine(int id, string title, string issueLabel)
        : Item(id, std::move(title)), issueLabel(std::move(issueLabel)) {}
    string type() const override { return "Magazine"; }
    string creator() const override { return issueLabel; }
    int loanDays() const override { return 7; }
};

// -------------------------------------------------------------- Library
class Library {
    vector<unique_ptr<Item>> items;
    map<int, string> members;  // member id -> name
    int nextItemId = 1;
    int nextMemberId = 1;
    static constexpr int FINE_PER_DAY = 2;  // rupees

    static string clean(string s) {  // '|' is our file separator
        replace(s.begin(), s.end(), '|', '/');
        return s;
    }
    static string lower(string s) {
        transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return tolower(c); });
        return s;
    }

public:
    Item* findItem(int id) {
        for (auto& it : items)
            if (it->getId() == id) return it.get();
        return nullptr;
    }

    int addBook(const string& title, const string& author) {
        items.push_back(make_unique<Book>(nextItemId, clean(title), clean(author)));
        return nextItemId++;
    }
    int addMagazine(const string& title, const string& issueLabel) {
        items.push_back(make_unique<Magazine>(nextItemId, clean(title), clean(issueLabel)));
        return nextItemId++;
    }
    int addMember(const string& name) {
        members[nextMemberId] = clean(name);
        return nextMemberId++;
    }

    // Returns an error message, or "" on success.
    string issue(int itemId, int memberId) {
        Item* it = findItem(itemId);
        if (!it) return "No item with that ID.";
        if (!members.count(memberId)) return "No member with that ID.";
        if (it->isIssued()) return "Item is already issued.";
        it->issueTo(memberId);
        return "";
    }

    // Returns fine amount (>= 0), or -1 if the return is invalid.
    int giveBackItem(int itemId, int daysKept) {
        Item* it = findItem(itemId);
        if (!it || !it->isIssued()) return -1;
        int late = max(0, daysKept - it->loanDays());
        it->giveBack();
        return late * FINE_PER_DAY;
    }

    vector<const Item*> search(const string& query) const {
        vector<const Item*> out;
        string q = lower(query);
        for (const auto& it : items)
            if (lower(it->getTitle()).find(q) != string::npos ||
                lower(it->creator()).find(q) != string::npos)
                out.push_back(it.get());
        return out;
    }

    void printItem(const Item& it) const {
        cout << "  [" << it.getId() << "] " << it.type() << ": " << it.getTitle()
             << " (" << it.creator() << ") - ";
        if (it.isIssued()) {
            auto m = members.find(it.getIssuedTo());
            cout << "issued to " << (m != members.end() ? m->second : "unknown");
        } else {
            cout << "available";
        }
        cout << "\n";
    }

    void listItems() const {
        if (items.empty()) { cout << "  (no items yet)\n"; return; }
        for (const auto& it : items) printItem(*it);
    }
    void listMembers() const {
        if (members.empty()) { cout << "  (no members yet)\n"; return; }
        for (const auto& [id, name] : members) {
            int count = 0;
            for (const auto& it : items)
                if (it->isIssued() && it->getIssuedTo() == id) ++count;
            cout << "  [" << id << "] " << name << " - " << count << " item(s) issued\n";
        }
    }

    // ------------------------------------------------------ persistence
    // items.txt  : type|id|title|creator|issuedTo
    // members.txt: id|name
    void save(const string& dir = ".") const {
        ofstream fi(dir + "/items.txt"), fm(dir + "/members.txt");
        for (const auto& it : items)
            fi << it->type() << '|' << it->getId() << '|' << it->getTitle() << '|'
               << it->creator() << '|' << it->getIssuedTo() << '\n';
        for (const auto& [id, name] : members) fm << id << '|' << name << '\n';
    }

    void load(const string& dir = ".") {
        string line;
        ifstream fm(dir + "/members.txt");
        while (getline(fm, line)) {
            stringstream ss(line);
            string id, name;
            if (getline(ss, id, '|') && getline(ss, name)) {
                members[stoi(id)] = name;
                nextMemberId = max(nextMemberId, stoi(id) + 1);
            }
        }
        ifstream fi(dir + "/items.txt");
        while (getline(fi, line)) {
            stringstream ss(line);
            string type, id, title, creator, to;
            if (!(getline(ss, type, '|') && getline(ss, id, '|') && getline(ss, title, '|') &&
                  getline(ss, creator, '|') && getline(ss, to)))
                continue;
            int iid = stoi(id), memberId = stoi(to);
            if (type == "Book") items.push_back(make_unique<Book>(iid, title, creator));
            else items.push_back(make_unique<Magazine>(iid, title, creator));
            if (memberId != -1) items.back()->issueTo(memberId);
            nextItemId = max(nextItemId, iid + 1);
        }
    }
};

// ------------------------------------------------------------ UI helpers
string readLine(const string& prompt) {
    cout << prompt;
    string s;
    getline(cin, s);
    return s;
}

int readInt(const string& prompt) {
    while (true) {
        string s = readLine(prompt);
        try {
            size_t pos;
            int v = stoi(s, &pos);
            if (pos == s.size()) return v;
        } catch (...) {}
        cout << "Please enter a valid number.\n";
    }
}

void printMenu() {
    cout << "\n===== Library Management System =====\n"
         << " 1. Add book\n 2. Add magazine\n 3. Add member\n"
         << " 4. Issue item\n 5. Return item\n 6. Search\n"
         << " 7. List all items\n 8. List members\n 0. Save and exit\n";
}

int main() {
    Library lib;
    lib.load();

    while (true) {
        printMenu();
        int choice = readInt("Choose an option: ");
        switch (choice) {
            case 1: {
                string t = readLine("Title: "), a = readLine("Author: ");
                cout << "Added book with ID " << lib.addBook(t, a) << "\n";
                break;
            }
            case 2: {
                string t = readLine("Title: "), i = readLine("Issue (e.g. Oct 2026): ");
                cout << "Added magazine with ID " << lib.addMagazine(t, i) << "\n";
                break;
            }
            case 3:
                cout << "Added member with ID " << lib.addMember(readLine("Name: ")) << "\n";
                break;
            case 4: {
                int itemId = readInt("Item ID: "), memberId = readInt("Member ID: ");
                string err = lib.issue(itemId, memberId);
                cout << (err.empty() ? "Issued successfully." : err) << "\n";
                break;
            }
            case 5: {
                int itemId = readInt("Item ID: "), days = readInt("Days kept: ");
                int fine = lib.giveBackItem(itemId, days);
                if (fine < 0) cout << "That item is not currently issued.\n";
                else if (fine == 0) cout << "Returned. No fine.\n";
                else cout << "Returned late. Fine: Rs " << fine << "\n";
                break;
            }
            case 6: {
                auto res = lib.search(readLine("Search title/author: "));
                if (res.empty()) cout << "  No matches.\n";
                for (const Item* it : res) lib.printItem(*it);
                break;
            }
            case 7: lib.listItems(); break;
            case 8: lib.listMembers(); break;
            case 0:
                lib.save();
                cout << "Data saved. Goodbye!\n";
                return 0;
            default: cout << "Invalid option.\n";
        }
    }
}
