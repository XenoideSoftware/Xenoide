#include <windows.h>
#include <mCtrl/button.h>

int main(void) {
    /* Force a real reference to the import lib without spinning up a window loop.
     * A void* cast sidesteps MCTRL_API's __stdcall calling convention. */
    void *p = (void*)&mcButton_Initialize;
    (void)p;
    return 0;
}
