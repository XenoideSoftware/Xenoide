#include <Scintilla.h>

#include <cstdio>

int main() {
    std::printf("scintilla3 test_package: INVALID_POSITION=%d, SCI_GETTEXT=%d\n",
                static_cast<int>(INVALID_POSITION),
                static_cast<int>(SCI_GETTEXT));
    return 0;
}
