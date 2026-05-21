# 🏥 Hospital Emergency System

A C++-based **Emergency Room Patient Management System** that simulates real-world hospital triage operations using custom-built data structures and algorithms.

---

# 📌 Project Overview

The **Hospital Emergency System** is designed to manage patients in an emergency room by prioritizing critical cases based on symptom severity and medical history.

The system applies core **Data Structures & Algorithms** concepts by implementing all major structures manually without using built-in STL structures such as `<queue>` or `<list>`.

This project demonstrates:
- Custom linked lists
- Binary Min-Heap implementation
- FIFO queue processing
- File handling with CSV
- Dynamic memory management
- Object-Oriented Programming in C++

---

# 🚀 Features

✅ Register new patients  
✅ Handle returning patients using saved records  
✅ Automatically calculate patient urgency levels  
✅ Prioritize emergency cases dynamically  
✅ Maintain complete medical history records  
✅ Save and load patient data using CSV files  
✅ Log treated patients chronologically  
✅ Display the emergency queue as a heap/tree structure  

---

# 🧠 Data Structures Used

| Data Structure | Purpose |
|---|---|
| Singly Linked List | Stores patient medical history |
| Binary Min-Heap | Emergency priority queue |
| FIFO Queue | Treatment history log |
| Vector Cache | In-memory patient database |

---

# ⚙️ System Workflow

1. Patient enters the ER
2. System checks whether the patient already exists
3. Current symptom severity is retrieved
4. Medical history risk is calculated
5. Final priority score is generated
6. Patient is inserted into the Min-Heap queue
7. Most urgent patient is treated first
8. Treatment is archived into the history log
9. Data is saved permanently into CSV files

---

# 🏗️ Main Modules

## 1. Patient
Stores:
- Patient ID
- Name
- Current symptom
- Priority score
- Medical history

### Key Functions
- `finalizePriority()`
- `setNewSymptom()`
- `archiveCurrentSymptom()`

---

## 2. MedicalHistoryList
Custom singly linked list used to store:
- Previous symptoms
- Risk severity scores

### Key Functions
- `addRecord()`
- `calculateTotalRisk()`
- `printHistory()`

---

## 3. EmergencyQueue
Custom Binary Min-Heap implementation.

Responsible for:
- Sorting patients by urgency
- Ensuring critical patients are treated first

### Key Functions
- `enqueue()`
- `dequeue()`
- `heapifyUp()`
- `heapifyDown()`

---

## 4. PatientDatabase
Handles:
- File reading/writing
- Loading previous patients
- Persistent storage

### Files Used
- `patients.csv`
- `symptoms_ref.csv`

---

## 5. TreatmentLog
FIFO queue that stores treated patients chronologically.

### Key Functions
- `addToLog()`
- `displayLog()`

---

# 📊 Complexity Analysis

| Operation | Time Complexity |
|---|---|
| Linked List Insert | O(1) |
| History Risk Calculation | O(n) |
| Heap Insert | O(log n) |
| Heap Remove | O(log n) |
| Patient Search | O(v) |
| Save to File | O(v × n) |

Where:
- `n` = number of medical history records
- `v` = number of patients

---

# 🛠️ Technologies Used

- C++
- OOP Concepts
- Dynamic Memory Allocation
- CSV File Handling
- Custom Data Structures
- Heap Algorithms

---

# 📖 User Manual

## 🖥️ Main Menu

When the program starts, the following menu appears:

```text
1. Register Patient
2. Treat Next Patient
3. Display Emergency Queue
4. Display Treatment Log
5. Display Database Records
6. Exit
```

---

## 1️⃣ Register Patient

Allows the user to:
- Register a new patient
- Process a returning patient

## New Patient
The system asks for:
- Patient name
- Current symptom

The system then:
- Generates a unique ID
- Calculates priority
- Inserts patient into the emergency queue

## Returning Patient
The system asks for:
- Existing patient ID
- New symptom

The system:
- Loads previous history
- Calculates updated risk
- Reprioritizes the patient

---

## 2️⃣ Treat Next Patient

The system:
- Removes the highest-priority patient from the queue
- Archives the symptom
- Stores the patient in the treatment log

Patients with lower priority values are treated first.

---

## 3️⃣ Display Emergency Queue

Displays:
- All waiting patients
- Their priority values
- Heap/tree structure representation

This helps visualize how the Min-Heap works.

---

## 4️⃣ Display Treatment Log

Displays all treated patients in chronological order.

Information shown:
- Patient ID
- Name
- Symptom
- Priority

---

## 5️⃣ Display Database Records

Shows all stored patient records loaded from `patients.csv`.

Includes:
- IDs
- Names
- Medical history

---

## 6️⃣ Exit System

Before shutting down, the system:
- Saves all patient records
- Updates CSV files
- Frees dynamically allocated memory

---

# ✅ Testing & Validation

The system was tested for:
- Correct queue prioritization
- Proper heap balancing
- Accurate treatment logging
- File loading/saving
- Invalid input handling
- Memory cleanup

---

# 📚 Educational Concepts Demonstrated

- Linked Lists
- Binary Heaps
- Queues
- Dynamic Memory
- File Persistence
- Object-Oriented Programming
- Complexity Analysis
- Pointer Manipulation

---

# 📄 License

This project was developed for educational purposes as part of:

**CSE123 – Data Structures and Algorithms**  
Spring 2026

---


# 🖥️ Native Frameworkless Win32 Desktop GUI Engine (`main_gui.cpp`)
The GUI version completely avoids using bulky UI engines (such as Electron or Qt) by communicating directly with the Windows kernel via native Win32 system calling wrappers (`windows.h` and `commctrl.h`).
* **Graphical Heap Tree Visualizer:** Includes a built-in custom drawing system. By wrapping analytical coordinate formulas (`sin`, `cos`, and tree-depth multipliers), the interface renders the memory state of the binary min-heap array directly into a GDI graphics view layout on screen. Developers can watch heap nodes shift position in real time as patients are processed.

---

# 📂 Repository Blueprint

```text
├── src/
│   ├── main.cpp            # Interactive Terminal CLI Environment
│   └── main_gui.cpp        # Native Win32 Graphic Interface + GDI Heap Tree Visualizer
├── data/
│   ├── patients.csv        # Semicolon-delimited historical patient logs
│   └── symptoms_ref.csv    # Central clinical severity classification mapping
└── docs/
    └── README.md
    
