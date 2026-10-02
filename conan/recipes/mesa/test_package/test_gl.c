#include <GL/osmesa.h>

int main(void) {
    /* Verify OSMesa symbols are available (link-time check) */
    (void)OSMesaCreateContext;
    (void)OSMesaMakeCurrent;
    (void)OSMesaDestroyContext;
    return 0;
}
