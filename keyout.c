// include the interface to Pd
#include "m_pd.h"
// instead of #include "WinUser.h"
#ifdef _WINDOWS
    #define KEYEVENTF_EXTENDEDKEY   0x0001
    #define KEYEVENTF_KEYUP		    0x0002
    #define KEYEVENTF_SCANCODE      0x0008
    #define MAPVK_VK_TO_VSC         (0)
    #define MAPVK_VK_TO_VSC_EX      (4)
    #define VK_LEFT                 0x25
    #define VK_UP                   0x26
    #define VK_RIGHT                0x27
    #define VK_DOWN                 0x28

    extern unsigned int __stdcall MapVirtualKeyA(unsigned int uCode, unsigned int uMapType);
    //extern void __stdcall keybd_event(unsigned char bVk, unsigned char bScan, unsigned long dwFlags, unsigned __int64 dwExtraInfo);

    typedef unsigned __int64 ULONG_PTR, * PULONG_PTR;
    typedef struct tagMOUSEINPUT {
        long    dx;
        long    dy;
        unsigned long   mouseData;
        unsigned long   dwFlags;
        unsigned long   time;
        ULONG_PTR dwExtraInfo;
    } MOUSEINPUT, * PMOUSEINPUT, * LPMOUSEINPUT;

    typedef struct tagKEYBDINPUT {
        unsigned short  wVk;
        unsigned short  wScan;
        unsigned long   dwFlags;
        unsigned long   time;
        /*
         * When dwFlags has KEYEVENTF_SYSTEMINJECTION specified this field may carry
         * KEY_UNICODE_SEQUENCE_ITEM or KEY_UNICODE_SEQUENCE_END which are used by InputService
         * to distinguish injected unicode sequences. Those flags are stored in low word.
         * When dwFlags has KEYEVENTF_ATTRIBUTED_INPUT specified this field carries in its high word
         * ID of attributes associated with injected input. This ID is assigned by InputService and
         * recognized only by it.
         * For all other usage scenarios please refer to official documentation.
         */
        ULONG_PTR dwExtraInfo;
    } KEYBDINPUT, * PKEYBDINPUT, * LPKEYBDINPUT;

    typedef struct tagHARDWAREINPUT {
        unsigned long   uMsg;
        unsigned short  wParamL;
        unsigned short  wParamH;
    } HARDWAREINPUT, * PHARDWAREINPUT, * LPHARDWAREINPUT;

    #define INPUT_MOUSE     0
    #define INPUT_KEYBOARD  1
    #define INPUT_HARDWARE  2

    typedef struct tagINPUT {
        unsigned long   type;
        union
        {
            MOUSEINPUT      mi;
            KEYBDINPUT      ki;
            HARDWAREINPUT   hi;
        };
    } INPUT, * PINPUT, * LPINPUT;

    extern unsigned int __stdcall SendInput(
            _In_ unsigned int cInputs,            // number of input in the array
            _In_reads_(cInputs) LPINPUT pInputs,  // array of inputs
            _In_ int cbSize);                     // sizeof(INPUT)
#else   // macOS
#include <CoreGraphics/CoreGraphics.h>
void simulateKeyPress(CGKeyCode keyCode)
{
    // Create an HID hardware event source
    CGEventSourceRef src = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);

    // Create a new keyboard key press event
    CGEventRef keyDown = CGEventCreateKeyboardEvent(src, keyCode, true);
    
    // Post keyboard key down event
    CGEventPost(kCGHIDEventTap, keyDown);

    // Create a new keyboard key release event
    CGEventRef keyUp = CGEventCreateKeyboardEvent(src, keyCode, false);
    
    // Post keyboard key up event
    CGEventPost(kCGHIDEventTap, keyUp);

    // Release resources
    CFRelease(keyDown);
    CFRelease(keyUp);
    CFRelease(src);
}
#endif /* _WINDOWS */

// define a new "class"
static t_class* keyout_class;

// this is the dataspace of our new object
// the first (mandatory) "t_object"
// and a variable that holds the current keycode
typedef struct _keyout
{
	t_object x_obj;
	t_float i_keycode;
} t_keyout;

// this method is called whenever a "float" is sent to the object
// a reference to the class-dataspace is given as argument
// this enables us to do something with the data
void keyout_float(t_keyout* x, t_float f)
{
	x->i_keycode = f;
#ifdef _WINDOWS
	int i_vkeycode = (int)f;
	int i_scancode = MapVirtualKeyA(i_vkeycode, MAPVK_VK_TO_VSC);
	// keybd_event(_In_ BYTE bVk, _In_ BYTE bScan, _In_ DWORD dwFlags, _In_ ULONG_PTR dwExtraInfo);
	//keybd_event(i_vkeycode, i_scancode, 0, 0);
	//keybd_event(i_vkeycode, i_scancode, KEYEVENTF_KEYUP, 0);

    INPUT input[2] = { 0 };
    if (i_vkeycode == VK_LEFT ||
        i_vkeycode == VK_UP ||
        i_vkeycode == VK_RIGHT ||
        i_vkeycode == VK_DOWN)
    {
        i_scancode += 0xe000;
        input[0] = (INPUT){ 
                    .type = INPUT_KEYBOARD,
                    .ki = (KEYBDINPUT) {
                            .wVk = 0,
                            .wScan = i_scancode,
                            .dwFlags = KEYEVENTF_EXTENDEDKEY | KEYEVENTF_SCANCODE
                            } };
        input[1] = (INPUT){ 
                    .type = INPUT_KEYBOARD,
                    .ki = (KEYBDINPUT) {
                            .wVk = 0,
                            .wScan = i_scancode,
                            .dwFlags = KEYEVENTF_EXTENDEDKEY | KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP
                            } };
    }
    else
    {
        input[0] = (INPUT){
                    .type = INPUT_KEYBOARD,
                    .ki = (KEYBDINPUT) {
                            .wVk = 0,
                            .wScan = i_scancode,
                            .dwFlags = KEYEVENTF_SCANCODE
                            } };
        input[1] = (INPUT){
                    .type = INPUT_KEYBOARD,
                    .ki = (KEYBDINPUT) {
                            .wVk = 0,
                            .wScan = i_scancode,
                            .dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP
                            } };
    }
    unsigned int ret = SendInput(2, input, sizeof(INPUT));
    if (ret != 2)
    {
        // failure
    }
#else   // macOS
    // For a complete list of CGKeyCode see Events.h
    CGKeyCode keyCode = (int)f;
    simulateKeyPress(keyCode);
#endif
}

// this is the "constructor" of the class
// we have one argument of type floating-point (as specified below in the keyout_setup() routine)
void* keyout_new(t_floatarg f)
{
	t_keyout* x = (t_keyout*)pd_new(keyout_class);
	x->i_keycode = f;
	return (void*)x;
}

// define the function-space of the class
void keyout_setup(void)
{
	keyout_class = class_new(gensym("keyout"),
		(t_newmethod)keyout_new,
		0, sizeof(t_keyout),
		CLASS_DEFAULT,
		A_DEFFLOAT, 0);
	class_addfloat(keyout_class, keyout_float);
}
