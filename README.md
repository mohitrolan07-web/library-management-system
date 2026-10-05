# Library Management System (C++)

A console-based library management application written in modern C++17. It manages books and magazines, library members, issuing and returning of items with automatic late-fine calculation, and saves all data to disk between runs.

## Features

- Add books and magazines, and register members
- Issue and return items (an item can only be issued to one member at a time)
- Late-fine calculation: Rs 2 per day after the loan period (books: 14 days, magazines: 7 days)
- Case-insensitive search by title or author/issue
- List all items with availability and the list of members with their issued counts
- Data persists in plain text files (`items.txt`, `members.txt`)

## Concepts used

- **OOP:** abstract base class `Item` with derived `Book` and `Magazine`, using virtual functions (polymorphism) for type, creator and loan period
- **STL:** `vector`, `map`, `algorithm`, string streams
- **Memory safety:** `std::unique_ptr` for owning items (no manual `new`/`delete`)
- **File handling:** `ifstream` / `ofstream` with a simple `|`-separated format

## Build and run

```bash
g++ -std=c++17 -Wall -Wextra -o lms main.cpp
./lms          # on Windows: lms.exe
```

## Possible improvements

- Split the code into header and source files
- Store real issue and due dates instead of asking for "days kept"
- Add unit tests and a CMake build

## License

MIT
