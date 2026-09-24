#include <windows.h>
#include <propertyGrid.h>

int main(void) {
    /* Force a real reference to the static lib without creating a window. */
    HWND (*p)(HWND, DWORD) = New_PropertyGrid;
    (void)p;
    return 0;
}
