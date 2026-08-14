#include "wine_compat.h"

#include "windows.h"

namespace sunrise::client::graphics {

// Helper function to let wine know that we will be using the display
void initialize_wine_display() {
    HDC hdc = GetDC(NULL);
    if (hdc) {
        ReleaseDC(NULL, hdc);
    }
}
} // namespace sunrise::client::graphics
