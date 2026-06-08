# 🎨 2D Terminal Graphics Editor

A menu-driven, interactive 2D graphics editor written in C using **ncurses**. It allows you to create, modify, delete, and list geometric shapes (Circles, Rectangles, Lines, and Triangles) rendered dynamically on a character-based canvas.

---

## 🚀 Features

*   **Interactive Menu UI:** Fully keyboard-driven menu system with visual highlights, scrollable dialogs, and instant canvas updates.
*   **Drawing Primitives:**
    *   **Circle:** Drawn using the Midpoint Circle Algorithm.
    *   **Line:** Rendered using Bresenham's Line Algorithm.
    *   **Rectangle:** Formed by four connecting lines.
    *   **Triangle:** Created by connecting three coordinates.
*   **Editor Controls:**
    *   Add shapes with custom parameters.
    *   Delete individual shapes by selecting them from a scrollable checklist.
    *   Modify existing shapes interactively.
    *   Scrollable list of all shapes with their dimensions and coordinates.
    *   Clear the canvas or refresh the layout.
*   **Dynamic Layout:** Automatically fits to terminal screens and supports terminal window resizing.

---

## 🛠️ Prerequisites & Setup

### Windows (MSYS2 / UCRT64)
1. Ensure you have **MSYS2** installed.
2. Install the **GCC** compiler and **ncurses** development libraries inside the UCRT64 terminal:
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-ncurses
   ```

### Linux / macOS
Install the ncurses development package:
*   **Ubuntu/Debian:** `sudo apt-get install libncurses5-dev libncursesw5-dev`
*   **macOS:** `brew install ncurses`

---

## 💻 How to Compile & Run

### Method 1: Using VS Code (Recommended)
This repository includes a `.vscode/` configuration directory set up for Windows (MSYS2 UCRT64).
1. Open the project folder in VS Code.
2. Install the **C/C++** extension by Microsoft.
3. Press **`F5`** to automatically compile and run the editor in a separate console window.

### Method 2: Using the Terminal (Makefile)
You can compile and run using the provided `Makefile`:

*   **To Compile:**
    ```bash
    make
    ```
*   **To Run:**
    ```bash
    make run
    ```
*   **To Clean Build Artifacts:**
    ```bash
    make clean
    ```

### Method 3: Direct Compilation Command
```bash
gcc -Wall -Wextra -std=c11 -O2 main.c shapes.c -o editor.exe -lncursesw -lm
```

---

## 🎮 How to Use

*   **Navigation:** Use the **UP** and **DOWN** arrow keys to move through menu items.
*   **Select:** Press **ENTER** to select a menu option or confirm coordinates.
*   **Cancel/Exit Dialogs:** Press **ESC** to cancel input dialogs.
*   **Exit Application:** Select `Exit` in the main menu or press **`Q`** on the main screen.

---

## 📂 Project Structure

```text
graphics_editor/
├── .vscode/               # VS Code build and launch configurations
│   ├── c_cpp_properties.json
│   ├── tasks.json
│   └── launch.json
├── main.c                 # ncurses terminal UI and menu loop logic
├── shapes.c               # Line/Circle drawing algorithms & shape manager
├── shapes.h               # Shape definitions and data structures
├── Makefile               # GNU Make build recipe
├── .gitignore             # Prevents binaries/objects from tracking
└── README.md              # This project documentation file
```
