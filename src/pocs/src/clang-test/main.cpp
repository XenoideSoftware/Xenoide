#define UNICODE
#define _UNICODE

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>

#include <clang-c/Index.h>

#include <cctype>
#include <string>

#pragma comment(lib, "comctl32.lib")

// ─── IDs ────────────────────────────────────────────────────────────────────

#define IDM_FILE_OPEN 100
#define IDM_FILE_EXIT 101
#define IDC_TREEVIEW 200

// ─── Globals ─────────────────────────────────────────────────────────────────

static HWND g_hWnd = nullptr;
static HWND g_hTree = nullptr;
static CXIndex g_index = nullptr;
static CXTranslationUnit g_tu = nullptr;

// ─── Helpers ─────────────────────────────────────────────────────────────────

// Consume a CXString and return it as a wide string.
static std::wstring ToWide(CXString cxs) {
    const char *p = clang_getCString(cxs);
    std::wstring result;
    if (p && *p) {
        int n = MultiByteToWideChar(CP_UTF8, 0, p, -1, nullptr, 0);
        result.resize(n - 1);
        MultiByteToWideChar(CP_UTF8, 0, p, -1, result.data(), n);
    }
    clang_disposeString(cxs);
    return result;
}

// Normalise a file path: forward-slashes, lowercase.
static std::string NormPath(const char *p) {
    if (!p)
        return {};
    std::string s = p;
    for (char &c : s) {
        if (c == '\\')
            c = '/';
        else
            c = (char)tolower((unsigned char)c);
    }
    return s;
}

// Return a short kind tag, or nullptr to skip (and recurse transparently).
static const wchar_t *KindTag(CXCursorKind k) {
    switch (k) {
    case CXCursor_Namespace:
        return L"[namespace]";
    case CXCursor_ClassDecl:
        return L"[class]";
    case CXCursor_StructDecl:
        return L"[struct]";
    case CXCursor_UnionDecl:
        return L"[union]";
    case CXCursor_EnumDecl:
        return L"[enum]";
    case CXCursor_EnumConstantDecl:
        return L"[const]";
    case CXCursor_FunctionDecl:
        return L"[function]";
    case CXCursor_CXXMethod:
        return L"[method]";
    case CXCursor_Constructor:
        return L"[ctor]";
    case CXCursor_Destructor:
        return L"[dtor]";
    case CXCursor_ConversionFunction:
        return L"[conv]";
    case CXCursor_FieldDecl:
        return L"[field]";
    case CXCursor_VarDecl:
        return L"[var]";
    case CXCursor_TypedefDecl:
        return L"[typedef]";
    case CXCursor_TypeAliasDecl:
        return L"[using]";
    case CXCursor_FunctionTemplate:
        return L"[fn tmpl]";
    case CXCursor_ClassTemplate:
        return L"[class tmpl]";
    case CXCursor_ClassTemplatePartialSpecialization:
        return L"[partial spec]";
    default:
        return nullptr;
    }
}

// ─── AST visitor ─────────────────────────────────────────────────────────────

struct VisitCtx {
    HWND hTree;
    HTREEITEM parent;
    std::string normFile; // normalised path of the opened file
};

static CXChildVisitResult Visit(CXCursor cursor, CXCursor /*parent*/, CXClientData raw) {
    auto *ctx = static_cast<VisitCtx *>(raw);

    // Skip nodes that don't belong to the opened file.
    CXSourceLocation loc = clang_getCursorLocation(cursor);
    CXFile cxf = nullptr;
    clang_getFileLocation(loc, &cxf, nullptr, nullptr, nullptr);
    if (!cxf)
        return CXChildVisit_Continue;

    CXString cxName = clang_getFileName(cxf);
    std::string nodeFile = NormPath(clang_getCString(cxName));
    clang_disposeString(cxName);

    if (nodeFile != ctx->normFile)
        return CXChildVisit_Continue;

    CXCursorKind kind = clang_getCursorKind(cursor);
    const wchar_t *tag = KindTag(kind);

    if (!tag) {
        // Transparent container (e.g. extern "C" linkage spec, access specifier).
        // Keep the same parent so its children are inserted at the same level.
        clang_visitChildren(cursor, Visit, ctx);
        return CXChildVisit_Continue;
    }

    // Build label text: "[kind] spelling"  (or "[kind] <anonymous>" for unnamed types)
    std::wstring spelling = ToWide(clang_getCursorSpelling(cursor));
    if (spelling.empty())
        spelling = L"<anonymous>";
    std::wstring text = std::wstring(tag) + L" " + spelling;

    // Insert a tree item under the current parent.
    TVINSERTSTRUCTW ins = {};
    ins.hParent = ctx->parent;
    ins.hInsertAfter = TVI_LAST;
    ins.item.mask = TVIF_TEXT;
    ins.item.pszText = text.data();
    HTREEITEM hItem = TreeView_InsertItem(ctx->hTree, &ins);

    // Recurse into the node's children using the new item as parent.
    VisitCtx child{ctx->hTree, hItem, ctx->normFile};
    clang_visitChildren(cursor, Visit, &child);

    return CXChildVisit_Continue;
}

// ─── File parsing ─────────────────────────────────────────────────────────────

static void ParseFile(const wchar_t *wpath) {
    // Convert path to UTF-8.
    int n = WideCharToMultiByte(CP_UTF8, 0, wpath, -1, nullptr, 0, nullptr, nullptr);
    std::string path(n - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wpath, -1, path.data(), n, nullptr, nullptr);

    // Dispose any previous translation unit.
    if (g_tu) {
        clang_disposeTranslationUnit(g_tu);
        g_tu = nullptr;
    }

    TreeView_DeleteAllItems(g_hTree);

    const char *clangArgs[] = {"-std=c++20", "-x", "c++"};
    g_tu = clang_parseTranslationUnit(g_index, path.c_str(), clangArgs, 3, nullptr, 0, CXTranslationUnit_SkipFunctionBodies);

    if (!g_tu) {
        MessageBoxW(g_hWnd, L"libclang could not parse the file.", L"Parse Error", MB_ICONERROR);
        return;
    }

    // Update window title.
    std::wstring title = std::wstring(L"Clang AST Viewer  \u2014  ") + wpath;
    SetWindowTextW(g_hWnd, title.c_str());

    // Root tree item — the file name itself.
    TVINSERTSTRUCTW ins = {};
    ins.hParent = TVI_ROOT;
    ins.hInsertAfter = TVI_LAST;
    ins.item.mask = TVIF_TEXT;
    ins.item.pszText = const_cast<wchar_t *>(wpath);
    HTREEITEM hRoot = TreeView_InsertItem(g_hTree, &ins);

    // Walk the AST.
    VisitCtx ctx{g_hTree, hRoot, NormPath(path.c_str())};
    clang_visitChildren(clang_getTranslationUnitCursor(g_tu), Visit, &ctx);

    TreeView_Expand(g_hTree, hRoot, TVE_EXPAND);
}

// ─── Menu ────────────────────────────────────────────────────────────────────

static HMENU BuildMenuBar() {
    HMENU hBar = CreateMenu();
    HMENU hFile = CreatePopupMenu();
    AppendMenuW(hFile, MF_STRING, IDM_FILE_OPEN, L"&Open...\tCtrl+O");
    AppendMenuW(hFile, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hFile, MF_STRING, IDM_FILE_EXIT, L"E&xit\tAlt+F4");
    AppendMenuW(hBar, MF_POPUP, (UINT_PTR)hFile, L"&File");
    return hBar;
}

// ─── Window procedure ─────────────────────────────────────────────────────────

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_CREATE: {
        SetMenu(hWnd, BuildMenuBar());

        g_hTree = CreateWindowExW(
            0,
            WC_TREEVIEWW,
            L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS,
            0,
            0,
            0,
            0,
            hWnd,
            (HMENU)IDC_TREEVIEW,
            (HINSTANCE)GetWindowLongPtrW(hWnd, GWLP_HINSTANCE),
            nullptr
        );

        // Use a slightly larger font for readability.
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SendMessageW(g_hTree, WM_SETFONT, (WPARAM)hFont, TRUE);
        return 0;
    }

    case WM_SIZE: {
        RECT rc;
        GetClientRect(hWnd, &rc);
        SetWindowPos(g_hTree, nullptr, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER);
        return 0;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_FILE_OPEN: {
            wchar_t buf[MAX_PATH] = {};
            OPENFILENAMEW ofn = {};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = buf;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = L"C/C++ Source Files\0*.cpp;*.cxx;*.cc;*.c;*.h;*.hpp;*.hxx\0"
                              L"All Files\0*.*\0";
            ofn.lpstrTitle = L"Open C++ Source File";
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
            if (GetOpenFileNameW(&ofn))
                ParseFile(buf);
            break;
        }
        case IDM_FILE_EXIT:
            PostQuitMessage(0);
            break;
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

// ─── Entry point ─────────────────────────────────────────────────────────────

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    INITCOMMONCONTROLSEX icc = {sizeof(icc), ICC_TREEVIEW_CLASSES};
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"ClangAstViewer";
    wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    RegisterClassExW(&wc);

    g_hWnd = CreateWindowExW(0, L"ClangAstViewer", L"Clang AST Viewer", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 900, 650, nullptr, nullptr, hInst, nullptr);

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    g_index = clang_createIndex(/*excludePCH=*/0, /*displayDiag=*/0);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (g_tu)
        clang_disposeTranslationUnit(g_tu);
    if (g_index)
        clang_disposeIndex(g_index);

    return (int)msg.wParam;
}
