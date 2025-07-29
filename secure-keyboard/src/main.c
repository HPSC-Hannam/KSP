#include "input_monitor.h"
#include "capture_guard.h"
#include "self_defense.h"
#include "ui.h"

int main() {
    init_self_defense();
    init_capture_guard();
    start_ui();

    return start_input_monitor();
}
