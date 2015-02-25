#include "ctrcommon/common.hpp"
#include "service.hpp"

#include <3ds.h>

const std::string input_get_button_name(Button button) {
    if(button == BUTTON_A) {
        return "A";
    } else if(button == BUTTON_B) {
        return "B";
    } else if(button == BUTTON_SELECT) {
        return "Select";
    } else if(button == BUTTON_START) {
        return "Start";
    } else if(button == BUTTON_DRIGHT) {
        return "DPad Right";
    } else if(button == BUTTON_DLEFT) {
        return "DPad Left";
    } else if(button == BUTTON_DUP) {
        return "DPad Up";
    } else if(button == BUTTON_DDOWN) {
        return "DPad Down";
    } else if(button == BUTTON_R) {
        return "R";
    } else if(button == BUTTON_L) {
        return "L";
    } else if(button == BUTTON_X) {
        return "X";
    } else if(button == BUTTON_Y) {
        return "Y";
    } else if(button == BUTTON_ZL) {
        return "ZL";
    } else if(button == BUTTON_ZR) {
        return "ZR";
    } else if(button == BUTTON_TOUCH) {
        return "Touch";
    } else if(button == BUTTON_CSTICK_RIGHT) {
        return "CStick Right";
    } else if(button == BUTTON_CSTICK_LEFT) {
        return "CStick Left";
    } else if(button == BUTTON_CSTICK_UP) {
        return "CStick Up";
    } else if(button == BUTTON_CSTICK_DOWN) {
        return "CStick Down";
    } else if(button == BUTTON_CPAD_RIGHT) {
        return "CirclePad Right";
    } else if(button == BUTTON_CPAD_LEFT) {
        return "CirclePad Left";
    } else if(button == BUTTON_CPAD_UP) {
        return "CirclePad Up";
    } else if(button == BUTTON_CPAD_DOWN) {
        return "CirclePad Down";
    } else if(button == BUTTON_UP) {
        return "Up";
    } else if(button == BUTTON_DOWN) {
        return "Down";
    } else if(button == BUTTON_LEFT) {
        return "Left";
    } else if(button == BUTTON_RIGHT) {
        return "Right";
    }

    return "Unknown Button";
}

void input_poll() {
    if(!service_require("hid")) {
        return;
    }

    hidScanInput();
}

bool input_is_any_pressed() {
    for(u32 button = 0; button < 32; button++) {
        if(input_is_pressed((Button) (1 << button))) {
            return true;
        }
    }

    return false;
}

Button input_get_any_pressed() {
    for(u32 button = 0; button < 32; button++) {
        if(input_is_pressed((Button) (1 << button))) {
            return (Button) (1 << button);
        }
    }

    return (Button) -1;
}

bool input_is_released(Button button) {
    if(!service_require("hid")) {
        return false;
    }

    return (hidKeysUp() & button) != 0;
}

bool input_is_pressed(Button button) {
    if(!service_require("hid")) {
        return false;
    }

    return (hidKeysDown() & button) != 0;
}

bool input_is_held(Button button) {
    if(!service_require("hid")) {
        return false;
    }

    return (hidKeysHeld() & button) != 0;
}

Touch input_get_touch() {
    if(!service_require("hid")) {
        return {0, 0};
    }

    touchPosition pos;
    hidTouchRead(&pos);
    return {pos.px, pos.py};
}