#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <cmath>
using namespace std;

// ============================================================
//  RESOURCE IDs
// ============================================================
#define IDC_TAB_MAIN        100
#define IDC_BTN_REGISTER    101
#define IDC_BTN_TREAT       102
#define IDC_BTN_SAVE        103
#define IDC_EDIT_NAME       104
#define IDC_EDIT_SYMPTOM    105
#define IDC_EDIT_ID         106
#define IDC_RADIO_NEW       107
#define IDC_RADIO_EXISTING  108
#define IDC_LIST_QUEUE      109
#define IDC_LIST_LOG        110
#define IDC_LIST_DB         111
#define IDC_STATUS          112
#define IDC_LBL_NAME        113
#define IDC_LBL_SYMPTOM     114
#define IDC_LBL_ID          115
#define IDC_GRP_REGISTER    116
#define IDC_GRP_TREAT       117
#define IDC_LBL_NEXT        118
#define IDC_BTN_HEAP        119   // NEW: Show Heap Visualizer
#define IDC_BTN_HEAP_CLOSE  120
#define IDC_TIMER_PULSE     200

// ============================================================
//  COLOR PALETTE  (Clean White + Vivid Accents)
// ============================================================
// Main window — crisp white clinical look
#define CLR_BG           RGB(245, 247, 252)   // soft white-blue
#define CLR_CARD         RGB(255, 255, 255)   // pure white card
#define CLR_PANEL        RGB(235, 239, 248)   // very light blue panel
#define CLR_BORDER       RGB(200, 210, 230)   // soft grey-blue border
#define CLR_SIDEBAR      RGB(25,  35,  65)    // deep navy sidebar

// Vivid accents (pop against white)
#define CLR_CYAN         RGB(0,   160, 220)   // vivid sky-blue
#define CLR_MAGENTA      RGB(220, 30,  120)   // deep rose/magenta
#define CLR_GREEN        RGB(20,  180, 90)    // vivid green
#define CLR_ORANGE       RGB(255, 140, 0)     // amber orange
#define CLR_YELLOW       RGB(255, 200, 0)     // gold yellow

// Text (dark on white)
#define CLR_TEXT         RGB(25,  35,  60)    // near-black navy
#define CLR_SUBTEXT      RGB(100, 115, 145)   // medium grey-blue
#define CLR_TEXT_DARK    RGB(255, 255, 255)   // white text on coloured buttons

// Heap node colours (vivid, on white bg)
#define CLR_HEAP_ROOT    RGB(220, 30,  100)   // root = crimson-rose  (most urgent)
#define CLR_HEAP_L1      RGB(230, 100, 0)     // level 1 = burnt orange
#define CLR_HEAP_L2      RGB(180, 150, 0)     // level 2 = deep gold
#define CLR_HEAP_L3      RGB(20,  160, 80)    // level 3+ = forest green
#define CLR_HEAP_EDGE    RGB(180, 195, 220)   // soft connector line
#define CLR_HEAP_BG      RGB(250, 252, 255)   // near-white heap bg
#define CLR_HEAP_GRID    RGB(225, 232, 245)   // very faint grid lines
#define CLR_HEAP_GLOW    RGB(0,   140, 210)   // node shadow ring

// ============================================================
//  DATA STRUCTURES  (unchanged from original)
// ============================================================
class PatientDatabase;

struct HistoryNode {
    string symptom;
    int severityScore;
    HistoryNode* next;
};

class MedicalHistoryList {
private:
    HistoryNode* head;
    HistoryNode* tail;
public:
    MedicalHistoryList() : head(NULL), tail(NULL) {}
    int isEmpty() { return head == NULL ? 1 : 0; }

    void addRecord(string desc, int impact) {
        HistoryNode* n = new HistoryNode();
        n->symptom = desc; n->severityScore = impact; n->next = NULL;
        if (isEmpty()) { head = tail = n; }
        else { tail->next = n; tail = n; }
    }

    int calculateTotalRisk() {
        int r = 0;
        for (HistoryNode* t = head; t; t = t->next) r += t->severityScore;
        return r;
    }

    string historyString() {
        string s;
        for (HistoryNode* t = head; t; t = t->next) {
            if (!s.empty()) s += "; ";
            s += t->symptom;
        }
        return s.empty() ? "No history" : s;
    }

    HistoryNode* getHead() { return head; }

    ~MedicalHistoryList() {
        HistoryNode* cur = head;
        while (cur) { HistoryNode* tmp = cur; cur = cur->next; delete tmp; }
    }
};

class Patient {
private:
    int id;
    static int count;
    string name, currentSymptom;
    int basePriority, calculatedPriority;
public:
    MedicalHistoryList history;
    bool isExisting;

    Patient(string n, string symp, int existingID = -1);

    void finalizePriority() {
        calculatedPriority = basePriority - history.calculateTotalRisk();
    }
    void archiveCurrentSymptom();
    void setNewSymptom(string symptom);

    int    getId()                 { return id;                 }
    string getName()               { return name;               }
    string getSymptom()            { return currentSymptom;     }
    int    getBasePriority()       { return basePriority;       }
    int    getCalculatedPriority() { return calculatedPriority; }
    static void setCount(int c)    { count = c;                 }
};

class PatientDatabase {
private:
    string filename;
    vector<Patient*> allPatients;
public:
    PatientDatabase(string file = "patients.csv") : filename(file) {
        ifstream in(filename.c_str());
        if (!in || in.peek() == EOF) {
            in.close();
            ofstream out(filename.c_str());
            out << "ID,Name,History\n";
        } else { in.close(); }
    }

    int getMaxID(string fn) {
        ifstream file(fn.c_str());
        if (!file.is_open()) return 0;
        string line;
        getline(file, line);
        int maxID = 0;
        while (getline(file, line)) {
            stringstream ss(line);
            string idStr;
            getline(ss, idStr, ',');
            if (!idStr.empty()) {
                try { int cur = atoi(idStr.c_str()); if (cur > maxID) maxID = cur; }
                catch(...) {}
            }
        }
        return maxID;
    }

    int getSeverityScore(string symptomName) {
        ifstream file("symptoms_ref.csv");
        if (!file.is_open()) return 5;
        string line, name; int score = 5;
        getline(file, line);
        while (getline(file, line)) {
            stringstream ss(line);
            getline(ss, name, ',');
            string scoreStr;
            getline(ss, scoreStr);
            if (!scoreStr.empty()) score = atoi(scoreStr.c_str());
            if (name == symptomName) { file.close(); return score; }
        }
        file.close();
        return 10;
    }

    void loadAllPatientsFromFile() {
        ifstream file(filename.c_str());
        if (!file.is_open()) return;
        string line;
        if (!getline(file, line)) return;
        for (size_t i = 0; i < allPatients.size(); i++) delete allPatients[i];
        allPatients.clear();
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string idStr, name, historyStr;
            getline(ss, idStr, ',');
            getline(ss, name, ',');
            getline(ss, historyStr);
            if (idStr.empty()) continue;
            int id = atoi(idStr.c_str());
            Patient* p = new Patient(name, "", id);
            stringstream hss(historyStr);
            string sym;
            while (getline(hss, sym, ';'))
                if (!sym.empty()) p->history.addRecord(sym, getSeverityScore(sym));
            allPatients.push_back(p);
        }
        file.close();
    }

    Patient* findById(int id) {
        for (size_t i = 0; i < allPatients.size(); i++)
            if (allPatients[i]->getId() == id) return allPatients[i];
        return NULL;
    }

    void saveAllToFile() {
        ofstream File(filename.c_str());
        if (!File.is_open()) return;
        File << "ID,Name,History\n";
        for (size_t i = 0; i < allPatients.size(); i++) {
            Patient* p = allPatients[i];
            File << p->getId() << "," << p->getName() << ",";
            HistoryNode* cur = p->history.getHead();
            while (cur) {
                File << cur->symptom;
                if (cur->next) File << ";";
                cur = cur->next;
            }
            File << "\n";
        }
        File.close();
    }

    void addPatientToVector(Patient* p) { if (p) allPatients.push_back(p); }
    const vector<Patient*>& getAllPatients() const { return allPatients; }

    ~PatientDatabase() {
        for (size_t i = 0; i < allPatients.size(); i++) delete allPatients[i];
        allPatients.clear();
    }
};

class EmergencyQueue {
private:
    int capacity, currentSize;
    Patient** arr;
    void heapifyUp(int i) {
        if (i == 0) return;
        int p = (i-1)/2;
        if (arr[i]->getCalculatedPriority() < arr[p]->getCalculatedPriority()) {
            Patient* tmp = arr[i]; arr[i] = arr[p]; arr[p] = tmp;
            heapifyUp(p);
        }
    }
    void heapifyDown(int i) {
        int best=i, l=2*i+1, r=2*i+2;
        if (l<currentSize && arr[l]->getCalculatedPriority()<arr[best]->getCalculatedPriority()) best=l;
        if (r<currentSize && arr[r]->getCalculatedPriority()<arr[best]->getCalculatedPriority()) best=r;
        if (best!=i) {
            Patient* tmp = arr[i]; arr[i] = arr[best]; arr[best] = tmp;
            heapifyDown(best);
        }
    }
public:
    EmergencyQueue(int cap) : capacity(cap), currentSize(0) {
        arr = new Patient*[capacity];
        for (int i = 0; i < capacity; i++) arr[i] = NULL;
    }
    bool enqueue(Patient* p) {
        if (currentSize==capacity) return false;
        arr[currentSize]=p; heapifyUp(currentSize++); return true;
    }
    Patient* dequeue() {
        if (currentSize==0) return NULL;
        Patient* root=arr[0];
        arr[0]=arr[--currentSize]; arr[currentSize]=NULL;
        if (currentSize>0) heapifyDown(0);
        return root;
    }
    int size() const { return currentSize; }
    Patient* at(int i) const { return (i>=0&&i<currentSize)?arr[i]:NULL; }
    ~EmergencyQueue() { delete[] arr; }
};

class TreatmentLog {
private:
    Patient** log;
    int size_;
    int capacity;
public:
    TreatmentLog(int cap) : size_(0), capacity(cap) {
        log = new Patient*[capacity];
        for (int i = 0; i < capacity; i++) log[i] = NULL;
    }
    void addToLog(Patient* p) { if (p && size_<capacity) log[size_++]=p; }
    int size() const { return size_; }
    Patient* at(int i) const { return (i>=0&&i<size_)?log[i]:NULL; }
    ~TreatmentLog() { delete[] log; }
};

// ── Globals ──────────────────────────────────────────────────
PatientDatabase* db  = NULL;
int Patient::count   = 0;

Patient::Patient(string n, string symp, int existingID)
    : name(n), currentSymptom(symp), isExisting(existingID!=-1)
{
    basePriority = db ? db->getSeverityScore(currentSymptom) : 5;
    calculatedPriority = basePriority;
    if (!isExisting) { count++; id=count; } else { id=existingID; }
}
void Patient::setNewSymptom(string symptom) {
    currentSymptom = symptom;
    basePriority = db ? db->getSeverityScore(currentSymptom) : 5;
    calculatedPriority = basePriority;
}
void Patient::archiveCurrentSymptom() {
    if (!currentSymptom.empty() && currentSymptom != "None") {
        int sev = db ? db->getSeverityScore(currentSymptom) : 5;
        history.addRecord(currentSymptom, sev);
        currentSymptom = "";
    }
}

// ── Win32 globals ─────────────────────────────────────────────
HWND hWnd, hTab;
HWND hGrpReg, hRadioNew, hRadioExisting;
HWND hLblName,    hEditName;
HWND hLblID,      hEditID;
HWND hLblSymptom, hEditSymptom;
HWND hBtnRegister;
HWND hGrpTreat, hLblNext, hBtnTreat;
HWND hBtnSave, hBtnHeap;
HWND hListQueue, hListLog, hListDB;
HWND hStatus;
HWND hHeapWnd = NULL;   // Heap visualizer popup

EmergencyQueue* ER   = NULL;
TreatmentLog*   logQ = NULL;

HFONT  hFontMain, hFontBold, hFontMono, hFontTitle, hFontSmall;
HBRUSH hBrushBg, hBrushCard, hBrushPanel, hBrushCyan, hBrushMagenta, hBrushGreen;

int currentTab   = 0;
int pulseFrame   = 0;   // for animated glow on heap nodes

// ============================================================
//  HELPERS
// ============================================================
string intToStr(int n) {
    char buf[32]; sprintf(buf, "%d", n); return string(buf);
}

void SetStatus(const string& msg, bool isError = false) {
    SetWindowTextA(hStatus, msg.c_str());
    SetWindowLongPtr(hStatus, GWLP_USERDATA, isError ? 1 : 0);
    InvalidateRect(hStatus, NULL, TRUE);
}

// ── Draw a rounded rectangle helper ──────────────────────────
void DrawRoundedRect(HDC hdc, int x, int y, int w, int h, int r,
                     COLORREF fill, COLORREF stroke, int strokeW = 1) {
    HBRUSH br = CreateSolidBrush(fill);
    HPEN   pen = CreatePen(PS_SOLID, strokeW, stroke);
    HBRUSH oldBr  = (HBRUSH)SelectObject(hdc, br);
    HPEN   oldPen = (HPEN)SelectObject(hdc, pen);
    RoundRect(hdc, x, y, x+w, y+h, r, r);
    SelectObject(hdc, oldBr);
    SelectObject(hdc, oldPen);
    DeleteObject(br);
    DeleteObject(pen);
}

// ── Draw a diamond shape ─────────────────────────────────────
void DrawDiamond(HDC hdc, int cx, int cy, int half,
                 COLORREF fill, COLORREF stroke) {
    POINT pts[4] = {
        {cx,       cy-half},
        {cx+half,  cy     },
        {cx,       cy+half},
        {cx-half,  cy     }
    };
    HBRUSH br  = CreateSolidBrush(fill);
    HPEN   pen = CreatePen(PS_SOLID, 2, stroke);
    HBRUSH oldBr  = (HBRUSH)SelectObject(hdc, br);
    HPEN   oldPen = (HPEN)SelectObject(hdc, pen);
    Polygon(hdc, pts, 4);
    SelectObject(hdc, oldBr);
    SelectObject(hdc, oldPen);
    DeleteObject(br); DeleteObject(pen);
}

// ── Draw glowing circle (glow = larger dim ring behind) ───────
void DrawGlowCircle(HDC hdc, int cx, int cy, int r,
                    COLORREF fill, COLORREF glow, int glowW = 3) {
    // glow ring
    HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
    for (int g = glowW; g > 0; g--) {
        BYTE alpha = (BYTE)(60 + g*30);
        HPEN gpen = CreatePen(PS_SOLID, g*2,
            RGB( (GetRValue(glow)*alpha)/255,
                 (GetGValue(glow)*alpha)/255,
                 (GetBValue(glow)*alpha)/255 ));
        HPEN oldP = (HPEN)SelectObject(hdc, gpen);
        HBRUSH oldBr = (HBRUSH)SelectObject(hdc, nullBr);
        Ellipse(hdc, cx-r-g*2, cy-r-g*2, cx+r+g*2, cy+r+g*2);
        SelectObject(hdc, oldP);
        SelectObject(hdc, oldBr);
        DeleteObject(gpen);
    }
    // filled circle
    HBRUSH br  = CreateSolidBrush(fill);
    HPEN   pen = CreatePen(PS_SOLID, 2, glow);
    SelectObject(hdc, br);
    SelectObject(hdc, pen);
    Ellipse(hdc, cx-r, cy-r, cx+r, cy+r);
    SelectObject(hdc, (HBRUSH)GetStockObject(WHITE_BRUSH));
    SelectObject(hdc, (HPEN)GetStockObject(BLACK_PEN));
    DeleteObject(br); DeleteObject(pen);
}

// ── Determine heap node colour by depth ──────────────────────
COLORREF HeapNodeColor(int idx) {
    if (idx == 0) return CLR_HEAP_ROOT;
    if (idx <= 2) return CLR_HEAP_L1;
    if (idx <= 6) return CLR_HEAP_L2;
    return CLR_HEAP_L3;
}

// ── Compute proper subtree width (in leaf-slots) ─────────────
//    Every missing child still occupies its leaf slot so the
//    tree is symmetric and no branches cross.
int SubtreeLeaves(int idx, int n) {
    if (idx >= n) return 1;           // phantom leaf — still 1 slot
    int l = SubtreeLeaves(2*idx+1, n);
    int r = SubtreeLeaves(2*idx+2, n);
    return l + r;
}

// ── Recursive position assignment ────────────────────────────
struct NodePos { int x, y; };
void AssignPos(int idx, int n, int depth,
               int xLeft, int xRight, int startY, int levelH,
               vector<NodePos>& pos) {
    if (idx >= n) return;
    int lLeaves = SubtreeLeaves(2*idx+1, n);
    int rLeaves = SubtreeLeaves(2*idx+2, n);
    int total   = lLeaves + rLeaves;
    // centre of left subtree portion
    int xMid = xLeft + (xRight - xLeft) * lLeaves / total;
    pos[idx].x = xMid;
    pos[idx].y = startY + depth * levelH;
    AssignPos(2*idx+1, n, depth+1, xLeft, xMid,    startY, levelH, pos);
    AssignPos(2*idx+2, n, depth+1, xMid,  xRight,  startY, levelH, pos);
}

// ── Draw the min-heap tree ────────────────────────────────────
void DrawHeapTree(HDC hdc, int winW, int winH) {
    int n = ER->size();

    // ── Title bar (always drawn) ──────────────────────────────
    // Dark navy top band
    RECT titleBand = {0, 0, winW, 52};
    HBRUSH navyBr = CreateSolidBrush(RGB(25, 35, 65));
    FillRect(hdc, &titleBand, navyBr);
    DeleteObject(navyBr);

    // Coloured accent strip on left edge of title band
    HBRUSH acBr = CreateSolidBrush(CLR_HEAP_ROOT);
    RECT acRect = {0, 0, 6, 52};
    FillRect(hdc, &acRect, acBr);
    DeleteObject(acBr);

    SetBkMode(hdc, TRANSPARENT);
    HFONT oldFont = (HFONT)SelectObject(hdc, hFontBold);

    // Title text
    SetTextColor(hdc, RGB(255,255,255));
    char titleStr[80];
    sprintf(titleStr, "  Min-Heap Visualizer  —  %d node%s in ER queue",
            n, n==1?"":"s");
    RECT titleR = {10, 8, winW-10, 44};
    DrawTextA(hdc, titleStr, -1, &titleR, DT_LEFT|DT_VCENTER|DT_SINGLELINE);

    // Priority label (right side)
    SetTextColor(hdc, CLR_HEAP_ROOT);
    const char* priLabel = "Lower score = more urgent";
    RECT plR = {0, 8, winW-12, 44};
    DrawTextA(hdc, priLabel, -1, &plR, DT_RIGHT|DT_VCENTER|DT_SINGLELINE);

    if (n == 0) {
        SetTextColor(hdc, CLR_SUBTEXT);
        const char* msg = "Register patients to see the heap tree here.";
        RECT r = {0, winH/2-20, winW, winH/2+20};
        DrawTextA(hdc, msg, -1, &r, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        SelectObject(hdc, oldFont);
        return;
    }

    // ── Layout parameters ─────────────────────────────────────
    const int TITLE_H  = 58;   // top band height + breathing room
    const int LEGEND_H = 90;   // reserved at bottom for legend
    const int MARGIN_X = 50;   // left/right margin
    const int nodeR    = 32;   // circle radius

    // Count tree depth
    int maxDepth = 0;
    { int tmp = n-1; while(tmp>0){tmp=(tmp-1)/2; maxDepth++;} }

    // Vertical spacing — fill the available height evenly
    int availH = winH - TITLE_H - LEGEND_H;
    int levels  = maxDepth + 1;
    int levelH  = (levels > 1) ? (availH / levels) : availH;
    if (levelH < 80) levelH = 80;
    int startY  = TITLE_H + levelH / 2;   // centre of first level

    // Assign x,y to every node with the recursive balanced layout
    vector<NodePos> pos(n);
    AssignPos(0, n, 0,
              MARGIN_X, winW - MARGIN_X,
              startY, levelH, pos);

    // ── Draw subtle guide lines between levels ────────────────
    HPEN gridPen = CreatePen(PS_DOT, 1, CLR_HEAP_GRID);
    HPEN oldPen  = (HPEN)SelectObject(hdc, gridPen);
    for (int d = 1; d <= maxDepth; d++) {
        int yLine = startY + d * levelH - levelH/2;
        MoveToEx(hdc, MARGIN_X, yLine, NULL);
        LineTo(hdc, winW - MARGIN_X, yLine);
    }
    SelectObject(hdc, oldPen);
    DeleteObject(gridPen);

    // ── Draw edges ────────────────────────────────────────────
    for (int i = 0; i < n; i++) {
        int lc = 2*i+1, rc2 = 2*i+2;
        for (int ch : {lc, rc2}) {
            if (ch >= n) continue;
            // Gradient-coloured edge: colour matches child's level colour
            COLORREF ec = HeapNodeColor(ch);
            HPEN ep = CreatePen(PS_SOLID, 2,
                RGB(GetRValue(ec)*3/5, GetGValue(ec)*3/5, GetBValue(ec)*3/5));
            HPEN op = (HPEN)SelectObject(hdc, ep);
            MoveToEx(hdc, pos[i].x,  pos[i].y,  NULL);
            LineTo  (hdc, pos[ch].x, pos[ch].y);
            SelectObject(hdc, op);
            DeleteObject(ep);
        }
    }

    // ── Draw nodes ────────────────────────────────────────────
    SelectObject(hdc, hFontBold);
    for (int i = 0; i < n; i++) {
        Patient* p = ER->at(i);
        if (!p) continue;

        COLORREF col = HeapNodeColor(i);
        // Pale tinted fill derived from the accent colour
        COLORREF fill = RGB(
            255 - (255-GetRValue(col))/5,
            255 - (255-GetGValue(col))/5,
            255 - (255-GetBValue(col))/5);

        // Pulse root
        int rr = nodeR;
        if (i == 0) rr = nodeR + (pulseFrame % 8 < 4 ? 3 : 0);

        // Drop shadow
        HBRUSH shadowBr = CreateSolidBrush(RGB(180,190,210));
        HPEN   nullPen  = (HPEN)GetStockObject(NULL_BRUSH);
        SelectObject(hdc, shadowBr);
        SelectObject(hdc, nullPen);
        Ellipse(hdc, pos[i].x-rr+3, pos[i].y-rr+4,
                     pos[i].x+rr+3, pos[i].y+rr+4);
        SelectObject(hdc, (HBRUSH)GetStockObject(WHITE_BRUSH));
        DeleteObject(shadowBr);

        // Circle fill + coloured border
        HBRUSH nodeBr = CreateSolidBrush(fill);
        HPEN   nodePen = CreatePen(PS_SOLID, 3, col);
        SelectObject(hdc, nodeBr);
        SelectObject(hdc, nodePen);
        Ellipse(hdc, pos[i].x-rr, pos[i].y-rr,
                     pos[i].x+rr, pos[i].y+rr);
        SelectObject(hdc, (HBRUSH)GetStockObject(WHITE_BRUSH));
        SelectObject(hdc, (HPEN)GetStockObject(BLACK_PEN));
        DeleteObject(nodeBr);
        DeleteObject(nodePen);

        // Inner thin ring for depth (root gets double ring)
        if (i == 0) {
            HPEN ringPen = CreatePen(PS_SOLID, 1, col);
            HPEN rOld = (HPEN)SelectObject(hdc, ringPen);
            HBRUSH nullBr2 = (HBRUSH)GetStockObject(NULL_BRUSH);
            HBRUSH bOld = (HBRUSH)SelectObject(hdc, nullBr2);
            Ellipse(hdc, pos[i].x-rr+5, pos[i].y-rr+5,
                         pos[i].x+rr-5, pos[i].y+rr-5);
            SelectObject(hdc, rOld); SelectObject(hdc, bOld);
            DeleteObject(ringPen);
        }

        // Priority number
        SelectObject(hdc, hFontBold);
        SetTextColor(hdc, col);
        char priBuf[8];
        sprintf(priBuf, "%d", p->getCalculatedPriority());
        RECT nr = {pos[i].x-rr, pos[i].y-rr, pos[i].x+rr, pos[i].y+4};
        DrawTextA(hdc, priBuf, -1, &nr, DT_CENTER|DT_VCENTER|DT_SINGLELINE);

        // Patient name
        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, CLR_TEXT);
        string shortName = p->getName();
        if ((int)shortName.size() > 9) shortName = shortName.substr(0,8) + ".";
        RECT nnr = {pos[i].x-rr, pos[i].y+2, pos[i].x+rr, pos[i].y+rr};
        DrawTextA(hdc, shortName.c_str(), -1, &nnr, DT_CENTER|DT_TOP|DT_SINGLELINE);

        // Array-index badge (small pill above node)
        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, CLR_SUBTEXT);
        char idxBuf[8]; sprintf(idxBuf, "[%d]", i);
        RECT ibr = {pos[i].x-20, pos[i].y-rr-16, pos[i].x+20, pos[i].y-rr};
        DrawTextA(hdc, idxBuf, -1, &ibr, DT_CENTER|DT_VCENTER|DT_SINGLELINE);

        SelectObject(hdc, hFontBold);
    }

    // ── Legend (bottom strip) ─────────────────────────────────
    // Light grey band
    RECT legendBand = {0, winH-LEGEND_H, winW, winH};
    HBRUSH lgBr = CreateSolidBrush(RGB(238,242,250));
    FillRect(hdc, &legendBand, lgBr);
    DeleteObject(lgBr);
    // top border of legend band
    HPEN lgPen = CreatePen(PS_SOLID, 1, RGB(200,210,230));
    HPEN lgOld = (HPEN)SelectObject(hdc, lgPen);
    MoveToEx(hdc, 0, winH-LEGEND_H, NULL);
    LineTo(hdc, winW, winH-LEGEND_H);
    SelectObject(hdc, lgOld);
    DeleteObject(lgPen);

    struct LegItem { COLORREF c; const char* label; } legend[] = {
        {CLR_HEAP_ROOT, "Root — highest priority (treated first)"},
        {CLR_HEAP_L1,   "Level 1"},
        {CLR_HEAP_L2,   "Level 2"},
        {CLR_HEAP_L3,   "Level 3+ — lowest priority"},
    };
    SelectObject(hdc, hFontSmall);
    int lx = 20, ly = winH - LEGEND_H + 10;
    for (int i = 0; i < 4; i++) {
        // coloured dot
        HBRUSH dotBr = CreateSolidBrush(legend[i].c);
        RECT   dotR  = {lx, ly+i*18+2, lx+13, ly+i*18+13};
        // round dot via Ellipse
        HPEN dp = CreatePen(PS_SOLID,1,legend[i].c);
        SelectObject(hdc, dotBr); SelectObject(hdc, dp);
        Ellipse(hdc, lx, ly+i*18+2, lx+13, ly+i*18+13);
        SelectObject(hdc, (HBRUSH)GetStockObject(WHITE_BRUSH));
        SelectObject(hdc, (HPEN)GetStockObject(BLACK_PEN));
        DeleteObject(dotBr); DeleteObject(dp);
        // label
        SetTextColor(hdc, legend[i].c);
        RECT tr2 = {lx+18, ly+i*18, lx+340, ly+i*18+16};
        DrawTextA(hdc, legend[i].label, -1, &tr2, DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    }

    // Array representation label
    SetTextColor(hdc, CLR_SUBTEXT);
    char arrLabel[256] = "arr[ ";
    for (int i = 0; i < n && i < 12; i++) {
        char tmp2[16];
        sprintf(tmp2, "%d", ER->at(i)->getCalculatedPriority());
        strcat(arrLabel, tmp2);
        if (i < n-1 && i < 11) strcat(arrLabel, ", ");
    }
    if (n > 12) strcat(arrLabel, "...");
    strcat(arrLabel, " ]");
    RECT arrR = {winW/2, winH-LEGEND_H+8, winW-12, winH-8};
    DrawTextA(hdc, arrLabel, -1, &arrR, DT_RIGHT|DT_TOP|DT_SINGLELINE);

    SelectObject(hdc, oldFont);
}

// ── Heap window procedure ─────────────────────────────────────
LRESULT CALLBACK HeapWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);

        // Double-buffer
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
        HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

        // White background
        HBRUSH bgBr = CreateSolidBrush(CLR_HEAP_BG);
        FillRect(memDC, &rc, bgBr);
        DeleteObject(bgBr);

        // Subtle dot grid on white
        SetBkMode(memDC, TRANSPARENT);
        for (int gx = 30; gx < rc.right; gx += 36)
            for (int gy = 60; gy < rc.bottom - 90; gy += 36) {
                HPEN dp = CreatePen(PS_SOLID, 1, CLR_HEAP_GRID);
                HPEN op = (HPEN)SelectObject(memDC, dp);
                MoveToEx(memDC, gx, gy, NULL);
                LineTo(memDC, gx+1, gy);
                SelectObject(memDC, op);
                DeleteObject(dp);
            }

        DrawHeapTree(memDC, rc.right, rc.bottom);

        // Blit
        BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
        SelectObject(memDC, oldBmp);
        DeleteObject(memBmp);
        DeleteDC(memDC);

        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_TIMER:
        pulseFrame++;
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    case WM_DESTROY:
        KillTimer(hwnd, IDC_TIMER_PULSE);
        hHeapWnd = NULL;
        return 0;
    case WM_ERASEBKGND:
        return 1;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void OpenHeapWindow(HINSTANCE hInst) {
    if (hHeapWnd) {
        SetForegroundWindow(hHeapWnd);
        InvalidateRect(hHeapWnd, NULL, FALSE);
        return;
    }
    static bool registered = false;
    if (!registered) {
        WNDCLASSA wc = {};
        wc.lpfnWndProc   = HeapWndProc;
        wc.hInstance     = hInst;
        wc.lpszClassName = "HeapVisWin";
        wc.hbrBackground = CreateSolidBrush(CLR_HEAP_BG);
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        RegisterClassA(&wc);
        registered = true;
    }
    hHeapWnd = CreateWindowA(
        "HeapVisWin",
        "Min-Heap Visualizer  —  ER Priority Queue",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 860, 540,
        hWnd, NULL, hInst, NULL);
    ShowWindow(hHeapWnd, SW_SHOW);
    UpdateWindow(hHeapWnd);
    SetTimer(hHeapWnd, IDC_TIMER_PULSE, 600, NULL);
}

// ============================================================
//  REFRESH HELPERS
// ============================================================
void RefreshQueue() {
    SendMessage(hListQueue, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < ER->size(); i++) {
        Patient* p = ER->at(i);
        if (!p) continue;
        string row = (i==0 ? ">> " : "   ");
        row += "[P:" + intToStr(p->getCalculatedPriority()) + "]  ";
        row += p->getName();
        row += "  |  " + p->getSymptom();
        row += "  (ID #" + intToStr(p->getId()) + ")";
        SendMessageA(hListQueue, LB_ADDSTRING, 0, (LPARAM)row.c_str());
    }
    if (ER->size() > 0) {
        Patient* next = ER->at(0);
        string txt = "NEXT  >>  " + next->getName()
                   + "   [Priority " + intToStr(next->getCalculatedPriority()) + "]"
                   + "   --  " + next->getSymptom();
        SetWindowTextA(hLblNext, txt.c_str());
    } else {
        SetWindowTextA(hLblNext, "ER queue is currently empty.");
    }
    if (hHeapWnd) InvalidateRect(hHeapWnd, NULL, FALSE);
}

void RefreshLog() {
    SendMessage(hListLog, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < logQ->size(); i++) {
        Patient* p = logQ->at(i);
        if (!p) continue;
        string sym = p->getSymptom().empty() ? "(archived)" : p->getSymptom();
        string row = intToStr(i+1) + ".  ";
        row += p->getName();
        row += "  |  Priority: " + intToStr(p->getCalculatedPriority());
        row += "  |  Treated for: " + sym;
        row += "  (ID #" + intToStr(p->getId()) + ")";
        SendMessageA(hListLog, LB_ADDSTRING, 0, (LPARAM)row.c_str());
    }
}

void RefreshDB() {
    SendMessage(hListDB, LB_RESETCONTENT, 0, 0);
    const vector<Patient*>& all = db->getAllPatients();
    for (size_t i = 0; i < all.size(); i++) {
        Patient* p = all[i];
        string row = "#" + intToStr(p->getId()) + "  ";
        row += p->getName();
        row += "  |  " + p->history.historyString();
        SendMessageA(hListDB, LB_ADDSTRING, 0, (LPARAM)row.c_str());
    }
}

void RefreshAll() { RefreshQueue(); RefreshLog(); RefreshDB(); }

void ShowTab(int tab) {
    currentTab = tab;
    ShowWindow(hListQueue, tab==0 ? SW_SHOW : SW_HIDE);
    ShowWindow(hListLog,   tab==1 ? SW_SHOW : SW_HIDE);
    ShowWindow(hListDB,    tab==2 ? SW_SHOW : SW_HIDE);
}

// ============================================================
//  BUSINESS LOGIC
// ============================================================
void DoRegister() {
    bool isExisting = (SendMessage(hRadioExisting, BM_GETCHECK, 0, 0) == BST_CHECKED);
    char buf[256];
    GetWindowTextA(hEditSymptom, buf, 256);
    string symptom = string(buf);

    if (symptom.empty()) { SetStatus("Please enter a symptom.", true); return; }

    if (isExisting) {
        GetWindowTextA(hEditID, buf, 256);
        string idStr = string(buf);
        if (idStr.empty()) { SetStatus("Please enter a Patient ID.", true); return; }
        int id = atoi(idStr.c_str());
        if (id <= 0) { SetStatus("Invalid ID — must be a positive number.", true); return; }
        Patient* p = db->findById(id);
        if (!p) { SetStatus("No patient found with ID #" + idStr, true); return; }
        p->setNewSymptom(symptom);
        p->finalizePriority();
        ER->enqueue(p);
        SetStatus("Welcome back, " + p->getName()
                + "!  Priority: " + intToStr(p->getCalculatedPriority()));
    } else {
        GetWindowTextA(hEditName, buf, 256);
        string name = string(buf);
        if (name.empty()) { SetStatus("Please enter the patient's name.", true); return; }
        Patient* p = new Patient(name, symptom);
        p->finalizePriority();
        db->addPatientToVector(p);
        ER->enqueue(p);
        SetStatus("Registered " + name
                + "  |  ID #" + intToStr(p->getId())
                + "  |  Priority: " + intToStr(p->getCalculatedPriority()));
    }
    SetWindowTextA(hEditName, ""); SetWindowTextA(hEditSymptom, ""); SetWindowTextA(hEditID, "");
    RefreshAll();
}

void DoTreat() {
    Patient* p = ER->dequeue();
    if (!p) { SetStatus("ER queue is empty -- no patient to treat.", true); return; }
    string msg = "Treating " + p->getName() + " for " + p->getSymptom();
    p->archiveCurrentSymptom();
    logQ->addToLog(p);
    SetStatus(msg);
    RefreshAll();
}

// ============================================================
//  CONTROL FACTORY HELPERS
// ============================================================
HWND MakeLabel(HWND parent, const char* text, int x, int y, int w, int h, DWORD extra=0) {
    return CreateWindowA("STATIC", text,
        WS_CHILD|WS_VISIBLE|SS_LEFT|extra,
        x,y,w,h, parent, NULL,
        (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE), NULL);
}

HWND MakeEdit(HWND parent, int id, int x, int y, int w, int h, bool numOnly=false) {
    DWORD style = WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL;
    if (numOnly) style |= ES_NUMBER;
    return CreateWindowA("EDIT", "", style,
        x,y,w,h, parent, (HMENU)(intptr_t)id,
        (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE), NULL);
}

HWND MakeButton(HWND parent, const char* text, int id, int x, int y, int w, int h) {
    return CreateWindowA("BUTTON", text,
        WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|BS_OWNERDRAW,
        x,y,w,h, parent, (HMENU)(intptr_t)id,
        (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE), NULL);
}

HWND MakeRadio(HWND parent, const char* text, int id, int x, int y, int w, int h, bool checked=false) {
    HWND hw = CreateWindowA("BUTTON", text,
        WS_CHILD|WS_VISIBLE|BS_RADIOBUTTON,
        x,y,w,h, parent, (HMENU)(intptr_t)id,
        (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE), NULL);
    if (checked) SendMessage(hw, BM_SETCHECK, BST_CHECKED, 0);
    return hw;
}

HWND MakeGroup(HWND parent, const char* text, int id, int x, int y, int w, int h) {
    return CreateWindowA("BUTTON", text,
        WS_CHILD|WS_VISIBLE|BS_GROUPBOX,
        x,y,w,h, parent, (HMENU)(intptr_t)id,
        (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE), NULL);
}

HWND MakeListBox(HWND parent, int id, int x, int y, int w, int h) {
    return CreateWindowA("LISTBOX", "",
        WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_HSCROLL|LBS_NOTIFY|LBS_NOINTEGRALHEIGHT,
        x,y,w,h, parent, (HMENU)(intptr_t)id,
        (HINSTANCE)GetWindowLongPtr(parent, GWLP_HINSTANCE), NULL);
}

void ApplyFont(HWND hw, HFONT f) {
    SendMessage(hw, WM_SETFONT, (WPARAM)f, TRUE);
}

// ── Owner-draw button colour per ID ──────────────────────────
void GetButtonColors(int id, COLORREF& bg, COLORREF& text, COLORREF& border) {
    switch(id) {
    case IDC_BTN_REGISTER:
        bg=CLR_CYAN;    text=RGB(255,255,255); border=RGB(0,130,200); break;
    case IDC_BTN_TREAT:
        bg=CLR_MAGENTA; text=RGB(255,255,255); border=RGB(180,20,90); break;
    case IDC_BTN_SAVE:
        bg=RGB(50,65,100); text=RGB(200,215,255); border=RGB(80,100,150); break;
    case IDC_BTN_HEAP:
        bg=CLR_GREEN;   text=RGB(255,255,255); border=RGB(15,140,60); break;
    default:
        bg=RGB(240,243,250); text=CLR_TEXT; border=CLR_BORDER; break;
    }
}

// ============================================================
//  MAIN WINDOW PROCEDURE
// ============================================================
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_CREATE: {
        HINSTANCE hInst = ((CREATESTRUCT*)lParam)->hInstance;

        hFontMain  = CreateFontA(15,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,
                        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,
                        DEFAULT_PITCH, "Consolas");
        hFontBold  = CreateFontA(15,0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,
                        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,
                        DEFAULT_PITCH, "Consolas");
        hFontMono  = CreateFontA(14,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,
                        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,
                        DEFAULT_PITCH, "Lucida Console");
        hFontTitle = CreateFontA(20,0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,
                        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,
                        DEFAULT_PITCH, "Consolas");
        hFontSmall = CreateFontA(12,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,
                        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,
                        DEFAULT_PITCH, "Consolas");

        hBrushBg      = CreateSolidBrush(CLR_BG);      // soft white-blue (right panel)
        hBrushCard    = CreateSolidBrush(CLR_CARD);    // pure white
        hBrushPanel   = CreateSolidBrush(CLR_PANEL);   // light blue panel
        hBrushCyan    = CreateSolidBrush(CLR_CYAN);
        hBrushMagenta = CreateSolidBrush(CLR_MAGENTA);
        hBrushGreen   = CreateSolidBrush(CLR_GREEN);

        int LP=12, LW=308, RP=330;

        // ── Tab control ──────────────────────────────────
        hTab = CreateWindowA(WC_TABCONTROLA, "",
            WS_CHILD|WS_VISIBLE|TCS_HOTTRACK,
            RP, 10, 648, 28, hwnd, (HMENU)IDC_TAB_MAIN, hInst, NULL);
        ApplyFont(hTab, hFontBold);

        TCITEMA tie; tie.mask = TCIF_TEXT;
        tie.pszText = (LPSTR)"  ER Queue  ";
        TabCtrl_InsertItem(hTab, 0, &tie);
        tie.pszText = (LPSTR)"  Treatment Log  ";
        TabCtrl_InsertItem(hTab, 1, &tie);
        tie.pszText = (LPSTR)"  Patient Database  ";
        TabCtrl_InsertItem(hTab, 2, &tie);

        // ── List boxes ───────────────────────────────────
        hListQueue = MakeListBox(hwnd, IDC_LIST_QUEUE, RP, 42, 648, 480);
        hListLog   = MakeListBox(hwnd, IDC_LIST_LOG,   RP, 42, 648, 480);
        hListDB    = MakeListBox(hwnd, IDC_LIST_DB,    RP, 42, 648, 480);

        // ── Register group ───────────────────────────────
        hGrpReg = MakeGroup(hwnd, "  Register Patient", IDC_GRP_REGISTER, LP, 10, LW, 300);

        hRadioNew      = MakeRadio(hwnd, "New Patient",      IDC_RADIO_NEW,      LP+15, 36, 120, 22, true);
        hRadioExisting = MakeRadio(hwnd, "Existing Patient", IDC_RADIO_EXISTING, LP+148,36, 148, 22);

        hLblName   = MakeLabel(hwnd, "Full Name:",       LP+15,  66, LW-30, 18);
        hEditName  = MakeEdit (hwnd, IDC_EDIT_NAME,      LP+15,  86, LW-30, 26);
        hLblID     = MakeLabel(hwnd, "Patient ID:",      LP+15,  66, LW-30, 18);
        hEditID    = MakeEdit (hwnd, IDC_EDIT_ID,        LP+15,  86, LW-30, 26, true);

        hLblSymptom  = MakeLabel(hwnd, "Current Symptom:", LP+15, 120, LW-30, 18);
        hEditSymptom = MakeEdit (hwnd, IDC_EDIT_SYMPTOM,   LP+15, 140, LW-30, 26);

        MakeLabel(hwnd, "Tip: match names in symptoms_ref.csv", LP+15, 174, LW-30, 16);

        hBtnRegister = MakeButton(hwnd, "Add to ER Queue", IDC_BTN_REGISTER, LP+15, 198, LW-30, 34);

        MakeLabel(hwnd,
            "Priority = Base Score - History Risk\r\n"
            "Lower score = treated first  (Min-Heap)",
            LP+15, 242, LW-30, 52);

        // ── Treat group ──────────────────────────────────
        hGrpTreat = MakeGroup(hwnd, "  Treat Next Patient", IDC_GRP_TREAT, LP, 320, LW, 118);
        hLblNext  = MakeLabel(hwnd, "ER queue is currently empty.", LP+15, 342, LW-30, 40, SS_WORDELLIPSIS);
        hBtnTreat = MakeButton(hwnd, "Treat Next Patient  (Dequeue)", IDC_BTN_TREAT, LP+15, 390, LW-30, 34);

        // ── Heap visualizer button ───────────────────────
        hBtnHeap = MakeButton(hwnd, "Show Heap Visualizer", IDC_BTN_HEAP, LP, 450, LW, 30);

        // ── Save / Status ────────────────────────────────
        hBtnSave = MakeButton(hwnd, "Save & Exit", IDC_BTN_SAVE, LP, 490, LW, 30);
        hStatus  = MakeLabel (hwnd, "System ready.", LP, 532, 978, 22, SS_WORDELLIPSIS);

        // ── Apply fonts ──────────────────────────────────
        HWND controls[] = {
            hGrpReg, hGrpTreat,
            hRadioNew, hRadioExisting,
            hLblName, hEditName, hLblID, hEditID,
            hLblSymptom, hEditSymptom,
            hBtnRegister, hBtnTreat, hBtnSave, hBtnHeap,
            hLblNext, hStatus, hListQueue, hListLog, hListDB
        };
        for (int i = 0; i < 19; i++) ApplyFont(controls[i], hFontMain);
        ApplyFont(hBtnRegister, hFontBold);
        ApplyFont(hBtnTreat,    hFontBold);
        ApplyFont(hBtnSave,     hFontBold);
        ApplyFont(hBtnHeap,     hFontBold);

        ShowWindow(hLblID,  SW_HIDE);
        ShowWindow(hEditID, SW_HIDE);
        ShowTab(0);
        break;
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == IDC_RADIO_NEW) {
            SendMessage(hRadioNew,      BM_SETCHECK, BST_CHECKED,   0);
            SendMessage(hRadioExisting, BM_SETCHECK, BST_UNCHECKED, 0);
            ShowWindow(hLblName,  SW_SHOW); ShowWindow(hEditName,  SW_SHOW);
            ShowWindow(hLblID,    SW_HIDE); ShowWindow(hEditID,    SW_HIDE);
            SetWindowTextA(hBtnRegister, "Add New Patient to Queue");
        }
        else if (id == IDC_RADIO_EXISTING) {
            SendMessage(hRadioExisting, BM_SETCHECK, BST_CHECKED,   0);
            SendMessage(hRadioNew,      BM_SETCHECK, BST_UNCHECKED, 0);
            ShowWindow(hLblName,  SW_HIDE); ShowWindow(hEditName,  SW_HIDE);
            ShowWindow(hLblID,    SW_SHOW); ShowWindow(hEditID,    SW_SHOW);
            SetWindowTextA(hBtnRegister, "Add Existing Patient to Queue");
        }
        else if (id == IDC_BTN_REGISTER) DoRegister();
        else if (id == IDC_BTN_TREAT)    DoTreat();
        else if (id == IDC_BTN_HEAP) {
            HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
            OpenHeapWindow(hInst);
        }
        else if (id == IDC_BTN_SAVE) {
            db->saveAllToFile();
            SetStatus("Records saved. Goodbye!");
            Sleep(700);
            PostQuitMessage(0);
        }
        break;
    }

    case WM_NOTIFY: {
        NMHDR* nm = (NMHDR*)lParam;
        if (nm->idFrom == IDC_TAB_MAIN && nm->code == TCN_SELCHANGE)
            ShowTab(TabCtrl_GetCurSel(hTab));
        break;
    }

    // ── Owner-draw buttons ────────────────────────────────────
    case WM_DRAWITEM: {
        DRAWITEMSTRUCT* di = (DRAWITEMSTRUCT*)lParam;
        if (di->CtlType != ODT_BUTTON) break;
        int bid = di->CtlID;
        COLORREF bg, txt, bord;
        GetButtonColors(bid, bg, txt, bord);

        bool pressed = (di->itemState & ODS_SELECTED) != 0;

        char label[128] = "";
        GetWindowTextA(di->hwndItem, label, 128);

        // Draw rounded button shape
        HDC dc = di->hDC;
        RECT r  = di->rcItem;
        if (pressed) { r.left+=1; r.top+=1; }

        DrawRoundedRect(dc, r.left, r.top,
                        r.right - r.left, r.bottom - r.top,
                        8, bg, bord, 2);

        // Text
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, txt);
        HFONT oldF = (HFONT)SelectObject(dc, hFontBold);
        DrawTextA(dc, label, -1, &r, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        SelectObject(dc, oldF);

        // Focus dotted border
        if (di->itemState & ODS_FOCUS) {
            InflateRect(&r, -3, -3);
            DrawFocusRect(dc, &r);
        }
        return TRUE;
    }

    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        HWND ctrl = (HWND)lParam;
        SetBkMode(hdc, TRANSPARENT);
        if (ctrl == hStatus) {
            bool err = (GetWindowLongPtr(hStatus, GWLP_USERDATA) == 1);
            SetTextColor(hdc, err ? CLR_MAGENTA : CLR_GREEN);
            return (LRESULT)hBrushBg;   // white bg for status bar area
        } else if (ctrl == hLblNext) {
            SetTextColor(hdc, CLR_ORANGE);
            return (LRESULT)CreateSolidBrush(CLR_SIDEBAR); // navy bg
        } else {
            // Sidebar labels (left panel) → white text on navy
            SetTextColor(hdc, RGB(220, 230, 255));
            return (LRESULT)CreateSolidBrush(CLR_SIDEBAR);
        }
    }

    case WM_CTLCOLOREDIT: {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, RGB(255, 255, 255));
        SetTextColor(hdc, CLR_TEXT);
        return (LRESULT)hBrushCard;
    }

    case WM_CTLCOLORLISTBOX: {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, RGB(255, 255, 255));
        SetTextColor(hdc, CLR_TEXT);
        return (LRESULT)hBrushCard;
    }

    case WM_ERASEBKGND: {
        RECT rc;
        GetClientRect(hwnd, &rc);
        HDC hdc = (HDC)wParam;

        // White main background
        FillRect(hdc, &rc, hBrushBg);

        // Navy left sidebar panel (behind controls, purely decorative depth)
        RECT sidePanel = {0, 0, 322, rc.bottom};
        HBRUSH sideBr = CreateSolidBrush(CLR_SIDEBAR);
        FillRect(hdc, &sidePanel, sideBr);
        DeleteObject(sideBr);

        // Bright cyan right-edge accent line on sidebar
        HPEN accentPen = CreatePen(PS_SOLID, 4, CLR_CYAN);
        HPEN oldPen    = (HPEN)SelectObject(hdc, accentPen);
        MoveToEx(hdc, 322, 0,          NULL);
        LineTo  (hdc, 322, rc.bottom);
        SelectObject(hdc, oldPen);
        DeleteObject(accentPen);

        // Top accent bar on sidebar (magenta stripe)
        HPEN topPen = CreatePen(PS_SOLID, 3, CLR_MAGENTA);
        oldPen = (HPEN)SelectObject(hdc, topPen);
        MoveToEx(hdc, 0, 0, NULL);
        LineTo(hdc, 322, 0);
        SelectObject(hdc, oldPen);
        DeleteObject(topPen);

        // Three coloured indicator dots (top-right corner)
        struct { COLORREF c; int ox; } dots[] = {
            {CLR_MAGENTA, 22},
            {CLR_CYAN,    42},
            {CLR_GREEN,   62},
        };
        for (int d = 0; d < 3; d++) {
            HBRUSH dotBr = CreateSolidBrush(dots[d].c);
            HPEN   dotPen2 = CreatePen(PS_SOLID,1,dots[d].c);
            SelectObject(hdc, dotBr);
            SelectObject(hdc, dotPen2);
            Ellipse(hdc, rc.right-dots[d].ox, 6,
                         rc.right-dots[d].ox+14, 20);
            SelectObject(hdc, (HBRUSH)GetStockObject(WHITE_BRUSH));
            SelectObject(hdc, (HPEN)GetStockObject(BLACK_PEN));
            DeleteObject(dotBr); DeleteObject(dotPen2);
        }

        // Subtle bottom status bar band (white side only)
        RECT sb = {323, rc.bottom-28, rc.right, rc.bottom};
        HBRUSH sbBr = CreateSolidBrush(RGB(235, 240, 250));
        FillRect(hdc, &sb, sbBr);
        DeleteObject(sbBr);
        HPEN sepPen = CreatePen(PS_SOLID, 1, CLR_BORDER);
        oldPen = (HPEN)SelectObject(hdc, sepPen);
        MoveToEx(hdc, 323, rc.bottom-28, NULL);
        LineTo(hdc, rc.right, rc.bottom-28);
        SelectObject(hdc, oldPen);
        DeleteObject(sepPen);

        return 1;
    }

    case WM_DESTROY:
        DeleteObject(hFontMain);  DeleteObject(hFontBold);
        DeleteObject(hFontMono);  DeleteObject(hFontTitle);
        DeleteObject(hFontSmall);
        DeleteObject(hBrushBg);   DeleteObject(hBrushCard);
        DeleteObject(hBrushPanel);DeleteObject(hBrushCyan);
        DeleteObject(hBrushMagenta); DeleteObject(hBrushGreen);
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ============================================================
//  WinMain
// ============================================================
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    InitCommonControls();

    db = new PatientDatabase("patients.csv");
    Patient::setCount(db->getMaxID("patients.csv"));
    db->loadAllPatientsFromFile();

    ER   = new EmergencyQueue(100);
    logQ = new TreatmentLog(100);

    WNDCLASSA wc   = {};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.lpszClassName = "ERManagementWin";
    wc.hbrBackground = CreateSolidBrush(CLR_BG);   // soft white-blue
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassA(&wc);

    hWnd = CreateWindowA(
        "ERManagementWin",
        "Hospital ER Management System  //  Priority-Heap Engine",
        WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 1000, 580,
        NULL, NULL, hInst, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    RefreshAll();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    delete ER;
    delete logQ;
    delete db;
    return (int)msg.wParam;
}
