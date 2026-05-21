# 🏥 Low-Level Hospital Emergency Room Triage Engine
### High-Performance Priority Simulation Framework in Pure C++

[![Language](https://img.shields.io/badge/Language-C%2B%2B11%2F14%2F17-00599C.svg?style=flat-square&logo=c%2B%2B)](https://en.wikipedia.org/wiki/C%2B%2B)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%2F%20POSIX-0078D4.svg?style=flat-square&logo=windows)](https://microsoft.com)
[![Architecture](https://img.shields.io/badge/Architecture-Manual%20Memory%20%2F%20Zero--STL-FF6B6B.svg?style=flat-square)](#)

A deterministic, real-time emergency department triage simulator designed to bypass standard first-come, first-served array limitations. The engine calculates an analytical clinical urgency score by dynamically cross-referencing acute active complaints with historical chronic risk profiles, shifting high-risk medical exceptions to the root of a tracking register immediately.

Developed to demonstrate theoretical spatial optimization and low-level abstract pointer mechanics for **CSE123: Data Structures and Algorithms** at the Egypt University of Informatics (EUI).

---

### 1. The Priority Sorting Engine (`EmergencyQueue`)
* **Data Structure:** Pointer-Array Backed Complete Binary Min-Heap.
* **Algorithmic Complexity:** $O(1)$ Maximum Urgency Retrieval, $O(\log n)$ Sink/Swim Heapification.
* **Implementation Details:** Operates by continually balancing the heap layout through recursive mathematical indexing shifts. The absolute highest-risk configuration is bound to array zero, allowing medical teams to retrieve the next target assignment instantly without exhaustive index sweeps.

### 2. Chronological Medical History Logs (`MedicalHistoryList`)
* **Data Structure:** Singly-Linked List Utilizing Explicit `HistoryNode` Structs.
* **Algorithmic Complexity:** $O(1)$ Front Insertion, Linear Space Mapping.
* **Implementation Details:** Captures continuous medical trends per patient file over multi-session lifetimes. Appending fresh diagnostic flags is done via explicit reference re-pointing to completely avoid memory reallocations or vector-shifting cascades.

### 3. Current Live Session Audit Trails (`TreatmentLog`)
* **Data Structure:** Fixed-Size Linear FIFO Queue.
* **Algorithmic Complexity:** $O(1)$ Enqueue/Dequeue.
* **Implementation Details:** Isolates processed records during active terminal lifecycles, establishing a localized transactional trail prior to updating disk database arrays.

---

## ⚡ Key Engineering Challenges Overcome

### 🔁 Elimination of Circular Compilation Dependencies
Because the `Patient` class holds reference logic containing the master `PatientDatabase` and vice-versa, standard header inclusion structures result in circular compilation faults. 
* **The Solution:** The engine splits declaration and operational execution through **Forward Declarations** and **Delayed Implementation Layouts**. Class architectures are safely introduced to the compiler ahead of instantiation loops, keeping code execution tight and error-free:
  1. Forward declare `PatientDatabase`.
  2. Declare `Patient` structure prototype variables.
  3. Declare complete `PatientDatabase` structural scopes.
  4. Implement downstream constructor functions only after all interfaces are registered with the compiler.

### 🖥️ Native Frameworkless Win32 Desktop GUI Engine (`main_gui.cpp`)
The GUI version completely avoids using bulky UI engines (such as Electron or Qt) by communicating directly with the Windows kernel via native Win32 system calling wrappers (`windows.h` and `commctrl.h`).
* **Graphical Heap Tree Visualizer:** Includes a built-in custom drawing system. By wrapping analytical coordinate formulas (`sin`, `cos`, and tree-depth multipliers), the interface renders the memory state of the binary min-heap array directly into a GDI graphics view layout on screen. Developers can watch heap nodes shift position in real time as patients are processed.

---

## 📂 Repository Blueprint

```text
├── src/
│   ├── main.cpp            # Interactive Terminal CLI Environment
│   └── main_gui.cpp        # Native Win32 Graphic Interface + GDI Heap Tree Visualizer
├── data/
│   ├── patients.csv        # Semicolon-delimited historical patient logs
│   └── symptoms_ref.csv    # Central clinical severity classification mapping
└── docs/
    ├── CSE PROJECT REPORT.pdf
    └── Hospital emergency system-Data structures project.pptx
