#include "controller.hpp"

namespace nes {

/// @brief Controller represents an NES controller, which has a set of buttons that can be latched and read by the CPU.
/// @param state The initial state of the buttons.
/// The set_buttons method allows us to set the current state of the buttons, which can be
// used in tests to simulate button presses. The write method controls the strobe line, which latches the current button state when set high.
// The read method returns the current button state in a serial manner, shifting out the bits one by one with each read.
void Controller::set_buttons(u8 state) {
    state_ = state;
}
/// @brief  Write to the controller's strobe line. When strobe is high, the controller latches the current button state and keeps returning the 
/// A button state on reads.
/// @param value The value to write to the strobe line. Only bit 0 is used; if it is set, strobe is high, otherwise it is low.

void Controller::write(u8 value) {
    strobe_ = (value & 0x01U) != 0;

    if (strobe_) {
        shift_ = state_;
    }
}
/// @brief Read the current button state in a serial manner. When strobe is high, this returns the A button state. When strobe is low, 
/// it shifts out the latched button state with each read.
/// @return The current button state. The returned value has bit 0 set if the current button is pressed, and bit 6 is always set to 
/// 1 as per NES controller behavior.
u8 Controller::read() {
    if (strobe_) {
        return static_cast<u8>((state_ & 0x01U) | 0x40U);
    }

    const u8 value = static_cast<u8>((shift_ & 0x01U) | 0x40U);
    shift_ = static_cast<u8>(shift_ >> 1U);
    return value;
}

}