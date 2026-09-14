# Pharmaceutical Laboratory Management System 

A C++ console application that manages a pharmaceutical laboratory's drugs. Each
drug is stored as a node in a **Binary Search Tree keyed by drug number**, and each
node keeps a **doubly linked list of the products** needed to manufacture it. The
menu lets a user add, search, update, and delete drugs, manage their ingredient
lists, group drugs by category, and export the whole catalogue to a text file.

This repository contains a **clean refactored rewrite** of the original
single-file program: the tree logic is encapsulated in its own class, several
latent bugs are fixed, and the code is written in portable **C++98** so it builds
on both modern compilers and the classic **Dev-C++ (GCC 3.4.2)** toolchain.

---

## Table of Contents

- [Features](#features)
- [What changed in this refactor](#what-changed-in-this-refactor)
- [Code structure](#code-structure)
- [Getting started](#getting-started)
- [Menu reference](#menu-reference)
- [Example session](#example-session)
- [Saved file format](#saved-file-format)
- [Testing](#testing)
- [Credits](#credits)

---

## Features

**Drug management**
- Add a drug with a unique number, label, unit price, and category.
- List every drug, sorted by number (in-order tree traversal).
- Show the full details of one drug, including its ingredient list.
- Delete a drug by number (handles all three BST deletion cases).

**Product (ingredient) management**
- Attach a list of products, each with a quantity, to a drug.
- Remove a single product from a drug.
- Find every drug that uses a given product.
- Rename a product across every drug in one pass.

**Category management**
- List all unique categories.
- Group and display drugs by category.
- Delete every drug belonging to a category.

**Persistence**
- Save the whole catalogue to a text file as a fresh snapshot.

---

## What changed in this refactor

The behaviour is the same as the original program, but the implementation was
reorganised and three real bugs were fixed.

### Structure

- **Encapsulation.** All tree operations and the root pointer now live inside a
  single `DrugInventory` class. `main()` only prints the menu and reads input —
  it never touches a `Drug*` directly.
- **Single responsibility helpers.** Insertion, search, minimum, deletion, the
  in-order walks, and the category grouping are small private static methods,
  each doing one thing.
- **Input handling.** Two helpers (`promptValue<T>` and `promptLine`) replace the
  original mix of `cin >>` and `getline`. The new helpers consume the rest of the
  line after a numeric read, so stray newlines can no longer desynchronise the
  prompts, and corrupt input is cleared instead of sending the menu into an
  infinite loop.
- **Portable data types.** Prices use `double`; the code avoids `nullptr`, range-
  based `for`, `auto`, and `enum class` so it compiles under C++98.

### Bug fixes

| # | Original behaviour | Fix |
|---|--------------------|-----|
| 1 | `displayDrugCategories` used a function-local `static set` plus a dummy `root->number == root->number` guard. Categories printed on every recursive call and persisted between invocations. | Collect categories into a local `set<string>` in one traversal, then print once. |
| 2 | `saveDrugTreeToFile` opened the file with `ios::app`, and opened/closed it on **every** node — slow, and repeated saves duplicated the data. | Open the stream once with truncation, write the whole tree, close once. |
| 3 | `deleteDrugsByCategory` deleted nodes in the middle of a traversal by re-searching the tree from the root, which is fragile. | Collect the matching drug numbers first, then delete them one by one. |
| 4 | `removeProduct` gave the same message whether the drug or the product was missing. | Return a `RemoveOutcome` (`Remove_Ok` / `Remove_DrugMissing` / `Remove_ProductMissing`) and report the exact cause. |
| 5 | `displayDrugs` printed only the label. | The list now shows number, label, category, and price. |

The tree is also fully freed in the `DrugInventory` destructor, and a rejected
duplicate is deleted instead of being leaked.

---

## Code structure

Everything lives in one file, `drug_inventory.cpp`, laid out top to bottom as:

```
Product            // ingredient: name + quantity
Drug               // BST node: number, label, price, category, product list, left/right
RemoveOutcome      // enum returned by removeProduct
DrugInventory      // the tree + every operation on it
  ├─ addDrug / insert / makeNode
  ├─ contains / find / findMin
  ├─ deleteDrug / erase
  ├─ addProducts / removeProduct
  ├─ listAll / showDrug          (in-order traversal)
  ├─ listDrugsUsing / renameProduct
  ├─ listCategories / showGroupedByCategory
  ├─ saveToFile / writeInOrder
  └─ deleteCategory / collectNumbers
printMenu, promptValue, promptLine, readProducts   // UI helpers
main()                                             // the menu loop
```

- `list`, `map`, `set`, and `vector` come from the C++ STL.
- The BST key is the drug **number**; in-order traversal therefore lists drugs in
  ascending number order.

---

## Getting started

### With Dev-C++ (or any compiler)

1. Open `drug_inventory.cpp` in Dev-C++.
2. Press **F11** (Compile & Run), or use Execute → Compile & Run.

### With g++ / clang (command line)

```bash
g++ drug_inventory.cpp -o drug_inventory
./drug_inventory        # Linux / macOS
drug_inventory.exe      # Windows
```

The source is plain C++98, so no `-std=` flag is required — it builds with old
and new toolchains alike.

> **Note for very old MinGW (GCC 3.4.2, the compiler that ships with classic
> Dev-C++ 4.9.9.2):** on some Windows 10/11 machines `collect2.exe` aborts during
> linking with `Internal error: Aborted (program collect2)`. The compiler itself
> is fine — only the link wrapper crashes. If you hit it from the command line,
> compile to an object file and link with `ld` directly:
>
> ```bat
> c++.exe -c drug_inventory.cpp -o drug_inventory.o
> ld.exe -Bdynamic -o drug_inventory.exe ^
>   "<Dev-Cpp>\lib\crt2.o" "<Dev-Cpp>\lib\gcc\mingw32\3.4.2\crtbegin.o" ^
>   -L"<Dev-Cpp>\lib\gcc\mingw32\3.4.2" -L"<Dev-Cpp>\lib\gcc" ^
>   -L"<Dev-Cpp>\lib\gcc\mingw32\3.4.2\..\..\..\..\mingw32\lib" ^
>   -L"<Dev-Cpp>\lib\gcc\mingw32\3.4.2\..\..\.." ^
>   drug_inventory.o -lstdc++ -lmingw32 -lgcc -lmoldname -lmingwex -lmsvcrt ^
>   -luser32 -lkernel32 -ladvapi32 -lshell32 -lmingw32 -lgcc -lmoldname ^
>   -lmingwex -lmsvcrt "<Dev-Cpp>\lib\gcc\mingw32\3.4.2\crtend.o"
> ```
>
> Or, more simply, install a modern compiler (the current Dev-C++ / Orwell fork,
> or MinGW-w64) where the standard build works out of the box.

---

## Menu reference

```
--- Pharmaceutical Laboratory Management ---
 1. Add a new drug
 2. Add products to a drug
 3. List all drugs
 4. Show a drug's details
 5. Delete a drug
 6. Remove a product from a drug
 7. Find drugs that use a product
 8. Rename a product in every drug
 9. List drug categories
10. Group drugs by category
11. Save the drugs to a file
12. Delete every drug in a category
13. Exit
```

---

## Example session

```
1                                  # add a drug
50
Paracetamol
4.5
Analgesic
Drug added.

1                                  # add another
30
Amoxicillin
12
Antibiotic
Drug added.

2                                  # add products to drug 50
50
2
Paracetamol powder
10
Starch
5
Products added.

3                                  # list all (in-order by number)
#30  Amoxicillin  [Antibiotic]  12
#50  Paracetamol  [Analgesic]  4.5

4                                  # show drug 50
50
Number  : 50
Label   : Paracetamol
Price   : 4.5
Category: Analgesic
Required products:
  - Paracetamol powder x10
  - Starch x5

7                                  # which drugs use this product?
Paracetamol powder
  #50  Paracetamol

8                                  # rename a product everywhere
Starch
Corn starch
1 product occurrence(s) renamed.

9                                  # list unique categories
Drug categories in the laboratory:
  - Analgesic
  - Antibiotic

12                                 # delete a whole category
Antibiotic
1 drug(s) deleted.

13                                 # exit
Goodbye.
```

---

## Saved file format

Option **11** writes a snapshot such as `drugs.txt`:

```
Drug Number: 50
Label: Paracetamol
Unit Price: 4.5
Category: Analgesic
Products:
- Paracetamol powder (Quantity: 10)
- Corn starch (Quantity: 5)
----------------------
```

Each save overwrites the file with a fresh snapshot (no duplicated appends).

---

## Testing

The program was built with the Dev-C++ MinGW toolchain (GCC 3.4.2) and exercised
end to end. Verified behaviours include:

- Drugs listed in ascending number order after an out-of-order insertion sequence.
- Duplicate drug numbers rejected (`A drug numbered 50 already exists.`).
- BST deletion for leaf, single-child, and two-child nodes.
- Adding/removing products, including the "drug missing" vs "product missing" paths.
- Category listing returns each category exactly once.
- Grouping by category and deleting a category both behave correctly.
- Saving to file produces a sorted, non-duplicated snapshot.
- Invalid numeric input is rejected with a re-prompt instead of looping forever.

---

## Credits

- Original program and initial README: **harkati said dhiya eddine**
  ([github.com/HarkatiSaidDhiyaEddine](https://github.com/HarkatiSaidDhiyaEddine)).

