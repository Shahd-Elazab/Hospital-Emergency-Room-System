#include <iostream>
#include <fstream> // Required for CSV file handling
#include <sstream> // Required for parsing CSV rows
#include <string>
#include <vector>

using namespace std;


class PatientDatabase; 


// A single node in the MedicalHistoryList.
struct HistoryNode
{
    string symptom;       // Name of the past illness.
    int severityScore;    // Score of this illness.
    HistoryNode* next;    // Pointer to the next node in the sequence.
};

// This class is responsible for storing and managing the medical records of a single patient in memory using pointer manipulation.
class MedicalHistoryList
{
private:
    HistoryNode* head;    // The starting point of the linked list.
    HistoryNode* tail;

public:
    // Constructor: Sets head to NULL to initialize an empty list.
    MedicalHistoryList()
    {
        head=NULL;
        tail=NULL;
    }

    int isEmpty()
    {
        if(head==NULL)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    // Adds a new history record to the list by creating a new node from tail.
    void addRecord(string desc, int impact)
    {
        HistoryNode* newNode = new HistoryNode;

        newNode->symptom = desc;
        newNode->severityScore = impact;
        newNode->next = NULL;

        if(isEmpty())
        {
            head = newNode;
            tail = newNode;
        }
        else
        {
            tail->next = newNode;
            tail = newNode;
        }
    }

    // Iterates through the entire list to sum up all riskImpact values.
    // Returns: Total integer risk score used for priority boosting.
    int calculateTotalRisk()
    {
        int result= 0;
        if (isEmpty())
        {
            return result;
        }
        else
        {
            HistoryNode* tmp= NULL;
            for(tmp = head; tmp != NULL; tmp = tmp->next)
            {
                result += (tmp->severityScore);
            }
            return result;
        }
    }

    // Loops through the list to print all records for the UI/User manual.
    void printHistory()
    {
        if(isEmpty())
        {
            cout<<"NO MEDICAL HISTORY"<<endl;
        }
         else
        {
            HistoryNode* tmp;
            cout<<"MEDICAL HISTORY: ";
            for(tmp = head; tmp != NULL; tmp = tmp->next)
            {
                cout<<tmp->symptom<<", ";
            }
        }
    }

    // Getter for the head pointer to allow the database class to read the list.
    HistoryNode* getHead()
    {
        return head;
    }

    // Destructor
    ~MedicalHistoryList()
    {
        HistoryNode* current = head;

        while(current != NULL)
        {
            HistoryNode* temp = current;
            current = current->next;
            delete temp;
        }
    }
};



// This class acts as the "bridge" that holds patient details and connects their history to the priority system.
class Patient 
{
private:
    int id;                      // Unique identifier used for CSV file searching.
    static int count;            // To automatically generate unique IDs.
    string name;                 // Patient's full name.
    string currentSymptom;       // The illness they are currently complaining about.
    int basePriority;            // Initial urgency based only on the current symptom (1-10).
    int calculatedPriority;      // The final urgency score after history is applied.

public:
    MedicalHistoryList history;  // The patient's personal history stored as a Linked List.
    bool isExisting; // if true, it is an old patient, if false, it is a new patient.
    
    // We will define it after the definition of the PatientDatabase class.
    Patient(string n, string symp, int existingID = -1);

    // Logic: Calls history.calculateTotalRisk() and subtracts it from basePriority.
    // Formula: Final Priority = Base - History Risk.
    // Note: Since it's a Min-Heap, lower numbers move to the front of the line.
    void finalizePriority()
    {
        // Get total risk from medical history.
        int totalRisk = history.calculateTotalRisk();

        // Apply formula.
        calculatedPriority = basePriority + totalRisk;
    }

    // We'll define it later with the pateint constructor.
    void archiveCurrentSymptom();

    // We'll define it later with the pateint constructor.
    // This helper method set the score of the current symptom of the old patients.
    void setNewSymptom(string symptom);

    // Getters
    int getId()
    {
        return id;
    }

    string getName()
    {
        return name;
    }

    string getSymptom()
    {
        return currentSymptom;
    }

    int getBasePriority()
    {
        return basePriority; 
    }

    int getCalculatedPriority()
    {
        return calculatedPriority; 
    }
};

// Manages long-term data storage. It allows the system to recognize "Old" patients by searching a CSV file by ID.
class PatientDatabase
{
private:
    // The name of the csv file.
    string filename;
    
    // Vector to store all patient pointers loaded from the csv file.
    // We used pointers Because patients are dynamically allocated using: new, So the vector should store addresses.
    vector<Patient*> allPatients;     

public:
    // Constructor: Defines which file to open for reading/writing.
    // This constructor ensures that if the file is empty at first, we write at the begining of the file.
    PatientDatabase(string file = "patients.csv") // Default argument, so this constructor can work as default and parameterized.
    {
        filename = file;

        // Try to open the file for reading to see if it exists.
        ifstream inFile(filename);

        // If it doesn't exist (or is totally empty).
        if (!inFile || inFile.peek() == EOF)
        {
            inFile.close(); // Close the read attempt.

            // Create it and write the header immediately.
            ofstream outFile(filename);

            outFile << "Name,Symptom,History" << endl;
            outFile.close();
        }
        else
        {
            inFile.close();
        }
    }

    // This function returns the maximum ID in the file.
    int getMaxID(string filename)
    {
        ifstream file(filename);
        string line;

        int maxID = 0;

        if (!file.is_open()) 
            return 0; // File doesn't exist yet.

        getline(file, line); // Skip CSV header.

        while (getline(file, line)) 
        {
            stringstream ss(line);
            string idStr;
            getline(ss, idStr, ','); // Read the ID column.

            if (!idStr.empty()) 
            {

                int currentID = stoi(idStr);

                if (currentID > maxID) 
                {

                    maxID = currentID; // Update maxID.
                }
            }
        }
        file.close();
        return maxID;
    }

    // This function searches the symptoms_reference file for a priority score.
    int getSeverityScore(string symptomName)
    {
        ifstream file("symptoms_ref.csv");
        string line, name;

        int score;

        if (!file.is_open()) 
        {
            cout << "Error: Could not open reference file!" << endl;
            return 5; // Default medium priority if file fails.
        }

        // Skip the header line.
        getline(file, line);

        while (getline(file, line)) 
        {
            stringstream ss(line);
            getline(ss, name, ','); // Read symptom name.
            ss >> score;            // Read priority score.

            if (name == symptomName) 
            {
                file.close();
                return score; // Return the found severity.
            }
        }

        file.close();
        return 10; // Default to lowest priority if symptom not found.
    }


    // Function: loadAllPatientsFromFile().
    // What it does: Reads the entire CSV and populates the 'allPatients' vector.
    // Returns: void.
    void loadAllPatientsFromFile()
    {
        ifstream file(filename);
        if (!file.is_open()) 
        {
            cout << "Error: Could not open database file " << filename << endl;
            return;
        }

        string line;
        if (!getline(file, line)) return;
        for (Patient* p : allPatients) delete p;
        allPatients.clear();

        while (getline(file, line)) 
        {
            if (line.empty()) continue;

            stringstream ss(line);
            string idStr, name, historyStr;

            getline(ss, idStr, ',');
            getline(ss, name, ',');
            getline(ss, historyStr);

            if (idStr.empty()) continue;

            int id = stoi(idStr);
            Patient* p = new Patient(name, "", id);

            stringstream hss(historyStr);
            string symptom;
            while (getline(hss, symptom, ';')) 
            {
                if (!symptom.empty()) 
                {
                    p -> history.addRecord(symptom,getSeverityScore(symptom));
                }
            }
            allPatients.push_back(p);
        }
        file.close();
    }

    // Function: findById(int id).
    // What it does: Loops through the 'allPatients' vector to find a match.
    // Returns: Patient pointer if found, nullptr if it's a new patient.
    Patient* findById(int id)
    {
    for (Patient* p : allPatients)
    {
        if (p->getId() == id)
        {
            return p;
        }
    }

    return nullptr;
    }

    // Function: saveAllToFile().
    // What it does: Overwrites the CSV with the current state of the vector.
    void saveAllToFile()
    {
        ofstream File(filename);
        if (!File.is_open()) return;
        File << "ID,Name,History" << endl;

        for (Patient* p : allPatients)
        {
            File << p->getId() << "," << p->getName() << ",";

            HistoryNode* current = p -> history.getHead();
            while (current != NULL) {
                File << current->symptom;
                if (current->next != NULL) File << ";";
                current = current->next;
            }
            File << endl;
        }
        File.close();
    }

    void addPatientToVector(Patient* p)
    {
        if (p != nullptr)
        {
            allPatients.push_back(p);
        }
    }
    
    // This function loops on the vector and displays the patient.
    void displayPatients()
    {
        if (allPatients.empty())
        {
            cout << "No patients found in database." << endl;
            return;
        }

        cout << "\n--- Patients Database ---" << endl;

        for (Patient* p : allPatients)
        {
            cout << "ID: " << p->getId()
                << " | Name: " << p->getName()
                << " | History: ";

            HistoryNode* current = p->history.getHead();

            if (current == NULL)
            {
                cout << "No history";
            }
            else
            {
                while (current != NULL)
                {
                    cout << current->symptom;

                    if (current->next != NULL)
                    {
                        cout << ", ";
                    }

                    current = current->next;
                }
            }

            cout << endl;
        }
    }

    ~PatientDatabase() 
    {
    // Loop through the entire vector cache and delete the actual patient objects
    for (Patient* p : allPatients) 
    {
        delete p; 
    }
    allPatients.clear();
}
    
}; 

PatientDatabase* db = nullptr; 
int Patient::count = db->getMaxID("patients.csv");;

// Definition of the Patient class constructor.
// Constructor: Initializes the patient with their ID, name, symptom, and base priority and the calculated priority.
Patient::Patient(string n, string symp, int existingID) 
{
    name = n;
    currentSymptom = symp;
    isExisting = (existingID != -1);   // If -1, then it is a new patient.
    
    // Using the global pointer 'db'.
    basePriority = db->getSeverityScore(currentSymptom); 

    // Initially putting calculatedPriority = basePriority
    // But it will be calculated later in void finalizePriority().
    calculatedPriority = basePriority;

    if (!isExisting) // If new patient
    {
        count++;
        id = count;
    } 
    else // If old patient
    {
        id = existingID;
    }
}

// Definition of setNewSymptom(string symptom) method in Patient class.
// This function is used when it is an old patient, and we want to set his current symptom.
// As the time when we get the old patient info from the file, we intialize currentSymptom with "".
// So we use this function to redefine currentSymptom after we get it from the user
void Patient::setNewSymptom(string symptom)
{
    currentSymptom = symptom;

    // Recalculate base priority for the new issue
    basePriority = db->getSeverityScore(currentSymptom);

    // Initially putting calculatedPriority = basePriority
    // But it will be calculated later in void finalizePriority().
    calculatedPriority = basePriority;
}

// Definition of archiveCurrentSymptom() method in Patient class.
// This function adds a new record(node) in the MedicalHistory list of the pateint with the current symptom.
// To enable us later to update the vector and the file.
void Patient::archiveCurrentSymptom() 
    {
        if (!currentSymptom.empty() && currentSymptom != "None") 
        {
            int severity = db->getSeverityScore(currentSymptom);
            history.addRecord(currentSymptom, severity);
            
            currentSymptom = ""; // Clear it out so it can't be archived twice!
        }
    }

// The main "Waiting Room" structure that automatically sorts patients by urgency.
class EmergencyQueue 
{
private:
    // Maximum capacity of the waiting room.
    int capacity;            
    // The array here is a pointer of Patient pointers representing the min heap(complete binary tree)(priority queue).
    // We've made the array it self is a pointer to allow dynamic allocation of the size.
    Patient** arr;  
    // Number of patients currently waiting.         
    int currentSize;        


    // Compares a patient with its parent and moves it UP if more urgent.
    // After insertion at the index of the last leaf node.
    void heapifyUp(int index)
    {
        // Base case: If we are at the root, we can't go up anymore.
        if (index == 0) 
            return;

        int parent_index = (index - 1) / 2;
        if((arr[index] -> getCalculatedPriority()) < (arr[parent_index] -> getCalculatedPriority()) )
        {
            swap (arr[index], arr[parent_index]); // Built in function to swap elements in the array.
            heapifyUp(parent_index);
        }
    
    }

    // Moves a patient DOWN the tree to maintain heap order.
    // After the removal of root and replacing it with the last leaf node.
    void heapifyDown(int index)
    {
        int parent_index = index;
        int left_index = 2*(index) + 1;
        int right_index = 2*(index) + 2;

        // Check if LEFT child exists AND is more urgent than current smallest.
        if (left_index < currentSize && (arr[left_index] -> getCalculatedPriority()) < (arr[parent_index] -> getCalculatedPriority()))
        {
            parent_index = left_index;
        }
        // Check if RIGHT child exists AND is more urgent than current smallest.
        if (right_index < currentSize && (arr[right_index] -> getCalculatedPriority()) < (arr[parent_index] -> getCalculatedPriority()))
        {
            parent_index = right_index;
        }

        if (parent_index != index) // As if parent_index still equal the index so the parent is less than its childs, so we don't need to swap.
        {
            swap(arr[index], arr[parent_index]);
            heapifyDown(parent_index);
        }

    }

public:
    // Constructor: This is where the dynamic allocation happens.
    EmergencyQueue(int cap) 
    {
        capacity = cap;
        currentSize = 0; // Starts at zero, will be updated during enqueue and dequeue.
        
        // Dynamic Allocation: This creates the "cabinet" of pointers.
        arr = new Patient*[capacity]; 
        
        // Initialize all slots to nullptr to avoid "garbage" memory.
        for (int i = 0; i < capacity; i++) 
        {
            arr[i] = nullptr;
        }
    }


    // Inserts a patient in the heap and calls heapifyUp to sort them.
    void enqueue(Patient* p)
    {
        if (currentSize == capacity) 
        {
            cout << "The capacity of our emergency room is MAX. Sorry we can't take you." << endl;
            return;
        }
        // Note: we should insert at the last leaf node at index cuurentSize.
        // Ex: if currentSize = 0, we should insert at index 0.
        arr[currentSize] = p;
        heapifyUp(currentSize);

        currentSize ++;
    }

    // Removes and returns the root (the most critical patient) for treatment.
    Patient* dequeue()
    {
        if (currentSize == 0) 
        {
            cout << "The waiting emergency room is empty. There is no patient to treat." << endl;
            return nullptr;
        }
        Patient* root = arr[0]; // Stores the root (the most critical patient) to return it later.

        arr[0] = arr[currentSize - 1]; // Replace the root with the last leaf node.

        arr[currentSize - 1] = nullptr;
        currentSize --;

        heapifyDown(0);

         return root;
    }

    // Prints the list of all waiting patients for the video demonstration
    // A Min-Heap only guarantees that the root has the largest priority. 
    // It maintains a semi-sorted structure to keep insertion and removal at O(\log n) complexity.
    // To see them perfectly sorted, we would see that at TreatmentLog class.
    void displayPriorityQueue() // Displayed as binary tree
    {
        if (currentSize == 0) 
        {
            cout << "Queue is empty." << endl;
            return;
        }

        cout << "\n========================================================" << endl;
        cout << "          VISUAL EMERGENCY QUEUE TREE STRUCTURE          " << endl;
        cout << "========================================================" << endl;

        // Find total levels needed to display currentSize nodes
        int totalLevels = 0;
        while ((1 << totalLevels) - 1 < currentSize) 
        {
            totalLevels++;
        }

        int currentIndex = 0;

        // Iterate through each level of the tree
        for (int level = 0; level < totalLevels; level++) 
        {
            // Calculate nodes on this specific level (2^level)
            int nodesInLevel = 1 << level;

            // Dynamic formatting spaces based on tree depth
            int initialSpaces = (1 << (totalLevels - level)) * 3;
            int betweenSpaces = (1 << (totalLevels - level + 1)) * 3 - 6;

            // Print leading indentation spacing for the current level
            for (int s = 0; s < initialSpaces; s++) cout << " ";

            // Print all nodes residing on this specific level line
            for (int i = 0; i < nodesInLevel; i++) 
            {
                if (currentIndex < currentSize) 
                {
                    // Display the Patient Name along with their calculated priority
                    cout << "[" << arr[currentIndex]->getName() 
                        << " | P:" << arr[currentIndex]->getCalculatedPriority() << "]";
                } 
                else 
                {
                    // Structural placeholder for empty leaf positions
                    cout << "[---]";
                }

                // Print intermediate spacing between nodes on the same level line
                for (int s = 0; s < betweenSpaces; s++) cout << " ";
                
                currentIndex++;
            }
            cout << "\n\n"; // Drop down to prepare the next tree level row
        }
        cout << "========================================================" << endl;
    }

    // Destructor.
    ~EmergencyQueue() 
    {
        // To avoid memory leak.
        delete[] arr; 
    }
};


// Tracks the chronological order of treatment for auditing and performance analysis.
// Tracks the chronological order of treatment for auditing and performance analysis.
class TreatmentLog // Queue (FIFO)
{
private:
    Patient** log;          // Array of Patient pointers.
    int size;               // Number of patients treated so far.
    int capacity;

public:
    TreatmentLog(int cap)
    {
        capacity = cap;
        size = 0;
        log = new Patient*[capacity](); // Initializes all to nullptr automatically
    }

    // Adds a patient to the log immediately after they are dequeued from the heap.
    void addToLog(Patient* p) 
    {
        if (p == nullptr) return;
        if (size < capacity)
        {
            log[size] = p;
            size++;
        }
        else
        {
            cout << "\n[!] System Alert: Treatment Log history buffer is full!" << endl;
        }
    }

    // Displays the final treatment sequence in exact chronological order.
    void displayLog()
    {
        if (size == 0)
        {
            cout << "\n=============================================" << endl;
            cout << "       TREATMENT LOG IS CURRENTLY EMPTY      " << endl;
            cout << "=============================================" << endl;
            return;
        }

        cout << "\n=============================================" << endl;
        cout << "       CHRONOLOGICAL TREATMENT LOG (FIFO)    " << endl;
        cout << "=============================================" << endl;
        for (int i = 0; i < size; i++)
        {
            cout << i + 1 << ". [ID: " << log[i]->getId()
                 << "] - Name: " << log[i]->getName()
                 << " | Priority: " << log[i]->getCalculatedPriority()
                 << " | Treated For: " << log[i]->getSymptom() << endl;
        }
        cout << "=============================================" << endl;
    }

    ~TreatmentLog()
    {
        delete[] log;
    }
};


int main()
{
    db = new PatientDatabase("patients.csv");
    db->loadAllPatientsFromFile(); // Fill our database vector cache from disk storage

    // Allocate managing queue boundaries (Max capacity of 100 entries)
    EmergencyQueue ER(100);
    TreatmentLog historyLog(100);

    cout << "==========================================================" << endl;
    cout << "   WELCOME TO THE HOSPITAL EMERGENCY ROOM & OPERATIONS  " << endl;
    cout << "==========================================================" << endl;

    int mainChoice = 0;
    while (mainChoice != 6) // Updated to match our extended menu exit boundary
    {
        cout << "\nMAIN OPERATIONS PANEL:" << endl;
        cout << "1 - Register a Patient" << endl;
        cout << "2 - Treat Next Patient (Moves Heap Root -> Treatment Log)" << endl;
        cout << "3 - Display Waiting Priority Heap " << endl;
        cout << "4 - Display Chronological Treatment Log" << endl;
        cout << "5 - Display Long-Term Patient Database Records" << endl;
        cout << "6 - Exit System" << endl;
        cout << "Select Operation: ";
        
        if (!(cin >> mainChoice)) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "[!] Invalid option entry. Please type a valid option number." << endl;
            continue;
        }
        cin.ignore(); // Clean leftover newline from buffer

        switch (mainChoice)
        {
            case 1:
            {
                char keepAdding = 'y';
                while (keepAdding == 'y' || keepAdding == 'Y')
                {
                    cout << "\nIs this an existing (old) patient? (y/n): ";
                    char status;
                    cin >> status;
                    cin.ignore();

                    if (status == 'y' || status == 'Y')
                    {
                        cout << "Enter Patient ID: ";
                        int lookupId;
                        cin >> lookupId;
                        cin.ignore();

                        Patient* oldProfile = db->findById(lookupId);

                        if (oldProfile != nullptr)
                        {
                            cout << "[Found] Welcome back, " << oldProfile->getName() << "!" << endl;
                            
                            string currentSymptom;
                            cout << "Enter current complaining symptom: ";
                            getline(cin, currentSymptom);

                            // Update the original object directly in the vector cache!
                            oldProfile->setNewSymptom(currentSymptom);
                            oldProfile->finalizePriority(); 
                            
                            // Push the same pointer into the active queue
                            ER.enqueue(oldProfile);

                            cout << "--> Patient sorted into the waiting ER. Updated Priority Tier: " 
                                << oldProfile->getCalculatedPriority() << endl;
                        }
                        else
                        {
                            cout << "[!] Error: No record associated with ID (" << lookupId << ") found." << endl;
                        }
                    }
                    else if (status == 'n' || status == 'N')
                    {
                        string name, symptom;
                        cout << "Enter Patient's Full Name: ";
                        getline(cin, name);
                        cout << "Enter Current Complaining Symptom: ";
                        getline(cin, symptom);

                        Patient* newPatient = new Patient(name, symptom);
                        newPatient->finalizePriority();
                        
                        // CRITICAL FIX: Add them to long-term registry cache immediately
                        db->addPatientToVector(newPatient);
                        
                        ER.enqueue(newPatient);

                        cout << "--> New Account Issued! Generated ID: " << newPatient->getId() << endl;
                        cout << "--> Patient sorted into the waiting ER. Initial Priority Tier: " 
                            << newPatient->getCalculatedPriority() << endl;
                    }
                    else
                    {
                        cout << "[!] Invalid choice context selection." << endl;
                    }

                    cout << "\nWould you like to register another patient in the waiting ER? (y/n): ";
                    cin >> keepAdding;
                    cin.ignore();
                }
                break;
            }

            case 2:
            {
                Patient* targetPatient = ER.dequeue();
                if (targetPatient != nullptr)
                {
                    cout << "\n>>> [CALLING] Doctor is now treating: " << targetPatient->getName() 
                        << " (ID: " << targetPatient->getId() << ") for " << targetPatient->getSymptom() << " <<<" << endl;
                    
                    // CRITICAL FIX: Archive the symptom into their historical timeline list
                    targetPatient->archiveCurrentSymptom();
                    
                    // Now transfer tracking control into the chronological FIFO track
                    historyLog.addToLog(targetPatient);
                }
                break;
            }

            case 3:
                ER.displayPriorityQueue();
                break;

            case 4:
                historyLog.displayLog();
                break;

            case 5:
                // Loops through the file entries to print the saved long term records
                cout << "\n--- Long-Term Patient Profile Cache Registry ---" << endl;
                db->displayPatients(); 
                break;

            case 6:
                cout << "\nSaving session records and terminating software state... Goodbye!" << endl;
                // Safely save current tracking logs down to storage medium persistence file
                db->saveAllToFile(); 
                break;

            default:
                cout << "[!] Out of range input. Enter a value between 1 and 6." << endl;
                break;
        }
    }
    delete db; // Clean static memory
    return 0;
}