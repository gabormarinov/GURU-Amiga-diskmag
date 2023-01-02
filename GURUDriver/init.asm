	SECTION		printer

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/lists.i"
	INCLUDE		"exec/memory.i"
	INCLUDE		"exec/ports.i"
	INCLUDE		"exec/libraries.i"

	INCLUDE		"macros.i"

*------ Imported Functions -------------------------------------------

	XREF_EXE	CloseLibrary
	XREF_EXE	OpenLibrary
	XREF		_AbsExecBase


	XREF		_PEDData


*------ Exported Globals ---------------------------------------------

	XDEF		_Init
	XDEF		_Expunge
	XDEF		_Open
	XDEF		_Close
	XDEF		_PD
	XDEF		_PED
	XDEF		_SysBase
	XDEF		_DOSBase
	XDEF		_GfxBase
	XDEF		_IntuitionBase


**********************************************************************
	SECTION		printer,DATA
_PD		DC.L	0
_PED		DC.L	0
_SysBase	DC.L	0
_DOSBase	DC.L	0
_GfxBase	DC.L	0
_IntuitionBase	DC.L	0


**********************************************************************
	SECTION		printer,CODE
_Init:
		MOVE.L	4(A7),_PD
		LEA	_PEDData(PC),A0
		MOVE.L	A0,_PED
		MOVE.L	A6,-(A7)
		MOVE.L	_AbsExecBase,A6
		MOVE.L	A6,_SysBase

*	    ;------ open the dos library
		LEA	DLName(PC),A1
		MOVEQ	#0,D0
		CALLEXE OpenLibrary
		MOVE.L	D0,_DOSBase
		BEQ	initDLErr


*	    ;------ open the graphics library
		LEA	GLName(PC),A1
		MOVEQ	#0,D0
		CALLEXE OpenLibrary
		MOVE.L	D0,_GfxBase
		BEQ	initGLErr

*	    ;------ open the intuition library
		LEA	ILName(PC),A1
		MOVEQ	#0,D0
		CALLEXE OpenLibrary
		MOVE.L	D0,_IntuitionBase
		BEQ	initILErr

		MOVEQ	#0,D0
pdiRts:
		MOVE.L	(A7)+,A6
		RTS

initPAErr:
		MOVE.L	_IntuitionBase,A1
		LINKEXE	CloseLibrary

initILErr:
		MOVE.L	_GfxBase,A1
		LINKEXE	CloseLibrary

initGLErr:
		MOVE.L	_DOSBase,A1
		LINKEXE	CloseLibrary

initDLErr:
		MOVEQ	#-1,D0
		BRA.S	pdiRts

ILName:
		DC.B	'intuition.library'
		DC.B	0
DLName:
		DC.B	'dos.library'
		DC.B	0
GLName:
		DC.B	'graphics.library'
		DC.B	0
		DS.W	0


*---------------------------------------------------------------------
_Expunge:
		MOVE.L	_IntuitionBase,A1
		LINKEXE	CloseLibrary

		MOVE.L	_GfxBase,A1
		LINKEXE	CloseLibrary

		MOVE.L	_DOSBase,A1
		LINKEXE	CloseLibrary


*---------------------------------------------------------------------
_Open:
		MOVEQ	#0,D0
		RTS



*---------------------------------------------------------------------
_Close:
		MOVEQ	#0,D0
		RTS

		END
