// Pharmaceutical Laboratory Management
//
// Refactored version of the original single-file program. The drug catalog
// (a binary search tree keyed by drug number) and every operation on it now
// live inside DrugInventory, so main() only shows the menu and reads input.
//
// Written for C++98 so it also builds on the classic Dev-C++ (GCC 3.4.2).

#include <iostream>
#include <string>
#include <list>
#include <map>
#include <set>
#include <vector>
#include <fstream>
#include <limits>

using namespace std;

struct Product {
    string name;
    int quantity;

    Product() : quantity(0) {}
};

struct Drug {
    int number;
    string label;
    double unitPrice;
    string category;
    list<Product> products;
    Drug* left;
    Drug* right;

    Drug() : number(0), unitPrice(0.0), left(0), right(0) {}
};

// Plain enum (no "enum class") because the installed compiler predates C++11.
enum RemoveOutcome {
    Remove_Ok,
    Remove_DrugMissing,
    Remove_ProductMissing
};

// One menu action per public method; the tree layout stays private.
class DrugInventory {
public:
    ~DrugInventory();

    bool addDrug(int number, const string& label, double unitPrice, const string& category);
    bool contains(int number) const;
    void addProducts(int number, list<Product> products);

    void listAll() const;
    void showDrug(int number) const;
    bool deleteDrug(int number);
    RemoveOutcome removeProduct(int number, const string& productName);

    void listDrugsUsing(const string& productName) const;
    int renameProduct(const string& oldName, const string& newName);
    void listCategories() const;
    void showGroupedByCategory() const;
    bool saveToFile(const string& filename) const;
    int deleteCategory(const string& category);

private:
    Drug* root;

    static Drug* makeNode(int number, const string& label, double unitPrice, const string& category);
    static void destroyTree(Drug* node);
    static Drug* insert(Drug* node, Drug* fresh, bool& inserted);
    static Drug* find(Drug* node, int number);
    static Drug* findMin(Drug* node);
    static Drug* erase(Drug* node, int number, bool& removed);

    static void printInOrder(const Drug* node);
    static bool usesProduct(const Drug* node, const string& productName);
    static bool printDrugsUsing(const Drug* node, const string& productName);
    static int renameInTree(Drug* node, const string& oldName, const string& newName);
    static void collectCategories(const Drug* node, set<string>& categories);
    static void groupByCategory(const Drug* node, map<string, vector<const Drug*> >& groups);
    static void writeInOrder(const Drug* node, ostream& out);
    static void collectNumbers(const Drug* node, const string& category, vector<int>& numbers);

public:
    DrugInventory() : root(0) {}
};
DrugInventory::~DrugInventory() {
    destroyTree(root);
}

Drug* DrugInventory::makeNode(int number, const string& label, double unitPrice, const string& category) {
    Drug* node = new Drug;
    node->number = number;
    node->label = label;
    node->unitPrice = unitPrice;
    node->category = category;
    return node;
}

void DrugInventory::destroyTree(Drug* node) {
    if (!node) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}

Drug* DrugInventory::insert(Drug* node, Drug* fresh, bool& inserted) {
    if (!node) {
        inserted = true;
        return fresh;
    }
    if (fresh->number < node->number) {
        node->left = insert(node->left, fresh, inserted);
    } else if (fresh->number > node->number) {
        node->right = insert(node->right, fresh, inserted);
    } else {
        inserted = false; // duplicate key: keep the existing drug
    }
    return node;
}

Drug* DrugInventory::find(Drug* node, int number) {
    while (node && node->number != number) {
        node = (number < node->number) ? node->left : node->right;
    }
    return node;
}

Drug* DrugInventory::findMin(Drug* node) {
    while (node && node->left) {
        node = node->left;
    }
    return node;
}

Drug* DrugInventory::erase(Drug* node, int number, bool& removed) {
    if (!node) return 0;

    if (number < node->number) {
        node->left = erase(node->left, number, removed);
        return node;
    }
    if (number > node->number) {
        node->right = erase(node->right, number, removed);
        return node;
    }

    removed = true;

    if (!node->left) {           // zero or one child: splice the survivor up
        Drug* right = node->right;
        delete node;
        return right;
    }
    if (!node->right) {
        Drug* left = node->left;
        delete node;
        return left;
    }

    // Two children: copy the in-order successor's data up, then delete it.
    Drug* successor = findMin(node->right);
    node->number = successor->number;
    node->label = successor->label;
    node->unitPrice = successor->unitPrice;
    node->category = successor->category;
    node->products = successor->products;
    node->right = erase(node->right, successor->number, removed);
    return node;
}

bool DrugInventory::addDrug(int number, const string& label, double unitPrice, const string& category) {
    Drug* fresh = makeNode(number, label, unitPrice, category);
    bool inserted = false;
    root = insert(root, fresh, inserted);
    if (!inserted) {
        delete fresh;
    }
    return inserted;
}

bool DrugInventory::contains(int number) const {
    return find(root, number) != 0;
}

void DrugInventory::addProducts(int number, list<Product> products) {
    Drug* drug = find(root, number);
    if (!drug) return;
    // splice moves the nodes instead of copying each Product
    drug->products.splice(drug->products.end(), products);
}

void DrugInventory::listAll() const {
    if (!root) {
        cout << "No drugs recorded yet.\n";
        return;
    }
    printInOrder(root);
}

void DrugInventory::printInOrder(const Drug* node) {
    if (!node) return;
    printInOrder(node->left);
    cout << "#" << node->number << "  " << node->label
         << "  [" << node->category << "]  " << node->unitPrice << "\n";
    printInOrder(node->right);
}

void DrugInventory::showDrug(int number) const {
    const Drug* drug = find(root, number);
    if (!drug) {
        cout << "No drug numbered " << number << ".\n";
        return;
    }

    cout << "Number  : " << drug->number << "\n"
         << "Label   : " << drug->label << "\n"
         << "Price   : " << drug->unitPrice << "\n"
         << "Category: " << drug->category << "\n";

    if (drug->products.empty()) {
        cout << "No products recorded for this drug.\n";
        return;
    }
    cout << "Required products:\n";
    list<Product>::const_iterator it;
    for (it = drug->products.begin(); it != drug->products.end(); ++it) {
        cout << "  - " << it->name << " x" << it->quantity << "\n";
    }
}
bool DrugInventory::deleteDrug(int number) {
    bool removed = false;
    root = erase(root, number, removed);
    return removed;
}

RemoveOutcome DrugInventory::removeProduct(int number, const string& productName) {
    Drug* drug = find(root, number);
    if (!drug) return Remove_DrugMissing;

    list<Product>::iterator it = drug->products.begin();
    while (it != drug->products.end()) {
        if (it->name == productName) {
            drug->products.erase(it);
            return Remove_Ok;
        }
        ++it;
    }
    return Remove_ProductMissing;
}

bool DrugInventory::usesProduct(const Drug* node, const string& productName) {
    list<Product>::const_iterator it;
    for (it = node->products.begin(); it != node->products.end(); ++it) {
        if (it->name == productName) return true;
    }
    return false;
}

bool DrugInventory::printDrugsUsing(const Drug* node, const string& productName) {
    if (!node) return false;

    bool foundLeft = printDrugsUsing(node->left, productName);

    bool foundHere = usesProduct(node, productName);
    if (foundHere) {
        cout << "  #" << node->number << "  " << node->label << "\n";
    }

    bool foundRight = printDrugsUsing(node->right, productName);
    return foundLeft || foundHere || foundRight;
}

void DrugInventory::listDrugsUsing(const string& productName) const {
    if (!printDrugsUsing(root, productName)) {
        cout << "No drug uses \"" << productName << "\".\n";
    }
}

int DrugInventory::renameInTree(Drug* node, const string& oldName, const string& newName) {
    if (!node) return 0;

    int changed = 0;
    list<Product>::iterator it;
    for (it = node->products.begin(); it != node->products.end(); ++it) {
        if (it->name == oldName) {
            it->name = newName;
            ++changed;
        }
    }
    return changed + renameInTree(node->left, oldName, newName)
                   + renameInTree(node->right, oldName, newName);
}

int DrugInventory::renameProduct(const string& oldName, const string& newName) {
    return renameInTree(root, oldName, newName);
}

void DrugInventory::collectCategories(const Drug* node, set<string>& categories) {
    if (!node) return;
    collectCategories(node->left, categories);
    categories.insert(node->category);
    collectCategories(node->right, categories);
}

void DrugInventory::listCategories() const {
    set<string> categories;
    collectCategories(root, categories);
    if (categories.empty()) {
        cout << "No categories yet.\n";
        return;
    }
    cout << "Drug categories in the laboratory:\n";
    set<string>::const_iterator it;
    for (it = categories.begin(); it != categories.end(); ++it) {
        cout << "  - " << *it << "\n";
    }
}

void DrugInventory::groupByCategory(const Drug* node, map<string, vector<const Drug*> >& groups) {
    if (!node) return;
    groupByCategory(node->left, groups);
    groups[node->category].push_back(node);
    groupByCategory(node->right, groups);
}

void DrugInventory::showGroupedByCategory() const {
    if (!root) {
        cout << "The drug tree is empty.\n";
        return;
    }

    map<string, vector<const Drug*> > groups;
    groupByCategory(root, groups);

    cout << "Drugs organized by category:\n";
    map<string, vector<const Drug*> >::const_iterator it;
    for (it = groups.begin(); it != groups.end(); ++it) {
        cout << "\n" << it->first << "\n";
        const vector<const Drug*>& drugs = it->second;
        for (unsigned int i = 0; i < drugs.size(); ++i) {
            const Drug* drug = drugs[i];
            cout << "  - #" << drug->number << "  " << drug->label
                 << "  (" << drug->unitPrice << ")\n";
        }
    }
}

void DrugInventory::writeInOrder(const Drug* node, ostream& out) {
    if (!node) return;
    writeInOrder(node->left, out);

    out << "Drug Number: " << node->number << "\n"
        << "Label: " << node->label << "\n"
        << "Unit Price: " << node->unitPrice << "\n"
        << "Category: " << node->category << "\n"
        << "Products:\n";
    list<Product>::const_iterator it;
    for (it = node->products.begin(); it != node->products.end(); ++it) {
        out << "- " << it->name << " (Quantity: " << it->quantity << ")\n";
    }
    out << "----------------------\n";

    writeInOrder(node->right, out);
}

bool DrugInventory::saveToFile(const string& filename) const {
    // Open once and truncate, so each save writes a fresh snapshot.
    // c_str() because the old libstdc++ has no ofstream(string) overload.
    ofstream out(filename.c_str());
    if (!out) return false;
    writeInOrder(root, out);
    return true;
}

void DrugInventory::collectNumbers(const Drug* node, const string& category, vector<int>& numbers) {
    if (!node) return;
    collectNumbers(node->left, category, numbers);
    if (node->category == category) numbers.push_back(node->number);
    collectNumbers(node->right, category, numbers);
}

int DrugInventory::deleteCategory(const string& category) {
    // Collect the keys first, then delete them by number: mutating the tree
    // in the middle of a traversal would invalidate the nodes we're walking.
    vector<int> numbers;
    collectNumbers(root, category, numbers);
    for (unsigned int i = 0; i < numbers.size(); ++i) {
        deleteDrug(numbers[i]);
    }
    return static_cast<int>(numbers.size());
}
namespace {

void printMenu() {
    cout << "\n--- Pharmaceutical Laboratory Management ---\n"
            " 1. Add a new drug\n"
            " 2. Add products to a drug\n"
            " 3. List all drugs\n"
            " 4. Show a drug's details\n"
            " 5. Delete a drug\n"
            " 6. Remove a product from a drug\n"
            " 7. Find drugs that use a product\n"
            " 8. Rename a product in every drug\n"
            " 9. List drug categories\n"
            "10. Group drugs by category\n"
            "11. Save the drugs to a file\n"
            "12. Delete every drug in a category\n"
            "13. Exit\n";
}

// Reads one value of type T and clears bad input, so a stray letter can never
// trap the menu in an infinite loop.
template <typename T>
T promptValue(const string& prompt) {
    cout << prompt;
    T value = T();
    while (!(cin >> value)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Please enter a valid value: ";
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // drop the rest of the line
    return value;
}

// Reads a whole line, so names may contain spaces.
string promptLine(const string& prompt) {
    cout << prompt;
    string value;
    getline(cin, value);
    return value;
}

list<Product> readProducts() {
    int count = promptValue<int>("How many products? ");
    list<Product> products;
    for (int i = 0; i < count; ++i) {
        Product product;
        product.name = promptLine("  Product name: ");
        product.quantity = promptValue<int>("  Quantity: ");
        products.push_back(product);
    }
    return products;
}

} // namespace

int main() {
    DrugInventory inventory;

    while (true) {
        printMenu();
        int choice = promptValue<int>("Enter your choice: ");

        switch (choice) {
        case 1: {
            int number = promptValue<int>("Drug number: ");
            string label = promptLine("Label: ");
            double price = promptValue<double>("Unit price: ");
            string category = promptLine("Category: ");
            if (inventory.addDrug(number, label, price, category)) {
                cout << "Drug added.\n";
            } else {
                cout << "A drug numbered " << number << " already exists.\n";
            }
            break;
        }
        case 2: {
            int number = promptValue<int>("Drug number: ");
            if (!inventory.contains(number)) {
                cout << "No drug numbered " << number << ".\n";
                break;
            }
            inventory.addProducts(number, readProducts());
            cout << "Products added.\n";
            break;
        }
        case 3:
            inventory.listAll();
            break;
        case 4: {
            int number = promptValue<int>("Drug number: ");
            inventory.showDrug(number);
            break;
        }
        case 5: {
            int number = promptValue<int>("Drug number: ");
            cout << (inventory.deleteDrug(number) ? "Drug deleted.\n"
                                                  : "No such drug.\n");
            break;
        }
        case 6: {
            int number = promptValue<int>("Drug number: ");
            string product = promptLine("Product to remove: ");
            switch (inventory.removeProduct(number, product)) {
            case Remove_Ok:
                cout << "Product removed.\n";
                break;
            case Remove_DrugMissing:
                cout << "No such drug.\n";
                break;
            case Remove_ProductMissing:
                cout << "That product is not listed for this drug.\n";
                break;
            }
            break;
        }
        case 7: {
            string product = promptLine("Product name: ");
            inventory.listDrugsUsing(product);
            break;
        }
        case 8: {
            string oldName = promptLine("Product to replace: ");
            string newName = promptLine("New name: ");
            int changed = inventory.renameProduct(oldName, newName);
            cout << changed << " product occurrence(s) renamed.\n";
            break;
        }
        case 9:
            inventory.listCategories();
            break;
        case 10:
            inventory.showGroupedByCategory();
            break;
        case 11: {
            string filename = promptLine("File name: ");
            cout << (inventory.saveToFile(filename)
                         ? "Saved to " + filename + ".\n"
                         : "Could not open " + filename + " for writing.\n");
            break;
        }
        case 12: {
            string category = promptLine("Category to delete: ");
            int removed = inventory.deleteCategory(category);
            cout << removed << " drug(s) deleted.\n";
            break;
        }
        case 13:
            cout << "Goodbye.\n";
            return 0;
        default:
            cout << "Invalid choice, try again.\n";
        }
    }
}
