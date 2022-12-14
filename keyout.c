// include the interface to Pd
#include "m_pd.h"
// instead of #include "WinUser.h"
#ifdef _WINDOWS
	#define MAPVK_VK_TO_VSC		(0)
	#define KEYEVENTF_KEYUP		0x0002
	extern unsigned int __stdcall MapVirtualKeyA(unsigned int uCode, unsigned int uMapType);
	extern void __stdcall keybd_event(unsigned char bVk, unsigned char bScan, unsigned long dwFlags, unsigned __int64 dwExtraInfo);
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
	int i_vkeycode = floor(f);
	int i_scancode = MapVirtualKeyA(i_vkeycode, MAPVK_VK_TO_VSC);
	// keybd_event(_In_ BYTE bVk, _In_ BYTE bScan, _In_ DWORD dwFlags, _In_ ULONG_PTR dwExtraInfo);
	keybd_event(i_vkeycode, i_scancode, 0, 0);
	keybd_event(i_vkeycode, i_scancode, KEYEVENTF_KEYUP, 0);
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
