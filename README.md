# PdExternal_keyout

Pure data external for generating computer keyboard events (keyout object)

Sends keyboard events to the active window. 
The parameter is a virtual-key code. 
1. Windows:
The code must be a value in the range 1 to 254. For a complete list, see https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes. 
2. macOS:
The code must be a value in the range 1 to 127. For a complete list, see Events.h. 

Usage

See the keyout-help.pd file.

There are two sample files for macOS and Studio One:
StopStartRecord.pd and UpDownRecord.pd
