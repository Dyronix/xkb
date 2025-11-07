#include "xkbKeyMap.h"

#include <linux/input.h>
#include <X11/keysym.h>

namespace xkb
{  
    // NOTE: Only a representative subset is mapped here; extend as needed.
    std::optional<size_t> XKeySymToIndex(KeySym_Fwd sym)
    {
        switch (sym)
        {
        // Letters
        case XK_a:
        case XK_A:
            return KEY_A;
        case XK_b:
        case XK_B:
            return KEY_B;
        case XK_c:
        case XK_C:
            return KEY_C;
        case XK_d:
        case XK_D:
            return KEY_D;
        case XK_e:
        case XK_E:
            return KEY_E;
        case XK_f:
        case XK_F:
            return KEY_F;
        case XK_g:
        case XK_G:
            return KEY_G;
        case XK_h:
        case XK_H:
            return KEY_H;
        case XK_i:
        case XK_I:
            return KEY_I;
        case XK_j:
        case XK_J:
            return KEY_J;
        case XK_k:
        case XK_K:
            return KEY_K;
        case XK_l:
        case XK_L:
            return KEY_L;
        case XK_m:
        case XK_M:
            return KEY_M;
        case XK_n:
        case XK_N:
            return KEY_N;
        case XK_o:
        case XK_O:
            return KEY_O;
        case XK_p:
        case XK_P:
            return KEY_P;
        case XK_q:
        case XK_Q:
            return KEY_Q;
        case XK_r:
        case XK_R:
            return KEY_R;
        case XK_s:
        case XK_S:
            return KEY_S;
        case XK_t:
        case XK_T:
            return KEY_T;
        case XK_u:
        case XK_U:
            return KEY_U;
        case XK_v:
        case XK_V:
            return KEY_V;
        case XK_w:
        case XK_W:
            return KEY_W;
        case XK_x:
        case XK_X:
            return KEY_X;
        case XK_y:
        case XK_Y:
            return KEY_Y;
        case XK_z:
        case XK_Z:
            return KEY_Z;

        // Digits (top row)
        case XK_0:
            return KEY_0;
        case XK_1:
            return KEY_1;
        case XK_2:
            return KEY_2;
        case XK_3:
            return KEY_3;
        case XK_4:
            return KEY_4;
        case XK_5:
            return KEY_5;
        case XK_6:
            return KEY_6;
        case XK_7:
            return KEY_7;
        case XK_8:
            return KEY_8;
        case XK_9:
            return KEY_9;

        // Modifiers & controls
        case XK_space:
            return KEY_SPACE;
        case XK_Escape:
            return KEY_ESC;
        case XK_Tab:
            return KEY_TAB;
        case XK_Caps_Lock:
            return KEY_CAPSLOCK;
        case XK_Return:
            return KEY_ENTER;
        case XK_Shift_L:
            return KEY_LEFTSHIFT;
        case XK_Shift_R:
            return KEY_RIGHTSHIFT;
        case XK_Control_L:
            return KEY_LEFTCTRL;
        case XK_Control_R:
            return KEY_RIGHTCTRL;
        case XK_Alt_L:
            return KEY_LEFTALT;
        case XK_Alt_R:
            return KEY_RIGHTALT;

        // Arrows
        case XK_Up:
            return KEY_UP;
        case XK_Down:
            return KEY_DOWN;
        case XK_Left:
            return KEY_LEFT;
        case XK_Right:
            return KEY_RIGHT;

        // Function keys (optional, extend later)
        case XK_F1:
            return KEY_F1;
        case XK_F2:
            return KEY_F2;
        case XK_F3:
            return KEY_F3;
        case XK_F4:
            return KEY_F4;
        case XK_F5:
            return KEY_F5;
        case XK_F6:
            return KEY_F6;
        case XK_F7:
            return KEY_F7;
        case XK_F8:
            return KEY_F8;
        case XK_F9:
            return KEY_F9;
        case XK_F10:
            return KEY_F10;
        case XK_F11:
            return KEY_F11;
        case XK_F12:
            return KEY_F12;

        default:
            return std::nullopt;
        }
    }
} // namespace xkb