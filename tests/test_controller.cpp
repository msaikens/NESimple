#include <catch2/catch_test_macros.hpp>

#include "controller.hpp"

using namespace nes;

TEST_CASE("Controller shifts out latched button bits in NES order") {
    Controller controller {};

    controller.set_buttons(static_cast<u8>(Controller::A | Controller::Start));

    // Strobe high to latch, then low to start shifting.
    controller.write(1);
    controller.write(0);

    REQUIRE((controller.read() & 1) == 1); // A
    REQUIRE((controller.read() & 1) == 0); // B
    REQUIRE((controller.read() & 1) == 0); // Select
    REQUIRE((controller.read() & 1) == 1); // Start
}