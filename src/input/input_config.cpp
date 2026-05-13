#include "input/input_config.hpp"

#include <fstream>
#include <string>

namespace nes {
namespace {

void set_value(InputMapping& mapping, const std::string& key, int value) {
    if (key == "keyboard.a") {
        mapping.a.keyboard_scancode = value;
    } else if (key == "keyboard.b") {
        mapping.b.keyboard_scancode = value;
    } else if (key == "keyboard.select") {
        mapping.select.keyboard_scancode = value;
    } else if (key == "keyboard.start") {
        mapping.start.keyboard_scancode = value;
    } else if (key == "keyboard.up") {
        mapping.up.keyboard_scancode = value;
    } else if (key == "keyboard.down") {
        mapping.down.keyboard_scancode = value;
    } else if (key == "keyboard.left") {
        mapping.left.keyboard_scancode = value;
    } else if (key == "keyboard.right") {
        mapping.right.keyboard_scancode = value;
    } else if (key == "gamepad.a") {
        mapping.a.gamepad_button = value;
    } else if (key == "gamepad.b") {
        mapping.b.gamepad_button = value;
    } else if (key == "gamepad.select") {
        mapping.select.gamepad_button = value;
    } else if (key == "gamepad.start") {
        mapping.start.gamepad_button = value;
    } else if (key == "gamepad.up") {
        mapping.up.gamepad_button = value;
    } else if (key == "gamepad.down") {
        mapping.down.gamepad_button = value;
    } else if (key == "gamepad.left") {
        mapping.left.gamepad_button = value;
    } else if (key == "gamepad.right") {
        mapping.right.gamepad_button = value;
    }
}

void write_binding(
    std::ostream& out,
    const char* keyboard_key,
    const char* gamepad_key,
    const ButtonBinding& binding
) {
    out << keyboard_key << '=' << binding.keyboard_scancode << '\n';
    out << gamepad_key << '=' << binding.gamepad_button << '\n';
}

} // namespace

std::filesystem::path default_userdata_dir() {
    return std::filesystem::current_path() / "userdata";
}

std::filesystem::path default_input_mapping_path() {
    return default_userdata_dir() / "input_mapping.cfg";
}

bool load_input_mapping_from_file(
    const std::filesystem::path& path,
    InputMapping& mapping
) {
    std::ifstream file(path);
    if (!file) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        const std::size_t equals = line.find('=');
        if (equals == std::string::npos) {
            continue;
        }

        const std::string key = line.substr(0, equals);
        const std::string value_text = line.substr(equals + 1);

        try {
            const int value = std::stoi(value_text);
            set_value(mapping, key, value);
        } catch (...) {
            // Ignore malformed lines instead of failing the whole config.
        }
    }

    return true;
}

bool save_input_mapping_to_file(
    const std::filesystem::path& path,
    const InputMapping& mapping
) {
    std::error_code error {};
    std::filesystem::create_directories(path.parent_path(), error);

    if (error) {
        return false;
    }

    std::ofstream file(path, std::ios::out | std::ios::trunc);
    if (!file) {
        return false;
    }

    file << "# NESimple input mapping\n";
    file << "# Values are SDL scancode integers and SDL gamepad button enum integers.\n";
    file << "# -1 means unbound.\n\n";

    write_binding(file, "keyboard.a", "gamepad.a", mapping.a);
    write_binding(file, "keyboard.b", "gamepad.b", mapping.b);
    write_binding(file, "keyboard.select", "gamepad.select", mapping.select);
    write_binding(file, "keyboard.start", "gamepad.start", mapping.start);
    write_binding(file, "keyboard.up", "gamepad.up", mapping.up);
    write_binding(file, "keyboard.down", "gamepad.down", mapping.down);
    write_binding(file, "keyboard.left", "gamepad.left", mapping.left);
    write_binding(file, "keyboard.right", "gamepad.right", mapping.right);

    return true;
}

} // namespace nes