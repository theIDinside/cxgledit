/**
 * "Configuration file" - this is where the user sets what keys, shall result in what command, by translating the
 * GLFW_KEY_... value to an enum value, defined in keybindings.hpp
*/

#include "keybindings.hpp"
#include <windows.h>

BOOL WINAPI DllMain(
        HINSTANCE hinstDLL,  // handle to DLL module
        DWORD fdwReason,     // reason for calling function
        LPVOID lpReserved )  // reserved
{
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

extern "C" {

Action translate(CXMode mode, KeyInput input) {
        auto& [key, mod] = input;
        if (mod & MOD_CONTROL) {// ctrl+key commands
            switch (key) {
                case GLFWKeys::KEY_O:
                    return Action::OpenFile;
                case GLFWKeys::KEY_W:
                    return Action::WriteFile;
                case GLFWKeys::KEY_G:
                    return Action::GotoLine;
                case GLFWKeys::KEY_D:
                    return Action::PrintDebug;
                case GLFWKeys::KEY_ESCAPE:
                    return Action::CommandPrompt;
                default:
                    return Action::DefaultAction;
            }
        } else {
            return Action::DefaultAction;
        }
    }
}

