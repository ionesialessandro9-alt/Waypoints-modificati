
#include <windows.h>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

static HWND gWnd = nullptr;

static std::wstring gText =
    L"ALEX WAYPOINTS\nConnessione a Dawn Client...";

std::wstring FindLiveFile() {
    wchar_t appdata[MAX_PATH]{};

    DWORD len = GetEnvironmentVariableW(
        L"APPDATA", appdata, MAX_PATH
    );

    if (len == 0 || len >= MAX_PATH)
        return L"";

    fs::path profiles =
        fs::path(appdata) / L".dawn" / L"profiles";

    std::error_code ec;

    if (!fs::exists(profiles, ec))
        return L"";

    fs::recursive_directory_iterator it(
        profiles,
        fs::directory_options::skip_permission_denied,
        ec
    );

    fs::recursive_directory_iterator end;

    while (it != end) {
        if (!ec && it->is_regular_file(ec)) {
            if (it->path().filename() ==
                L"alex-waypoints-live.json") {
                return it->path().wstring();
            }
        }

        it.increment(ec);

        if (ec)
            ec.clear();
    }

    return L"";
}

std::string ReadFileUtf8() {
    std::wstring path = FindLiveFile();

    if (path.empty())
        return {};

    std::ifstream file(path, std::ios::binary);

    if (!file)
        return {};

    std::ostringstream content;
    content << file.rdbuf();

    return content.str();
}

std::wstring Utf8ToWide(const std::string& s) {
    if (s.empty())
        return L"";

    int n = MultiByteToWideChar(
        CP_UTF8, 0, s.c_str(),
        (int)s.size(), nullptr, 0
    );

    if (n <= 0)
        return L"";

    std::wstring out(n, L'\0');

    MultiByteToWideChar(
        CP_UTF8, 0, s.c_str(),
        (int)s.size(), out.data(), n
    );

    return out;
}

std::wstring GetString(
    const std::string& s,
    const std::string& key
) {
    std::regex re(
        "\"" + key + "\"\\s*:\\s*\"([^\"]*)\""
    );

    std::smatch match;

    if (std::regex_search(s, match, re))
        return Utf8ToWide(match[1].str());

    return L"-";
}

double GetNumber(
    const std::string& s,
    const std::string& key
) {
    std::regex re(
        "\"" + key +
        "\"\\s*:\\s*(-?[0-9]+(?:\\.[0-9]+)?)"
    );

    std::smatch match;

    if (std::regex_search(s, match, re)) {
        try {
            return std::stod(match[1].str());
        }
        catch (...) {}
    }

    return 0;
}

void Refresh() {
    std::string data = ReadFileUtf8();

    if (data.empty()) {
        gText =
            L"CONNESSIONE A DAWN\n\n"
            L"File waypoint non trovato.\n"
            L"Avvia Minecraft con ALEX WAYPOINTS.";
    }
    else {
        std::wstring name = GetString(data, "name");

        double x = GetNumber(data, "x");
        double y = GetNumber(data, "y");
        double z = GetNumber(data, "z");
        double distance = GetNumber(data, "distance");

        std::wostringstream out;

        out << L"WAYPOINT ATTIVO\n\n";
        out << L"Nome: " << name << L"\n\n";

        out << L"COORDINATE\n";
        out << L"X: " << (int)x
            << L"   Y: " << (int)y
            << L"   Z: " << (int)z << L"\n\n";

        out << L"DISTANZA\n";
        out << std::fixed << std::setprecision(1)
            << distance << L" blocchi\n\n";

        out << L"DAWN CLIENT CONNECTED\n";
        out << L"ESC per chiudere";

        gText = out.str();
    }

    if (gWnd)
        InvalidateRect(gWnd, nullptr, TRUE);
}

LRESULT CALLBACK Proc(
    HWND h,
    UINT message,
    WPARAM w,
    LPARAM l
) {
    switch (message) {

    case WM_CREATE:
        SetTimer(h, 1, 1000, nullptr);
        return 0;

    case WM_TIMER:
        Refresh();
        return 0;

    case WM_LBUTTONDOWN:
        ReleaseCapture();
        SendMessageW(
            h, WM_NCLBUTTONDOWN, HTCAPTION, 0
        );
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

        HBRUSH bg = CreateSolidBrush(
            RGB(17, 17, 22)
        );
        FillRect(dc, &r, bg);
        DeleteObject(bg);

        RECT accent{ 0, 0, 5, r.bottom };

        HBRUSH red = CreateSolidBrush(
            RGB(230, 35, 55)
        );
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

        RECT titleRect{
            18, 12, r.right - 12, 48
        };

        DrawTextW(
            dc,
            L"ALEX / PRIVATE WAYPOINTS",
            -1,
            &titleRect,
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

        RECT bodyRect{
            18, 58, r.right - 14, r.bottom - 10
        };

        DrawTextW(
            dc,
            gText.c_str(),
            -1,
            &bodyRect,
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

    return DefWindowProcW(h, message, w, l);
}

int WINAPI wWinMain(
    HINSTANCE inst,
    HINSTANCE,
    PWSTR,
    int show
) {
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
