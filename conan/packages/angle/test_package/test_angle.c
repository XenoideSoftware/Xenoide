#include <EGL/egl.h>
#include <GLES2/gl2.h>

int main(void) {
    /* Verify EGL and GLES2 symbols are available (link-time check) */
    (void)eglGetDisplay;
    (void)eglInitialize;
    (void)glClear;
    (void)glGetString;
    return 0;
}
