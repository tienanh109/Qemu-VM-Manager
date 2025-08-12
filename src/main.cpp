#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <sstream>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")

// --- Main Control IDs ---
#define IDC_TAB_CONTROL         100
#define IDC_START_BTN           101
#define IDC_CLEAR_BTN           102
#define IDC_LOG_EDIT            103

// --- Basic Tab IDs ---
#define IDC_VMNAME_EDIT         201
#define IDC_RAM_EDIT            202
#define IDC_CPU_CORES_EDIT      203
#define IDC_CPU_THREADS_EDIT    204
#define IDC_STATIC_VMNAME       205
#define IDC_STATIC_RAM          206
#define IDC_STATIC_CPU_CORES    207
#define IDC_STATIC_CPU_THREADS  208

// --- Drives Tab IDs ---
#define IDC_DISK_A_EDIT         301
#define IDC_DISK_A_BROWSE       302
#define IDC_DISK_B_EDIT         303
#define IDC_DISK_B_BROWSE       304
#define IDC_DISK_C_EDIT         305
#define IDC_DISK_C_BROWSE       306
#define IDC_FLOPPY_EDIT         307
#define IDC_FLOPPY_BROWSE       308
#define IDC_ISO_EDIT            309
#define IDC_ISO_BROWSE_BTN      310
#define IDC_CREATEDISK_BTN      311
#define IDC_STATIC_DISK_A       312
#define IDC_STATIC_DISK_B       313
#define IDC_STATIC_DISK_C       314
#define IDC_STATIC_FLOPPY       315
#define IDC_STATIC_ISO          316

// --- Hardware Tab IDs ---
#define IDC_AUDIO_COMBO         401
#define IDC_VRAM_EDIT           402
#define IDC_BOOT_EDIT           403
// #define IDC_ACPI_CHECK          404
#define IDC_VNC_EDIT            405
#define IDC_STATIC_AUDIO        406
#define IDC_STATIC_VRAM         407
#define IDC_STATIC_BOOT         408
#define IDC_STATIC_VNC          409

// --- Advanced Tab IDs ---
#define IDC_CPU_MODEL_COMBO     501
#define IDC_MACHINE_COMBO       502
#define IDC_VGA_COMBO           503
#define IDC_NET_COMBO           504
#define IDC_ACCEL_CHECK         505
#define IDC_EXTRA_FLAGS_EDIT    506
#define IDC_STATIC_CPU_MODEL    507
#define IDC_STATIC_MACHINE      508
#define IDC_STATIC_VGA          509
#define IDC_STATIC_NET          510
#define IDC_STATIC_EXTRA_FLAGS  511

// --- Custom Tab IDs ---
#define IDC_CUSTOM_MODE_CHECK   601
#define IDC_CUSTOM_CMD_EDIT     602
#define IDC_QEMU_PATH_EDIT      603
#define IDC_QEMU_IMG_PATH_EDIT  604
#define IDC_PREVIEW_CMD_EDIT    605
#define IDC_UPDATE_PREVIEW_BTN  606
#define IDC_STATIC_CUSTOM_CMD   607
#define IDC_STATIC_QEMU_PATH    608
#define IDC_STATIC_QEMU_IMG_PATH 609
#define IDC_STATIC_PREVIEW_CMD  610

// --- Menu IDs ---
#define IDM_FILE_EXIT           1001
#define IDM_FILE_RESET_EXIT     1002
#define IDM_HELP_ABOUT          1003

// --- Create Disk Dialog IDs ---
#define IDD_CREATEDISK          2001
#define IDC_CD_PATH_EDIT        2002
#define IDC_CD_PATH_BROWSE      2003
#define IDC_CD_FORMAT_COMBO     2004
#define IDC_CD_SIZE_EDIT        2005
#define IDC_CD_CREATE           2006
#define IDC_CD_CANCEL           2007

// --- Global Handles ---
HINSTANCE hInst;
HWND hTab, hLog, hStartBtn, hClearBtn;
HWND hTabBasic, hTabDrives, hTabHardware, hTabAdvanced, hTabCustom;
HWND hVmName, hRam, hCpuCores, hCpuThreads;
HWND hDiskA, hDiskB, hDiskC, hFloppy, hIso, hCreateDiskBtn;
HWND hAudio, hVram, hBoot, hVnc; // tat hAcpi
HWND hCpuModel, hMachine, hVga, hNet, hAccel, hExtraFlags;
HWND hCustomMode, hCustomCmd, hQemuPath, hQemuImgPath, hPreviewCmd, hUpdatePreview;
std::wstring qemuPath; // Legacy global path, now primarily driven by hQemuPath control

// --- Function Prototypes ---
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK CreateDiskDlgProc(HWND, UINT, WPARAM, LPARAM);
void CreateMainControls(HWND);
void AddMenus(HWND);
void HandleTabChange(HWND);
void SaveSettings(HWND);
void LoadSettings(HWND);
void FindQemuPath(HWND);
void ClearAllFields();
std::wstring BrowseForFile(HWND, const wchar_t*, const wchar_t*, bool);
void StartVm(HWND);
void AppendLog(const std::wstring&);
void ShowCreateDiskDialog(HWND);
std::wstring BuildQemuCommand(HWND);
void UpdatePreviewCommand(HWND);

// --- WinMain ---
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    hInst = hInstance;
    const wchar_t CLASS_NAME[] = L"QemuGuiManagerWindowClass";
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    DWORD dwStyle = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX;

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"QEMU GUI Manager by tienanh109",
        dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, 700, 740,
        NULL, NULL, hInstance, NULL);

    if (!hwnd) return 0;
    ShowWindow(hwnd, nCmdShow);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

// --- Main Window Procedure ---
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        AddMenus(hwnd);
        CreateMainControls(hwnd);
        LoadSettings(hwnd);
        FindQemuPath(hwnd);
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_DISK_A_BROWSE: {
            std::wstring path = BrowseForFile(hwnd, L"Disk Images (*.qcow2;*.img)\0*.qcow2;*.img\0All Files (*.*)\0*.*\0", L"Select Disk Image File", true);
            if (!path.empty()) SetWindowText(hDiskA, path.c_str());
            break;
        }
        case IDC_DISK_B_BROWSE: {
            std::wstring path = BrowseForFile(hwnd, L"Disk Images (*.qcow2;*.img)\0*.qcow2;*.img\0All Files (*.*)\0*.*\0", L"Select Disk Image File", true);
            if (!path.empty()) SetWindowText(hDiskB, path.c_str());
            break;
        }
        case IDC_DISK_C_BROWSE: {
            std::wstring path = BrowseForFile(hwnd, L"Disk Images (*.qcow2;*.img)\0*.qcow2;*.img\0All Files (*.*)\0*.*\0", L"Select Disk Image File", true);
            if (!path.empty()) SetWindowText(hDiskC, path.c_str());
            break;
        }
        case IDC_FLOPPY_BROWSE: {
            std::wstring path = BrowseForFile(hwnd, L"Floppy Images (*.img;*.ima)\0*.img;*.ima\0All Files (*.*)\0*.*\0", L"Select Floppy Image File", true);
            if (!path.empty()) SetWindowText(hFloppy, path.c_str());
            break;
        }
        case IDC_ISO_BROWSE_BTN: {
            std::wstring path = BrowseForFile(hwnd, L"ISO Images (*.iso)\0*.iso\0All Files (*.*)\0*.*\0", L"Select ISO File", true);
            if (!path.empty()) SetWindowText(hIso, path.c_str());
            break;
        }
        case IDC_CREATEDISK_BTN:
            ShowCreateDiskDialog(hwnd);
            break;
        case IDC_START_BTN:
            StartVm(hwnd);
            break;
        case IDC_CLEAR_BTN:
            ClearAllFields();
            break;
        case IDC_UPDATE_PREVIEW_BTN:
            UpdatePreviewCommand(hwnd);
            break;
        case IDM_FILE_EXIT:
            DestroyWindow(hwnd);
            break;
        case IDM_FILE_RESET_EXIT:
            if (MessageBox(hwnd, L"This will delete all saved settings from the registry and close the application.\nAre you sure?", L"Confirm Reset", MB_YESNO | MB_ICONWARNING) == IDYES) {
                RegDeleteKey(HKEY_CURRENT_USER, L"Software\\tienanh109\\QemuGuiManager");
                DestroyWindow(hwnd);
            }
            break;
        case IDM_HELP_ABOUT:
            MessageBox(hwnd, L"QEMU GUI Manager v1.0 (Beta with Win32)\nDeveloped by tienanh109\nhttps://github.com/tienanh109", L"About", MB_OK | MB_ICONINFORMATION);
            break;
        }
        break;
    case WM_NOTIFY:
        if (((LPNMHDR)lParam)->idFrom == IDC_TAB_CONTROL && ((LPNMHDR)lParam)->code == TCN_SELCHANGE) {
            HandleTabChange(hwnd);
        }
        break;
    case WM_CLOSE:
        SaveSettings(hwnd);
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// --- UI Creation ---
void AddMenus(HWND hwnd) {
    HMENU hMenubar = CreateMenu();
    HMENU hFileMenu = CreateMenu();
    HMENU hHelpMenu = CreateMenu();

    AppendMenu(hFileMenu, MF_STRING, IDM_FILE_RESET_EXIT, L"Reset & Exit");
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFileMenu, MF_STRING, IDM_FILE_EXIT, L"&Exit");
    
    AppendMenu(hHelpMenu, MF_STRING, IDM_HELP_ABOUT, L"&About");

    AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");
    AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hHelpMenu, L"&Help");

    SetMenu(hwnd, hMenubar);
}

void CreateMainControls(HWND hwnd) {
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    // Create Tab Control
    hTab = CreateWindow(WC_TABCONTROL, L"", WS_CHILD | WS_VISIBLE, 10, 10, 660, 400, hwnd, (HMENU)IDC_TAB_CONTROL, hInst, NULL);
    TCITEM tie;
    tie.mask = TCIF_TEXT;
    tie.pszText = (LPWSTR)L"Basic & CPU"; TabCtrl_InsertItem(hTab, 0, &tie);
    tie.pszText = (LPWSTR)L"Drives & Media"; TabCtrl_InsertItem(hTab, 1, &tie);
    tie.pszText = (LPWSTR)L"Hardware"; TabCtrl_InsertItem(hTab, 2, &tie);
    tie.pszText = (LPWSTR)L"Advanced"; TabCtrl_InsertItem(hTab, 3, &tie);
    tie.pszText = (LPWSTR)L"Custom"; TabCtrl_InsertItem(hTab, 4, &tie);

    // --- Create Group Boxes for each tab ---
    hTabBasic = CreateWindow(L"BUTTON", L"Basic Settings", WS_CHILD | BS_GROUPBOX, 15, 45, 645, 355, hwnd, NULL, hInst, NULL);
    hTabDrives = CreateWindow(L"BUTTON", L"Drives & Media", WS_CHILD | BS_GROUPBOX, 15, 45, 645, 355, hwnd, NULL, hInst, NULL);
    hTabHardware = CreateWindow(L"BUTTON", L"Hardware Emulation", WS_CHILD | BS_GROUPBOX, 15, 45, 645, 355, hwnd, NULL, hInst, NULL);
    hTabAdvanced = CreateWindow(L"BUTTON", L"Advanced Options", WS_CHILD | BS_GROUPBOX, 15, 45, 645, 355, hwnd, NULL, hInst, NULL);
    hTabCustom = CreateWindow(L"BUTTON", L"Custom Command & Paths", WS_CHILD | BS_GROUPBOX, 15, 45, 645, 355, hwnd, NULL, hInst, NULL);
    
    // --- Basic & CPU Tab Controls ---
    CreateWindow(L"STATIC", L"VM Name:", WS_CHILD, 30, 75, 100, 20, hwnd, (HMENU)IDC_STATIC_VMNAME, hInst, NULL);
    hVmName = CreateWindow(L"EDIT", L"MyWin32VM", WS_CHILD | WS_BORDER, 140, 70, 500, 25, hwnd, (HMENU)IDC_VMNAME_EDIT, hInst, NULL);
    CreateWindow(L"STATIC", L"RAM (MB):", WS_CHILD, 30, 115, 100, 20, hwnd, (HMENU)IDC_STATIC_RAM, hInst, NULL);
    hRam = CreateWindow(L"EDIT", L"2048", WS_CHILD | WS_BORDER | ES_NUMBER, 140, 110, 120, 25, hwnd, (HMENU)IDC_RAM_EDIT, hInst, NULL);
    CreateWindow(L"STATIC", L"CPU Cores:", WS_CHILD, 30, 155, 100, 20, hwnd, (HMENU)IDC_STATIC_CPU_CORES, hInst, NULL);
    hCpuCores = CreateWindow(L"EDIT", L"2", WS_CHILD | WS_BORDER | ES_NUMBER, 140, 150, 120, 25, hwnd, (HMENU)IDC_CPU_CORES_EDIT, hInst, NULL);
    CreateWindow(L"STATIC", L"Threads/Core:", WS_CHILD, 30, 195, 100, 20, hwnd, (HMENU)IDC_STATIC_CPU_THREADS, hInst, NULL);
    hCpuThreads = CreateWindow(L"EDIT", L"1", WS_CHILD | WS_BORDER | ES_NUMBER, 140, 190, 120, 25, hwnd, (HMENU)IDC_CPU_THREADS_EDIT, hInst, NULL);

    // --- Drives Tab Controls ---
    CreateWindow(L"STATIC", L"Disk A (hda):", WS_CHILD, 30, 75, 100, 20, hwnd, (HMENU)IDC_STATIC_DISK_A, hInst, NULL);
    hDiskA = CreateWindow(L"EDIT", L"", WS_CHILD | WS_BORDER, 140, 70, 410, 25, hwnd, (HMENU)IDC_DISK_A_EDIT, hInst, NULL);
    CreateWindow(L"BUTTON", L"Browse...", WS_CHILD, 560, 70, 80, 25, hwnd, (HMENU)IDC_DISK_A_BROWSE, hInst, NULL);
    CreateWindow(L"STATIC", L"Disk B (hdb):", WS_CHILD, 30, 115, 100, 20, hwnd, (HMENU)IDC_STATIC_DISK_B, hInst, NULL);
    hDiskB = CreateWindow(L"EDIT", L"", WS_CHILD | WS_BORDER, 140, 110, 410, 25, hwnd, (HMENU)IDC_DISK_B_EDIT, hInst, NULL);
    CreateWindow(L"BUTTON", L"Browse...", WS_CHILD, 560, 110, 80, 25, hwnd, (HMENU)IDC_DISK_B_BROWSE, hInst, NULL);
    CreateWindow(L"STATIC", L"Disk C (hdc):", WS_CHILD, 30, 155, 100, 20, hwnd, (HMENU)IDC_STATIC_DISK_C, hInst, NULL);
    hDiskC = CreateWindow(L"EDIT", L"", WS_CHILD | WS_BORDER, 140, 150, 410, 25, hwnd, (HMENU)IDC_DISK_C_EDIT, hInst, NULL);
    CreateWindow(L"BUTTON", L"Browse...", WS_CHILD, 560, 150, 80, 25, hwnd, (HMENU)IDC_DISK_C_BROWSE, hInst, NULL);
    CreateWindow(L"STATIC", L"Floppy (fda):", WS_CHILD, 30, 195, 100, 20, hwnd, (HMENU)IDC_STATIC_FLOPPY, hInst, NULL);
    hFloppy = CreateWindow(L"EDIT", L"", WS_CHILD | WS_BORDER, 140, 190, 410, 25, hwnd, (HMENU)IDC_FLOPPY_EDIT, hInst, NULL);
    CreateWindow(L"BUTTON", L"Browse...", WS_CHILD, 560, 190, 80, 25, hwnd, (HMENU)IDC_FLOPPY_BROWSE, hInst, NULL);
    CreateWindow(L"STATIC", L"CD/DVD (iso):", WS_CHILD, 30, 235, 100, 20, hwnd, (HMENU)IDC_STATIC_ISO, hInst, NULL);
    hIso = CreateWindow(L"EDIT", L"", WS_CHILD | WS_BORDER, 140, 230, 410, 25, hwnd, (HMENU)IDC_ISO_EDIT, hInst, NULL);
    CreateWindow(L"BUTTON", L"Browse...", WS_CHILD, 560, 230, 80, 25, hwnd, (HMENU)IDC_ISO_BROWSE_BTN, hInst, NULL);
    // hCreateDiskBtn = CreateWindow(L"BUTTON", L"Create New Disk Image...", WS_CHILD, 140, 270, 200, 30, hwnd, (HMENU)IDC_CREATEDISK_BTN, hInst, NULL);

    // --- Hardware Tab Controls ---
    CreateWindow(L"STATIC", L"Audio:", WS_CHILD, 30, 75, 100, 20, hwnd, (HMENU)IDC_STATIC_AUDIO, hInst, NULL);
    hAudio = CreateWindow(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_CHILD | WS_VSCROLL, 140, 70, 180, 100, hwnd, (HMENU)IDC_AUDIO_COMBO, hInst, NULL);
    SendMessage(hAudio, CB_ADDSTRING, 0, (LPARAM)L"Disabled");
    SendMessage(hAudio, CB_ADDSTRING, 0, (LPARAM)L"Enabled (dsound)");
    SendMessage(hAudio, CB_SETCURSEL, 1, 0);

    CreateWindow(L"STATIC", L"VRAM (MB):", WS_CHILD, 30, 115, 100, 20, hwnd, (HMENU)IDC_STATIC_VRAM, hInst, NULL);
    hVram = CreateWindow(L"EDIT", L"16", WS_CHILD | WS_BORDER | ES_NUMBER, 140, 110, 120, 25, hwnd, (HMENU)IDC_VRAM_EDIT, hInst, NULL);

    CreateWindow(L"STATIC", L"Boot Order:", WS_CHILD, 30, 155, 100, 20, hwnd, (HMENU)IDC_STATIC_BOOT, hInst, NULL);
    hBoot = CreateWindow(L"EDIT", L"d", WS_CHILD | WS_BORDER, 140, 150, 120, 25, hwnd, (HMENU)IDC_BOOT_EDIT, hInst, NULL);

    // hAcpi = CreateWindow(L"BUTTON", L"Enable ACPI", WS_CHILD | BS_AUTOCHECKBOX, 140, 190, 150, 20, hwnd, (HMENU)IDC_ACPI_CHECK, hInst, NULL);
    // CheckDlgButton(hwnd, IDC_ACPI_CHECK, BST_CHECKED);

    CreateWindow(L"STATIC", L"VNC Display:", WS_CHILD, 30, 225, 100, 20, hwnd, (HMENU)IDC_STATIC_VNC, hInst, NULL);
    hVnc = CreateWindow(L"EDIT", L"", WS_CHILD | WS_BORDER, 140, 220, 120, 25, hwnd, (HMENU)IDC_VNC_EDIT, hInst, NULL);

    // --- Advanced Tab Controls ---
    CreateWindow(L"STATIC", L"CPU Model:", WS_CHILD, 30, 75, 100, 20, hwnd, (HMENU)IDC_STATIC_CPU_MODEL, hInst, NULL);
    hCpuModel = CreateWindow(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_CHILD | WS_VSCROLL, 140, 70, 180, 200, hwnd, (HMENU)IDC_CPU_MODEL_COMBO, hInst, NULL);
    SendMessage(hCpuModel, CB_ADDSTRING, 0, (LPARAM)L"max (best compatibility)");
    SendMessage(hCpuModel, CB_ADDSTRING, 0, (LPARAM)L"host (passthrough)");
    SendMessage(hCpuModel, CB_ADDSTRING, 0, (LPARAM)L"qemu64");
    SendMessage(hCpuModel, CB_ADDSTRING, 0, (LPARAM)L"kvm64");
    SendMessage(hCpuModel, CB_ADDSTRING, 0, (LPARAM)L"core2duo");
    SendMessage(hCpuModel, CB_ADDSTRING, 0, (LPARAM)L"phenom");
    SendMessage(hCpuModel, CB_SETCURSEL, 0, 0);

    CreateWindow(L"STATIC", L"Machine Type:", WS_CHILD, 30, 115, 100, 20, hwnd, (HMENU)IDC_STATIC_MACHINE, hInst, NULL);
    hMachine = CreateWindow(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_CHILD | WS_VSCROLL, 140, 110, 180, 100, hwnd, (HMENU)IDC_MACHINE_COMBO, hInst, NULL);
    SendMessage(hMachine, CB_ADDSTRING, 0, (LPARAM)L"pc (i440fx, default)");
    SendMessage(hMachine, CB_ADDSTRING, 0, (LPARAM)L"q35");
    SendMessage(hMachine, CB_SETCURSEL, 0, 0);

    CreateWindow(L"STATIC", L"VGA Card:", WS_CHILD, 30, 155, 100, 20, hwnd, (HMENU)IDC_STATIC_VGA, hInst, NULL);
    hVga = CreateWindow(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_CHILD | WS_VSCROLL, 140, 150, 180, 100, hwnd, (HMENU)IDC_VGA_COMBO, hInst, NULL);
    SendMessage(hVga, CB_ADDSTRING, 0, (LPARAM)L"std");
    SendMessage(hVga, CB_ADDSTRING, 0, (LPARAM)L"virtio");
    SendMessage(hVga, CB_ADDSTRING, 0, (LPARAM)L"qxl");
    SendMessage(hVga, CB_SETCURSEL, 0, 0);

    CreateWindow(L"STATIC", L"Network Card:", WS_CHILD, 30, 195, 100, 20, hwnd, (HMENU)IDC_STATIC_NET, hInst, NULL);
    hNet = CreateWindow(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_CHILD | WS_VSCROLL, 140, 190, 180, 100, hwnd, (HMENU)IDC_NET_COMBO, hInst, NULL);
    SendMessage(hNet, CB_ADDSTRING, 0, (LPARAM)L"virtio-net-pci");
    SendMessage(hNet, CB_ADDSTRING, 0, (LPARAM)L"e1000");
    SendMessage(hNet, CB_ADDSTRING, 0, (LPARAM)L"rtl8139");
    SendMessage(hNet, CB_SETCURSEL, 0, 0);

    hAccel = CreateWindow(L"BUTTON", L"Use WHPX Acceleration", WS_CHILD | BS_AUTOCHECKBOX, 140, 230, 300, 20, hwnd, (HMENU)IDC_ACCEL_CHECK, hInst, NULL);
    CheckDlgButton(hwnd, IDC_ACCEL_CHECK, BST_UNCHECKED);

    CreateWindow(L"STATIC", L"Extra Flags:", WS_CHILD, 30, 265, 100, 20, hwnd, (HMENU)IDC_STATIC_EXTRA_FLAGS, hInst, NULL);
    hExtraFlags = CreateWindow(L"EDIT", L"", WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 140, 260, 500, 25, hwnd, (HMENU)IDC_EXTRA_FLAGS_EDIT, hInst, NULL);

    // --- Custom Tab Controls ---
    hCustomMode = CreateWindow(L"BUTTON", L"Enable Custom Mode", WS_CHILD | BS_AUTOCHECKBOX, 30, 75, 200, 20, hwnd, (HMENU)IDC_CUSTOM_MODE_CHECK, hInst, NULL);
    
    CreateWindow(L"STATIC", L"Command Preview (read-only):", WS_CHILD, 30, 110, 250, 20, hwnd, (HMENU)IDC_STATIC_PREVIEW_CMD, hInst, NULL);
    hPreviewCmd = CreateWindow(L"EDIT", L"", WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY, 30, 135, 610, 60, hwnd, (HMENU)IDC_PREVIEW_CMD_EDIT, hInst, NULL);
    hUpdatePreview = CreateWindow(L"BUTTON", L"Update Preview", WS_CHILD, 510, 105, 130, 25, hwnd, (HMENU)IDC_UPDATE_PREVIEW_BTN, hInst, NULL);

    CreateWindow(L"STATIC", L"Custom Command (editable):", WS_CHILD, 30, 210, 250, 20, hwnd, (HMENU)IDC_STATIC_CUSTOM_CMD, hInst, NULL);
    hCustomCmd = CreateWindow(L"EDIT", L"", WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, 30, 235, 610, 60, hwnd, (HMENU)IDC_CUSTOM_CMD_EDIT, hInst, NULL);
    
    // *** FIX: Added missing controls for paths ***
    CreateWindow(L"STATIC", L"QEMU Path:", WS_CHILD, 30, 310, 100, 20, hwnd, (HMENU)IDC_STATIC_QEMU_PATH, hInst, NULL);
    hQemuPath = CreateWindow(L"EDIT", L"", WS_CHILD | WS_BORDER, 140, 305, 500, 25, hwnd, (HMENU)IDC_QEMU_PATH_EDIT, hInst, NULL);
    
    // --- Main Buttons and Log ---
    hStartBtn = CreateWindow(L"BUTTON", L"Start Virtual Machine", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 10, 420, 520, 40, hwnd, (HMENU)IDC_START_BTN, hInst, NULL);
    hClearBtn = CreateWindow(L"BUTTON", L"Clear Fields", WS_CHILD | WS_VISIBLE, 540, 420, 110, 40, hwnd, (HMENU)IDC_CLEAR_BTN, hInst, NULL);
    CreateWindow(L"STATIC", L"QEMU Log:", WS_CHILD | WS_VISIBLE, 10, 470, 120, 20, hwnd, NULL, hInst, NULL);
    hLog = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY, 10, 490, 660, 150, hwnd, (HMENU)IDC_LOG_EDIT, hInst, NULL);

    // Set font for all controls and show the first tab
    EnumChildWindows(hwnd, [](HWND child, LPARAM lParam) -> BOOL {
        SendMessage(child, WM_SETFONT, (WPARAM)lParam, TRUE);
        return TRUE;
    }, (LPARAM)hFont);
    HandleTabChange(hwnd);
}

void HandleTabChange(HWND hwnd) {
    int iSel = TabCtrl_GetCurSel(hTab);

    // Show/Hide GroupBoxes
    ShowWindow(hTabBasic, iSel == 0 ? SW_SHOW : SW_HIDE);
    ShowWindow(hTabDrives, iSel == 1 ? SW_SHOW : SW_HIDE);
    ShowWindow(hTabHardware, iSel == 2 ? SW_SHOW : SW_HIDE);
    ShowWindow(hTabAdvanced, iSel == 3 ? SW_SHOW : SW_HIDE);
    ShowWindow(hTabCustom, iSel == 4 ? SW_SHOW : SW_HIDE);

    // Show/Hide Basic Tab Controls
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_VMNAME), iSel == 0 ? SW_SHOW : SW_HIDE);
    ShowWindow(hVmName, iSel == 0 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_RAM), iSel == 0 ? SW_SHOW : SW_HIDE);
    ShowWindow(hRam, iSel == 0 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_CPU_CORES), iSel == 0 ? SW_SHOW : SW_HIDE);
    ShowWindow(hCpuCores, iSel == 0 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_CPU_THREADS), iSel == 0 ? SW_SHOW : SW_HIDE);
    ShowWindow(hCpuThreads, iSel == 0 ? SW_SHOW : SW_HIDE);

    // Show/Hide Drives Tab Controls
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_DISK_A), iSel == 1 ? SW_SHOW : SW_HIDE);
    ShowWindow(hDiskA, iSel == 1 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_DISK_A_BROWSE), iSel == 1 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_DISK_B), iSel == 1 ? SW_SHOW : SW_HIDE);
    ShowWindow(hDiskB, iSel == 1 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_DISK_B_BROWSE), iSel == 1 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_DISK_C), iSel == 1 ? SW_SHOW : SW_HIDE);
    ShowWindow(hDiskC, iSel == 1 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_DISK_C_BROWSE), iSel == 1 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_FLOPPY), iSel == 1 ? SW_SHOW : SW_HIDE);
    ShowWindow(hFloppy, iSel == 1 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_FLOPPY_BROWSE), iSel == 1 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_ISO), iSel == 1 ? SW_SHOW : SW_HIDE);
    ShowWindow(hIso, iSel == 1 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_ISO_BROWSE_BTN), iSel == 1 ? SW_SHOW : SW_HIDE);
    ShowWindow(hCreateDiskBtn, iSel == 1 ? SW_SHOW : SW_HIDE);

    // Show/Hide Hardware Tab Controls
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_AUDIO), iSel == 2 ? SW_SHOW : SW_HIDE);
    ShowWindow(hAudio, iSel == 2 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_VRAM), iSel == 2 ? SW_SHOW : SW_HIDE);
    ShowWindow(hVram, iSel == 2 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_BOOT), iSel == 2 ? SW_SHOW : SW_HIDE);
    ShowWindow(hBoot, iSel == 2 ? SW_SHOW : SW_HIDE);
    // ShowWindow(hAcpi, iSel == 2 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_VNC), iSel == 2 ? SW_SHOW : SW_HIDE);
    ShowWindow(hVnc, iSel == 2 ? SW_SHOW : SW_HIDE);

    // Show/Hide Advanced Tab Controls
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_CPU_MODEL), iSel == 3 ? SW_SHOW : SW_HIDE);
    ShowWindow(hCpuModel, iSel == 3 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_MACHINE), iSel == 3 ? SW_SHOW : SW_HIDE);
    ShowWindow(hMachine, iSel == 3 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_VGA), iSel == 3 ? SW_SHOW : SW_HIDE);
    ShowWindow(hVga, iSel == 3 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_NET), iSel == 3 ? SW_SHOW : SW_HIDE);
    ShowWindow(hNet, iSel == 3 ? SW_SHOW : SW_HIDE);
    ShowWindow(hAccel, iSel == 3 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_EXTRA_FLAGS), iSel == 3 ? SW_SHOW : SW_HIDE);
    ShowWindow(hExtraFlags, iSel == 3 ? SW_SHOW : SW_HIDE);

    // Show/Hide Custom Tab Controls
    ShowWindow(hCustomMode, iSel == 4 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_PREVIEW_CMD), iSel == 4 ? SW_SHOW : SW_HIDE);
    ShowWindow(hPreviewCmd, iSel == 4 ? SW_SHOW : SW_HIDE);
    ShowWindow(hUpdatePreview, iSel == 4 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_CUSTOM_CMD), iSel == 4 ? SW_SHOW : SW_HIDE);
    ShowWindow(hCustomCmd, iSel == 4 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_QEMU_PATH), iSel == 4 ? SW_SHOW : SW_HIDE);
    ShowWindow(hQemuPath, iSel == 4 ? SW_SHOW : SW_HIDE);

    if (iSel == 4) { // If custom tab is selected, update the preview
        UpdatePreviewCommand(hwnd);
    }
}

// --- Logic Functions ---
void AppendLog(const std::wstring& text) {
    int len = GetWindowTextLength(hLog);
    SendMessage(hLog, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessage(hLog, EM_REPLACESEL, 0, (LPARAM)(text + L"\r\n").c_str());
}

void ClearAllFields() {
    SetWindowText(hVmName, L"MyWin32VM");
    SetWindowText(hRam, L"2048");
    SetWindowText(hCpuCores, L"2");
    SetWindowText(hCpuThreads, L"1");
    SetWindowText(hDiskA, L"");
    SetWindowText(hDiskB, L"");
    SetWindowText(hDiskC, L"");
    SetWindowText(hFloppy, L"");
    SetWindowText(hIso, L"");
    SendMessage(hAudio, CB_SETCURSEL, 1, 0);
    SetWindowText(hVram, L"16");
    SetWindowText(hBoot, L"d"); //kh dc phep 2 cai cung luc
    // CheckDlgButton(GetParent(hAcpi), IDC_ACPI_CHECK, BST_CHECKED);
    SetWindowText(hVnc, L"");
    SendMessage(hCpuModel, CB_SETCURSEL, 0, 0);
    SendMessage(hMachine, CB_SETCURSEL, 0, 0);
    SendMessage(hVga, CB_SETCURSEL, 0, 0);
    SendMessage(hNet, CB_SETCURSEL, 0, 0);
    CheckDlgButton(GetParent(hAccel), IDC_ACCEL_CHECK, BST_UNCHECKED);
    SetWindowText(hExtraFlags, L"");
    CheckDlgButton(GetParent(hCustomMode), IDC_CUSTOM_MODE_CHECK, BST_UNCHECKED);
    SetWindowText(hCustomCmd, L"");
    AppendLog(L"All fields cleared.");
}

void SaveSettings(HWND hwnd) {
    // This function is getting very long, but it's straightforward.
    // It just saves every control's state to the registry.
    // In a real-world app, you might save to a config file instead.
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\tienanh109\\QemuGuiManager", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        wchar_t buffer[2048]; // Increased buffer size for extra flags and custom command
        GetWindowText(hVmName, buffer, 2048); RegSetValueEx(hKey, L"VMName", 0, REG_SZ, (BYTE*)buffer, (wcslen(buffer) + 1) * sizeof(wchar_t));
        GetWindowText(hRam, buffer, 2048);    RegSetValueEx(hKey, L"RAM", 0, REG_SZ, (BYTE*)buffer, (wcslen(buffer) + 1) * sizeof(wchar_t));
        GetWindowText(hCpuCores, buffer, 2048); RegSetValueEx(hKey, L"CPUCores", 0, REG_SZ, (BYTE*)buffer, (wcslen(buffer) + 1) * sizeof(wchar_t));
        GetWindowText(hCpuThreads, buffer, 2048); RegSetValueEx(hKey, L"CPUThreads", 0, REG_SZ, (BYTE*)buffer, (wcslen(buffer) + 1) * sizeof(wchar_t));
        GetWindowText(hDiskA, buffer, 2048);   RegSetValueEx(hKey, L"DiskPathA", 0, REG_SZ, (BYTE*)buffer, (wcslen(buffer) + 1) * sizeof(wchar_t));
        GetWindowText(hDiskB, buffer, 2048);   RegSetValueEx(hKey, L"DiskPathB", 0, REG_SZ, (BYTE*)buffer, (wcslen(buffer) + 1) * sizeof(wchar_t));
        GetWindowText(hDiskC, buffer, 2048);   RegSetValueEx(hKey, L"DiskPathC", 0, REG_SZ, (BYTE*)buffer, (wcslen(buffer) + 1) * sizeof(wchar_t));
        GetWindowText(hFloppy, buffer, 2048); RegSetValueEx(hKey, L"FloppyPath", 0, REG_SZ, (BYTE*)buffer, (wcslen(buffer) + 1) * sizeof(wchar_t));
        GetWindowText(hIso, buffer, 2048);    RegSetValueEx(hKey, L"IsoPath", 0, REG_SZ, (BYTE*)buffer, (wcslen(buffer) + 1) * sizeof(wchar_t));
        GetWindowText(hVram, buffer, 2048); RegSetValueEx(hKey, L"VRAM", 0, REG_SZ, (BYTE*)buffer, (wcslen(buffer) + 1) * sizeof(wchar_t));
        GetWindowText(hBoot, buffer, 2048); RegSetValueEx(hKey, L"BootOrder", 0, REG_SZ, (BYTE*)buffer, (wcslen(buffer) + 1) * sizeof(wchar_t));
        GetWindowText(hVnc, buffer, 2048); RegSetValueEx(hKey, L"VNC", 0, REG_SZ, (BYTE*)buffer, (wcslen(buffer) + 1) * sizeof(wchar_t));
        GetWindowText(hExtraFlags, buffer, 2048); RegSetValueEx(hKey, L"ExtraFlags", 0, REG_SZ, (BYTE*)buffer, (wcslen(buffer) + 1) * sizeof(wchar_t));
        
        DWORD dwValue = SendMessage(hAudio, CB_GETCURSEL, 0, 0); RegSetValueEx(hKey, L"AudioIdx", 0, REG_DWORD, (BYTE*)&dwValue, sizeof(dwValue));
        // dwValue = IsDlgButtonChecked(hwnd, IDC_ACPI_CHECK); RegSetValueEx(hKey, L"ACPI", 0, REG_DWORD, (BYTE*)&dwValue, sizeof(dwValue));
        dwValue = SendMessage(hCpuModel, CB_GETCURSEL, 0, 0); RegSetValueEx(hKey, L"CpuModelIdx", 0, REG_DWORD, (BYTE*)&dwValue, sizeof(dwValue));
        dwValue = SendMessage(hMachine, CB_GETCURSEL, 0, 0); RegSetValueEx(hKey, L"MachineIdx", 0, REG_DWORD, (BYTE*)&dwValue, sizeof(dwValue));
        dwValue = SendMessage(hVga, CB_GETCURSEL, 0, 0); RegSetValueEx(hKey, L"VgaIdx", 0, REG_DWORD, (BYTE*)&dwValue, sizeof(dwValue));
        dwValue = SendMessage(hNet, CB_GETCURSEL, 0, 0); RegSetValueEx(hKey, L"NetIdx", 0, REG_DWORD, (BYTE*)&dwValue, sizeof(dwValue));
        dwValue = IsDlgButtonChecked(hwnd, IDC_ACCEL_CHECK); RegSetValueEx(hKey, L"Accel", 0, REG_DWORD, (BYTE*)&dwValue, sizeof(dwValue));

        GetWindowText(hQemuPath, buffer, 2048); RegSetValueEx(hKey, L"QemuPath", 0, REG_SZ, (BYTE*)buffer, (wcslen(buffer) + 1) * sizeof(wchar_t));
        RegCloseKey(hKey);
    }
}

void LoadSettings(HWND hwnd) {
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\tienanh109\\QemuGuiManager", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        wchar_t buffer[2048];
        DWORD bufferSize = sizeof(buffer);
        if (RegQueryValueEx(hKey, L"VMName", NULL, NULL, (BYTE*)buffer, &bufferSize) == ERROR_SUCCESS) SetWindowText(hVmName, buffer);
        bufferSize = sizeof(buffer);
        if (RegQueryValueEx(hKey, L"RAM", NULL, NULL, (BYTE*)buffer, &bufferSize) == ERROR_SUCCESS) SetWindowText(hRam, buffer);
        bufferSize = sizeof(buffer);
        if (RegQueryValueEx(hKey, L"CPUCores", NULL, NULL, (BYTE*)buffer, &bufferSize) == ERROR_SUCCESS) SetWindowText(hCpuCores, buffer);
        bufferSize = sizeof(buffer);
        if (RegQueryValueEx(hKey, L"CPUThreads", NULL, NULL, (BYTE*)buffer, &bufferSize) == ERROR_SUCCESS) SetWindowText(hCpuThreads, buffer);
        bufferSize = sizeof(buffer);
        if (RegQueryValueEx(hKey, L"DiskPathA", NULL, NULL, (BYTE*)buffer, &bufferSize) == ERROR_SUCCESS) SetWindowText(hDiskA, buffer);
        bufferSize = sizeof(buffer);
        if (RegQueryValueEx(hKey, L"DiskPathB", NULL, NULL, (BYTE*)buffer, &bufferSize) == ERROR_SUCCESS) SetWindowText(hDiskB, buffer);
        bufferSize = sizeof(buffer);
        if (RegQueryValueEx(hKey, L"DiskPathC", NULL, NULL, (BYTE*)buffer, &bufferSize) == ERROR_SUCCESS) SetWindowText(hDiskC, buffer);
        bufferSize = sizeof(buffer);
        if (RegQueryValueEx(hKey, L"FloppyPath", NULL, NULL, (BYTE*)buffer, &bufferSize) == ERROR_SUCCESS) SetWindowText(hFloppy, buffer);
        bufferSize = sizeof(buffer);
        if (RegQueryValueEx(hKey, L"IsoPath", NULL, NULL, (BYTE*)buffer, &bufferSize) == ERROR_SUCCESS) SetWindowText(hIso, buffer);
        bufferSize = sizeof(buffer);
        if (RegQueryValueEx(hKey, L"VRAM", NULL, NULL, (BYTE*)buffer, &bufferSize) == ERROR_SUCCESS) SetWindowText(hVram, buffer);
        bufferSize = sizeof(buffer);
        if (RegQueryValueEx(hKey, L"BootOrder", NULL, NULL, (BYTE*)buffer, &bufferSize) == ERROR_SUCCESS) SetWindowText(hBoot, buffer);
        bufferSize = sizeof(buffer);
        if (RegQueryValueEx(hKey, L"VNC", NULL, NULL, (BYTE*)buffer, &bufferSize) == ERROR_SUCCESS) SetWindowText(hVnc, buffer);
        bufferSize = sizeof(buffer);
        if (RegQueryValueEx(hKey, L"ExtraFlags", NULL, NULL, (BYTE*)buffer, &bufferSize) == ERROR_SUCCESS) SetWindowText(hExtraFlags, buffer);
        
        DWORD dwValue;
        DWORD dwSize = sizeof(dwValue);
        if (RegQueryValueEx(hKey, L"AudioIdx", NULL, NULL, (BYTE*)&dwValue, &dwSize) == ERROR_SUCCESS) SendMessage(hAudio, CB_SETCURSEL, dwValue, 0);
        dwSize = sizeof(dwValue);
        // if (RegQueryValueEx(hKey, L"ACPI", NULL, NULL, (BYTE*)&dwValue, &dwSize) == ERROR_SUCCESS) CheckDlgButton(hwnd, IDC_ACPI_CHECK, dwValue);
        // dwSize = sizeof(dwValue);
        if (RegQueryValueEx(hKey, L"CpuModelIdx", NULL, NULL, (BYTE*)&dwValue, &dwSize) == ERROR_SUCCESS) SendMessage(hCpuModel, CB_SETCURSEL, dwValue, 0);
        dwSize = sizeof(dwValue);
        if (RegQueryValueEx(hKey, L"MachineIdx", NULL, NULL, (BYTE*)&dwValue, &dwSize) == ERROR_SUCCESS) SendMessage(hMachine, CB_SETCURSEL, dwValue, 0);
        dwSize = sizeof(dwValue);
        if (RegQueryValueEx(hKey, L"VgaIdx", NULL, NULL, (BYTE*)&dwValue, &dwSize) == ERROR_SUCCESS) SendMessage(hVga, CB_SETCURSEL, dwValue, 0);
        dwSize = sizeof(dwValue);
        if (RegQueryValueEx(hKey, L"NetIdx", NULL, NULL, (BYTE*)&dwValue, &dwSize) == ERROR_SUCCESS) SendMessage(hNet, CB_SETCURSEL, dwValue, 0);
        dwSize = sizeof(dwValue);
        if (RegQueryValueEx(hKey, L"Accel", NULL, NULL, (BYTE*)&dwValue, &dwSize) == ERROR_SUCCESS) CheckDlgButton(hwnd, IDC_ACCEL_CHECK, dwValue);

        bufferSize = sizeof(buffer);
        if (RegQueryValueEx(hKey, L"QemuPath", NULL, NULL, (BYTE*)buffer, &bufferSize) == ERROR_SUCCESS) {
            SetWindowText(hQemuPath, buffer);
        }
        RegCloseKey(hKey);
    }
}

std::wstring SearchForQemuInProgramFiles() {
    wchar_t pfPath[MAX_PATH];
    wchar_t pfx86Path[MAX_PATH];

    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 0, pfPath))) {
        std::wstring fullPath = std::wstring(pfPath) + L"\\qemu\\qemu-system-x86_64.exe";
        if (PathFileExists(fullPath.c_str())) return fullPath;
    }
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILESX86, NULL, 0, pfx86Path))) {
        std::wstring fullPath = std::wstring(pfx86Path) + L"\\qemu\\qemu-system-x86_64.exe";
        if (PathFileExists(fullPath.c_str())) return fullPath;
    }
    return L"";
}

void FindQemuPath(HWND hwnd) {
    wchar_t currentPath[MAX_PATH];
    GetWindowText(hQemuPath, currentPath, MAX_PATH);
    if (wcslen(currentPath) > 0 && PathFileExists(currentPath)) {
        AppendLog(L"Using QEMU path from settings: " + std::wstring(currentPath));
        return;
    }

    wchar_t foundPathBuffer[MAX_PATH] = L"qemu-system-x86_64.exe";
    if (PathFindOnPath(foundPathBuffer, NULL)) {
        AppendLog(L"Found QEMU in system PATH: " + std::wstring(foundPathBuffer));
        SetWindowText(hQemuPath, foundPathBuffer);
        return;
    }
    std::wstring pfPath = SearchForQemuInProgramFiles();
    if (!pfPath.empty()) {
        AppendLog(L"Found QEMU in Program Files: " + pfPath);
        SetWindowText(hQemuPath, pfPath.c_str());
        return;
    }
    AppendLog(L"Could not automatically find QEMU.");
    if (MessageBox(hwnd, L"Could not find 'qemu-system-x86_64.exe' automatically.\nWould you like to specify the path manually?", L"QEMU Not Found", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        std::wstring manualPath = BrowseForFile(hwnd, L"QEMU Executable (*.exe)\0*.exe\0All Files (*.*)\0*.*\0", L"Select QEMU Executable", true);
        if (!manualPath.empty()) {
            AppendLog(L"Set QEMU path to: " + manualPath);
            SetWindowText(hQemuPath, manualPath.c_str());
        }
    }
}

std::wstring BrowseForFile(HWND hwnd, const wchar_t* filter, const wchar_t* title, bool mustExist) {
    wchar_t filename[MAX_PATH] = { 0 };
    OPENFILENAME ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = title;
    ofn.Flags = OFN_DONTADDTORECENT | OFN_PATHMUSTEXIST;
    if(mustExist) ofn.Flags |= OFN_FILEMUSTEXIST;

    if (!mustExist && GetSaveFileName(&ofn)) return filename;
    if (mustExist && GetOpenFileName(&ofn)) return filename;
    
    return L"";
}

std::wstring BuildQemuCommand(HWND hwnd) {
    wchar_t buffer[4096]; // Increased buffer size for safety
    std::wstringstream cmd;
    
    GetWindowText(hQemuPath, buffer, MAX_PATH);
    cmd << L"\"" << buffer << L"\"";

    // Basic
    GetWindowText(hRam, buffer, 4096); cmd << L" -m " << buffer;
    
    wchar_t cores[16], threads[16];
    GetWindowText(hCpuCores, cores, 16);
    GetWindowText(hCpuThreads, threads, 16);
    cmd << L" -smp " << cores << L",threads=" << threads;

    // Advanced
    if (IsDlgButtonChecked(hwnd, IDC_ACCEL_CHECK)) cmd << L" -accel whpx";
    
    GetWindowText(hMachine, buffer, 4096);
    if(wcscmp(buffer, L"pc (i440fx, default)") != 0) cmd << L" -machine " << buffer;
    
    GetWindowText(hCpuModel, buffer, 4096);
    if(wcscmp(buffer, L"max (best compatibility)") != 0) {
        std::wstring cpuModelStr = buffer;
        size_t first_space = cpuModelStr.find(L" ");
        if (first_space != std::wstring::npos) cpuModelStr = cpuModelStr.substr(0, first_space);
        cmd << L" -cpu " << cpuModelStr;
    } else {
        cmd << L" -cpu max";
    }

    GetWindowText(hVga, buffer, 4096);
    std::wstring vgaCard = buffer;
    cmd << L" -vga " << vgaCard;

    // Hardware
    GetWindowText(hVram, buffer, 4096);
    if (vgaCard == L"qxl" || vgaCard == L"virtio") { // VRAM setting primarily for these cards
        cmd << L" -global " << vgaCard << L"-vga.vram_size=" << std::stoi(buffer) * 1024 * 1024;
    }

    GetWindowText(hAudio, buffer, 4096);
    if(wcscmp(buffer, L"Disabled") != 0) {
        cmd << L" -audiodev dsound,id=snd0 -machine pcspk-audiodev=snd0";
    }

    // if (!IsDlgButtonChecked(hwnd, IDC_ACPI_CHECK)) cmd << L" -no-acpi";

    GetWindowText(hVnc, buffer, 4096);
    if (wcslen(buffer) > 0) cmd << L" -vnc " << buffer;
    
    // Drives
    GetWindowText(hDiskA, buffer, 4096); if (wcslen(buffer) > 0) cmd << L" -hda \"" << buffer << L"\"";
    GetWindowText(hDiskB, buffer, 4096); if (wcslen(buffer) > 0) cmd << L" -hdb \"" << buffer << L"\"";
    GetWindowText(hDiskC, buffer, 4096); if (wcslen(buffer) > 0) cmd << L" -hdc \"" << buffer << L"\"";
    GetWindowText(hFloppy, buffer, 4096); if (wcslen(buffer) > 0) cmd << L" -fda \"" << buffer << L"\"";

    GetWindowText(hIso, buffer, 4096);
    if (wcslen(buffer) > 0) cmd << L" -cdrom \"" << buffer << L"\"";

    GetWindowText(hBoot, buffer, 4096);
    if (wcslen(buffer) > 0) cmd << L" -boot order=" << buffer;

    // Network
    GetWindowText(hNet, buffer, 4096);
    cmd << L" -netdev user,id=net0,hostfwd=tcp::10022-:22 -device " << buffer << L",netdev=net0";

    // Extra Flags
    GetWindowText(hExtraFlags, buffer, 4096);
    if (wcslen(buffer) > 0) cmd << L" " << buffer;

    return cmd.str();
}

void UpdatePreviewCommand(HWND hwnd) {
    std::wstring command = BuildQemuCommand(hwnd);
    SetWindowText(hPreviewCmd, command.c_str());
    SetWindowText(hCustomCmd, command.c_str());
}

void StartVm(HWND hwnd) {
    std::wstring finalCommand;
    if (IsDlgButtonChecked(hwnd, IDC_CUSTOM_MODE_CHECK)) {
        AppendLog(L"Using custom command...");
        int len = GetWindowTextLength(hCustomCmd);
        std::vector<wchar_t> buf(len + 1);
        GetWindowText(hCustomCmd, &buf[0], len + 1);
        finalCommand = &buf[0];
    } else {
        finalCommand = BuildQemuCommand(hwnd);
    }
    
    if (finalCommand.empty()) {
        MessageBox(hwnd, L"Could not generate a command to execute.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    AppendLog(L"Command to be executed:");
    AppendLog(finalCommand);
    AppendLog(L"------------------------------------");
    AppendLog(L"Starting virtual machine...");

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    
    if (!CreateProcess(NULL, &finalCommand[0], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
        AppendLog(L"Error: Could not start QEMU. Error code: " + std::to_wstring(GetLastError()));
    } else {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        AppendLog(L"Sent command to start QEMU in a new console window.");
    }
}

// --- Create Disk Dialog ---
void ShowCreateDiskDialog(HWND hwndParent) {
    DialogBox(hInst, MAKEINTRESOURCE(IDD_CREATEDISK), hwndParent, CreateDiskDlgProc);
}

INT_PTR CALLBACK CreateDiskDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static HWND hPath, hFormat, hSize;
    switch (message) {
    case WM_INITDIALOG: {
        hPath = GetDlgItem(hDlg, IDC_CD_PATH_EDIT);
        hFormat = GetDlgItem(hDlg, IDC_CD_FORMAT_COMBO);
        hSize = GetDlgItem(hDlg, IDC_CD_SIZE_EDIT);
        
        SendMessage(hFormat, CB_ADDSTRING, 0, (LPARAM)L"qcow2");
        SendMessage(hFormat, CB_ADDSTRING, 0, (LPARAM)L"raw");
        SendMessage(hFormat, CB_SETCURSEL, 0, 0);
        SetWindowText(hSize, L"10"); // Default 10 GB
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_CD_PATH_BROWSE: {
            std::wstring path = BrowseForFile(hDlg, L"Disk Images (*.qcow2;*.img)\0*.qcow2;*.img\0All Files (*.*)\0*.*\0", L"Save New Disk As...", false);
            if (!path.empty()) SetWindowText(hPath, path.c_str());
            break;
        }
        case IDC_CD_CREATE: {
            wchar_t pathBuf[MAX_PATH], formatBuf[20], sizeBuf[20], qemuPathBuf[MAX_PATH];
            GetWindowText(hPath, pathBuf, MAX_PATH);
            GetWindowText(hFormat, formatBuf, 20);
            GetWindowText(hSize, sizeBuf, 20);
            GetWindowText(hQemuPath, qemuPathBuf, MAX_PATH); // Get path from main window's custom tab

            if (wcslen(pathBuf) == 0) {
                MessageBox(hDlg, L"Please specify a path to save the disk image.", L"Error", MB_OK);
                break;
            }
            std::wstring qemuImgPath = qemuPathBuf;
            size_t pos = qemuImgPath.rfind(L"\\");
            if(pos != std::wstring::npos) {
                qemuImgPath = qemuImgPath.substr(0, pos) + L"\\qemu-img.exe";
            } else {
                 MessageBox(hDlg, L"Could not determine path for qemu-img.exe from main QEMU path.", L"Error", MB_OK);
                 break;
            }
            
            if(!PathFileExists(qemuImgPath.c_str())) {
                MessageBox(hDlg, (L"qemu-img.exe not found at:\n" + qemuImgPath).c_str(), L"Error", MB_OK);
                break;
            }

            std::wstringstream cmd;
            cmd << L"\"" << qemuImgPath << L"\" create -f " << formatBuf << L" \"" << pathBuf << L"\" " << sizeBuf << L"G";
            
            AppendLog(L"Executing: " + cmd.str());

            STARTUPINFO si = { sizeof(si) };
            PROCESS_INFORMATION pi;
            si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_HIDE;

            if (CreateProcess(NULL, &cmd.str()[0], NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
                WaitForSingleObject(pi.hProcess, INFINITE);
                DWORD exitCode;
                GetExitCodeProcess(pi.hProcess, &exitCode);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                if(exitCode == 0) {
                    MessageBox(hDlg, L"Disk image created successfully!", L"Success", MB_OK);
                    SetWindowText(hDiskA, pathBuf);
                    EndDialog(hDlg, LOWORD(wParam));
                } else {
                    MessageBox(hDlg, (L"Failed to create disk image. qemu-img exited with code: " + std::to_wstring(exitCode)).c_str(), L"Error", MB_OK);
                }
            } else {
                 MessageBox(hDlg, L"Failed to launch qemu-img.exe.", L"Error", MB_OK);
            }
            break;
        }
        case IDC_CD_CANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            break;
        }
        return (INT_PTR)TRUE;
    }
    return (INT_PTR)FALSE;
}
