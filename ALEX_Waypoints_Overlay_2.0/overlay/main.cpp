
#define UNICODE
#define _UNICODE

#include <windows.h>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>
#include <iomanip>

static HWND gWnd = nullptr;

static std::wstring gText =
    L"ALEX WAYPOINTS\n"
    L"Connessione a Dawn Client...";

static const wchar_t* LIVE =
LR"(C:\Users\Alessandro Ionesi\AppData\Roaming\.dawn\profiles\feather-default--1-21-8-copy\.minecraft\config\alex-waypoints-live.json)";

std::wstring Utf8ToWide(const std::string& s) {
    if (s.empty()) return L"";

    int n = MultiByteToWideChar(
        CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0
    );

    std::wstring out(n, L'\0');

    MultiByteToWideChar(
        CP_UTF8, 0, s.c_str(), (int)s.size(), out.data(), n
    );

    return out;
}

std::string ReadFileUtf8() {
    std::ifstream f(LIVE, std::ios::binary);

    if (!f) return {};

    std::ostringstream ss;
    ss << f.rdbuf();

    return ss.str();
}

std::wstring GetString(const std::string& s, const std::string& key) {
    std::regex re("\"" + key + "\"\\s*:\\s*\"([^\"]*)\"");
    std::smatch m;

    if (std::regex_search(s, m, re))
        return Utf8ToWide(m[1].str());

    return L"-";
}

double GetNumber(const std::string& s, const std::string& key) {
    std::regex re(
        "\"" + key + "\"\\s*:\\s*(-?[0-9]+(?:\\.[0-9]+)?)"
    );

    std::smatch m;

    if (std::regex_search(s, m, re)) {
        try {
            return std::stod(m[1].str());
        }
        catch (...) {}
    }

    return 0;
}

void Refresh() {
    std::string s = ReadFileUtf8();

    if (s.empty()) {
        gText =
            L"CONNESSIONE A DAWN\n\n"
            L"File waypoint non trovato.\n"
            L"Controlla che Minecraft sia aperto.";
    }
    else {
        auto name = GetString(s, "name");

        double x = GetNumber(s, "x");
        double y = GetNumber(s, "y");
        double z = GetNumber(s, "z");
        double d = GetNumber(s, "distance");

        std::wostringstream out;

        out << L"WAYPOINT ATTIVO\n\n";
        out << L"Nome: " << name << L"\n\n";

        out << L"COORDINATE\n";
        out << L"X: " << (int)x
            << L"   Y: " << (int)y
            << L"   Z: " << (int)z << L"\n\n";

        out << L"DISTANZA\n";
        out << std::fixed << std::setprecision(1)
            << d << L" blocchi\n\n";

        out << L"DAWN CLIENT CONNECTED\n";
        out << L"ESC per chiudere";

        gText = out.str();
    }

    if (gWnd)
        InvalidateRect(gWnd, nullptr, TRUE);
}

LRESULT CALLBACK Proc(HWND h, UINT m, WPARAM w, LPARAM l) {
    switch (m) {

    case WM_CREATE:
        SetTimer(h, 1, 500, nullptr);
        return 0;

    case WM_TIMER:
        Refresh();
        return 0;

    case WM_LBUTTONDOWN:
        ReleaseCapture();
        SendMessageW(h, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        return 0;

    case WM_KEYDOWN:
        if (w == VK_ESCAPE)
            DestroyWindow(h);
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(h, &ps);

        RECT r;
        GetClientRect(h, &r);

        HBRUSH bg = CreateSolidBrush(RGB(17, 17, 22));
        FillRect(dc, &r, bg);
        DeleteObject(bg);

        RECT accent{ 0, 0, 5, r.bottom };
        HBRUSH red = CreateSolidBrush(RGB(230, 35, 55));
        FillRect(dc, &accent, red);
        DeleteObject(red);

        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(255, 65, 80));

        HFONT title = CreateFontW(
            22, 0, 0, 0, FW_BOLD,
            FALSE, FALSE, FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY,
            DEFAULT_PITCH,
            L"Segoe UI"
        );

        HFONT old = (HFONT)SelectObject(dc, title);

        RECT t{ 18, 12, r.right - 12, 48 };

        DrawTextW(
            dc,
            L"ALEX / PRIVATE WAYPOINTS",
            -1,
            &t,
            DT_LEFT | DT_SINGLELINE | DT_VCENTER
        );

        SelectObject(dc, old);
        DeleteObject(title);

        SetTextColor(dc, RGB(240, 240, 245));

        HFONT body = CreateFontW(
            17, 0, 0, 0, FW_NORMAL,
            FALSE, FALSE, FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY,
            DEFAULT_PITCH,
            L"Segoe UI"
        );

        old = (HFONT)SelectObject(dc, body);

        RECT b{ 18, 58, r.right - 14, r.bottom - 10 };

        DrawTextW(
            dc,
            gText.c_str(),
            -1,
            &b,
            DT_LEFT | DT_TOP | DT_WORDBREAK
        );

        SelectObject(dc, old);
        DeleteObject(body);

        EndPaint(h, &ps);
        return 0;
    }

    case WM_DESTROY:
        KillTimer(h, 1);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(h, m, w, l);
}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE, PWSTR, int show) {
    WNDCLASSW wc{};

    wc.lpfnWndProc = Proc;
    wc.hInstance = inst;
    wc.lpszClassName = L"ALEX_WP_OVERLAY";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClassW(&wc);

    gWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        wc.lpszClassName,
        L"ALEX Waypoints",
        WS_POPUP | WS_VISIBLE,
        80, 100, 360, 260,
        nullptr, nullptr, inst, nullptr
    );

    if (!gWnd)
        return 1;

    ShowWindow(gWnd, show);
    Refresh();

    MSG msg;

    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}
