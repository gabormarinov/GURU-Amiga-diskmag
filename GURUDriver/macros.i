*------ external definition macros -----------------------------------

XREF_EXE	MACRO
	XREF		_LVO\1
		ENDM

XREF_DOS	MACRO
	XREF		_LVO\1
		ENDM

XREF_GFX	MACRO
	XREF		_LVO\1
		ENDM

XREF_ITU	MACRO
	XREF		_LVO\1
		ENDM

*------ library dispatch macros --------------------------------------

CALLEXE		MACRO
		CALLLIB _LVO\1
		ENDM

LINKEXE		MACRO
		LINKLIB _LVO\1,_SysBase
		ENDM

LINKDOS		MACRO
		LINKLIB _LVO\1,_DOSBase
		ENDM

LINKGFX		MACRO
		LINKLIB _LVO\1,_GfxBase
		ENDM

LINKITU		MACRO
		LINKLIB _LVO\1,_IntuitionBase
		ENDM
