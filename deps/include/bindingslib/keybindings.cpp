/**
 * "Configuration file" - this is where the user sets what keys, shall result in what command, by translating the
 * GLFW_KEY_... value to an enum value, defined in keybindings.hpp
*/

#include "keybindings.hpp"
#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL,// handle to DLL module
                    DWORD fdwReason,   // reason for calling function
                    LPVOID lpReserved) // reserved
{
    return TRUE;// Successful DLL_PROCESS_ATTACH.
}

extern "C" {

Action modal_input(KeyInput input);
Action normal_input(KeyInput input);
Action command_input(KeyInput input);
Action command_mode(KeyInput input);
Action popup_input(KeyInput input);


Action translate(CXMode mode, KeyInput input) {
    return normal_input(input);
}

Action normal_input(KeyInput input) {
    using Keys = GLFWKeys;
    auto &[key, mod] = input;
    if (mod & MOD_CONTROL) {// ctrl+key commands
        switch (key) {
            case Keys::KEY_O:
                return Action::OpenFile;
            case Keys::KEY_W:
                return Action::WriteFile;
            case Keys::KEY_G:
                return Action::GotoLine;
            case Keys::KEY_D:
                return Action::PrintDebug;
            case Keys::KEY_ESCAPE:
                return Action::CommandPrompt;
            default:
                return Action::DefaultAction;
        }
    } else {
        // Default action is for instance, we pressed 'a' and therefore we want to insert 'a' into whatever active buffer
        return Action::DefaultAction;
    }
}

Action popup_input(KeyInput input) {
    using Keys = GLFWKeys;
    // we ignore modifier in command mode
    auto&[k, _] = input;
    switch(k) {
        case Keys::KEY_ENTER:
            return Action::PopupItemChosen;
        case Keys::KEY_UP:
            return Action::ScrollUp;
        case Keys::KEY_DOWN:
            return Action::ScrollDown;
        case Keys::KEY_ESCAPE:
            return Action::Escape;
    }
    return Action::DefaultAction;
}

Action command_mode(KeyInput input) {
    using Keys = GLFWKeys;
    // we ignore modifier in command mode
    auto&[k, _] = input;
    switch(k) {
        case Keys::KEY_N:
            return Action::UseModeNormal;
    }
    return Action::DefaultAction;
}

}
