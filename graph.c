/*****************************************************************

Program:	graph.c

			A GURU lemezújság grafikus rutinjai.
	
Szerzõ:		Marinov Gábor	<<Gaborca>>

Dátum:		1990.07.29.

Utolsó
módosítás:	2011.08.11.

******************************************************************/

#include <stdio.h>
#include <exec/types.h>
#include <intuition/intuition.h>

#include <graphics/text.h>
#include <libraries/diskfont.h>
#include <libraries/dos.h>

#include <libraries/ppbase.h>

#include <functions.h>

struct IntuitionBase	*IntuitionBase	= NULL;
struct GfxBase 			*GfxBase		= NULL;
struct DosBase			*DosBase		= NULL;
struct PPBase 			*PPBase 		= NULL;

struct Library			*DiskfontBase	= NULL;

extern	struct	Screen	*PicScreen;
extern	struct	Window	*PicWindow;

struct Screen	*TopScreen, *BottomScreen, *LogoScreen				 = NULL;
struct Window	*TopWindow, *MiddleWindow, *BottomWindow, *LogoWindow, *TestWindow = NULL;
struct RastPort	*TopRast, *MiddleRast, *BottomRast	= NULL;



struct TextFont			*HeaderFont, *NormalFont = NULL;
struct TextAttr			textattr;

BYTE	OldPri		= 0;
BYTE	OldPri2		= 0;
ULONG	IntuiLock	= 0L;

extern	struct Screen *LoadIFF();

BOOL	TopUp;

extern	struct	Task	*InputTask, *GuruTask;

extern	BOOL	QPage;
extern	BOOL	IsMusicInMemory;
extern	BOOL	PlayMusic;
extern	void	EjectMusic();

extern	UWORD	Color;

extern	char	*ContAddr;
extern	long	ContSize;

#define	MAXICON 21

#define TOPBORDERHEIGHT 27
#define BOTTOMBORDERHEIGHT 40
#define BORDEREDGE 5
#define BORDERMIDDLE 1

#define EMPTYPOINTER 0
#define ARROWPOINTER 1
#define CLOCKPOINTER 2

int	Pointer = ARROWPOINTER;
int	TopShortPointer;

struct	Image	icon[MAXICON];


/* id:ikonok, bd:border */

extern UWORD id[];
extern UWORD bd[];

UWORD	Empty_Pointer = 0;

unsigned	IconOffset[] =
{
	0x0000,0x0228,0x0450,0x0678,0x08a0,0x0ac8,0x0cf0,0x0f18,0x1140,
	0x1368,0x1590,0x17b8,0x19e0,0x1c08,0x1e30,0x2058,0x2280,0x24a8,
	0x26d0,0x2a0c,0x2ae4
};

/*  az ikonok koordinátái x,y */
/*  az ikonok magassága és szélessége  */
unsigned IconX[] =
{
	18,92,166,18,92,240,414,488,562,18,92,166,18,92,240,414,488,562,315,12,590
};
unsigned IconY[] =
{
	9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,8,8
};
unsigned IconH[] =
{
	23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,12,12
};
unsigned IconW[] =
{
	59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,59,83,38,38
};

BYTE libben[] =
{
	-20,-13,-13,-13,-13,-13,-13,-13,-12,-12,-12,-11,-11,-11,-10,-10,
	-9,-9,-9,-8,-8,-7,-6,-6,-5,-4,-4,-3,-2,-2,-1,-1
};
BYTE libben2[] =
{
	-13,-13,-13,-12,-12,-12,-11,-11,-11,-10,-10,
	-9,-9,-9,-8,-8,-7,-6,-6,-5,-4,-4,-3,-2,-2,-1,-1
};

UWORD ColorMap[6][8] =
{
	{ 0x006,0x446,0x558,0x77A,0x89B,0xAAD,0xCCF,0xFF0 }, /* GURUalap */
	{ 0x406,0x627,0x739,0x95B,0xA7C,0xCAE,0xEDE,0xFF0 }, /* Lila     */
	{ 0x422,0x531,0x640,0x860,0x970,0xCA6,0xEDB,0xEE0 }, /* Barna    */
	{ 0x530,0x850,0x960,0xB80,0xCA0,0xEC8,0xEEC,0xFF0 }, /* Sarga    */
	{ 0x040,0x152,0x264,0x486,0x5A7,0xACC,0xCDE,0xDE1 }, /* Zold     */
	{ 0x334,0x444,0x666,0x888,0x999,0xBBB,0xDDD,0xDD2 }  /* Szurke   */
};

USHORT ClockImage[]=
{
0x0000,0x0000 ,0x2004,0x1008 ,0x4008,0x3996 ,0x0000,0x77ee,
0x06c0,0x1938 ,0x1ff0,0x200c ,0x1ff0,0x602e ,0x7ffc,0x8043,
0x7ffc,0x8083 ,0x3ff8,0xc107 ,0x7ffc,0x8003 ,0x7ffc,0x8003,
0x1ff0,0x600e ,0x1ff0,0x200c ,0x06c0,0x1938 ,0x0000,0x07e0
};


/***********************************************************

           A 8 állandó Gadget : az ikonok

************************************************************/

struct Gadget Gadget8 =
	{
	NULL,
	562, 9,
	59, 23,
	GADGIMAGE|GADGHNONE,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&icon[8],
	NULL,
	NULL,
	NULL,
	NULL,
	8,
	NULL,
	};
struct Gadget Gadget7 =
	{
	&Gadget8,
	488, 9,
	59, 23,
	GADGIMAGE|GADGHNONE,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&icon[7],
	NULL,
	NULL,
	NULL,
	NULL,
	7,
	NULL,
	};
struct Gadget Gadget6 =
	{
	&Gadget7,
	414, 9,
	59, 23,
	GADGIMAGE|GADGHIMAGE,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&icon[6],
	(APTR)&icon[15],
	NULL,
	NULL,
	NULL,
	6,
	NULL,
	};
struct Gadget Gadget5 =
	{
	&Gadget6,
	315, 9,
	83, 23,
	GADGIMAGE|GADGHNONE,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&icon[18],
	NULL,
	NULL,
	NULL,
	NULL,
	5,
	NULL,
	};
struct Gadget Gadget4 =
	{
	&Gadget5,
	240, 9,
	59, 23,
	GADGIMAGE|GADGHIMAGE,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&icon[5],
	(APTR)&icon[14],
	NULL,
	NULL,
	NULL,
	4,
	NULL,
	};

struct Gadget Gadget3 =
	{
	&Gadget4,
	166, 9,
	59, 23,
	GADGIMAGE|GADGHIMAGE,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&icon[2],
	(APTR)&icon[11],
	NULL,
	NULL,
	NULL,
	3,
	NULL,
	};

struct Gadget Gadget2 =
	{
	&Gadget3,
	92, 9,
	59, 23,
	GADGIMAGE|GADGHIMAGE,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&icon[1],
	(APTR)&icon[10],
	NULL,
	NULL,
	NULL,
	2,
	NULL,
	};

struct Gadget Gadget1 =
	{
	&Gadget2,
	18, 9,
	59, 23,
	GADGIMAGE|GADGHIMAGE,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&icon[0],
	(APTR)&icon[9],
	NULL,
	NULL,
	NULL,
	1,
	NULL,
	};

/***************************************************************/


struct Image BorderTop =
	{
	0,
	0,
	640,
	5,
	3,
	&bd[0x0000],
	7,
	0,
	NULL
	};

struct Image BorderMiddle =
	{
	0,
	0,
	640,
	1,
	3,
	&bd[0x04b0/2],
	7,
	0,
	NULL
	};

struct Image BorderBottom =
	{
	0,
	0,
	640,
	5,
	3,
	&bd[0x05a0/2],
	7,
	0,
	NULL
	};


struct NewScreen topscreen =
	{
	0,
	284,
	640,
	216,
	3,
	0,
	0,
	HIRES,
	CUSTOMSCREEN|SCREENBEHIND,
	NULL,
	NULL,
	NULL,
	NULL,
	};


struct NewScreen bottomscreen =
	{
	0,
	256, /* 264+18 */
	640,
	40,
	3,
	0,
	0,
	HIRES,
	CUSTOMSCREEN|SCREENBEHIND,
	NULL,
	NULL,
	NULL,
	NULL,
	};

struct NewWindow topwindow =
	{
	0,
	0,
	640,
	27,
	0,
	0,
	NULL,
	BORDERLESS|RMBTRAP|ACTIVATE,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	640,
	22,
	640,
	22,
	CUSTOMSCREEN
	};

struct NewWindow middlewindow =
	{
	0,
	34,
	640,
	165,
	0,
	0,
	GADGETUP|GADGETDOWN,

	BORDERLESS|RMBTRAP,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	640,
	165,
	640,
	165,
	CUSTOMSCREEN
	};

struct NewWindow bottomwindow =
	{
	0,
	0,
	640,
	40,
	0,
	0,
	GADGETUP|GADGETDOWN,

	BORDERLESS|RMBTRAP,
	&Gadget1,			/* &Gadget1 */
	NULL,
	NULL,
	NULL,
	NULL,
	640,
	40,
	640,
	40,
	CUSTOMSCREEN
	};

struct NewWindow testwindow =
	{
	0,
	0,
	640,
	10,
	2,
	1,
	NULL,
	BORDERLESS|RMBTRAP, 
	NULL,
	NULL,
	(UBYTE *)"PAL Amiga teszt (C) Gaborca",
	NULL,
	NULL,
	0,
	0,
	0,
	0,
	WBENCHSCREEN
	};


/*************************************************************/

SetMyPointer( type )
int type;
{
	int old;

	old = Pointer;

	switch( type ){
	case EMPTYPOINTER:
		if(TopWindow)		SetPointer(TopWindow	, &Empty_Pointer,1,16,0,0);
		if(MiddleWindow)	SetPointer(MiddleWindow	, &Empty_Pointer,1,16,0,0);
		if(BottomWindow)	SetPointer(BottomWindow	, &Empty_Pointer,1,16,0,0);
		if(LogoWindow)		SetPointer(LogoWindow	, &Empty_Pointer,1,16,0,0);
		if(PicWindow)		SetPointer(PicWindow	, &Empty_Pointer,1,16,0,0);
		Pointer = EMPTYPOINTER;
	break;

	case ARROWPOINTER:
		if(TopWindow)		ClearPointer( TopWindow );
		if(MiddleWindow)	ClearPointer( MiddleWindow );
		if(BottomWindow)	ClearPointer( BottomWindow );
		if(LogoWindow)		ClearPointer( LogoWindow );
		if(PicWindow)		ClearPointer( PicWindow );
		Pointer = ARROWPOINTER;
	break;

	case CLOCKPOINTER:
		if(TopWindow)		SetPointer(TopWindow	, ClockImage,16,16,0,0);
		if(MiddleWindow)	SetPointer(MiddleWindow	, ClockImage,16,16,0,0);
		if(BottomWindow)	SetPointer(BottomWindow	, ClockImage,16,16,0,0);
		if(LogoWindow)		SetPointer(LogoWindow	, ClockImage,16,16,0,0);
		if(PicWindow)		SetPointer(PicWindow	, ClockImage,16,16,0,0);
		Pointer = CLOCKPOINTER;
	break;
	}

	if( LogoWindow )	ActivateWindow( LogoWindow );
	return( old );
}


/*********************************************************/

Freeze()
{
	IntuiLock = LockIBase( 0L );
	OldPri  = SetTaskPri( InputTask ,-127 );
	OldPri2 = SetTaskPri( GuruTask  , 127 );
	Forbid();
}

UnFreeze()
{
	Permit();
	SetTaskPri( InputTask , OldPri );
	SetTaskPri( GuruTask  , OldPri2 );
	UnlockIBase( IntuiLock );
}

/*********************************************************/

/* A Screen-t TopOffset=284-tõl 0-ig fellibbenti ! */

ScreenLongUp( Screen )
register struct	Screen *Screen;
{
	register BYTE i;

	WBenchToBack();
	SetMyPointer( EMPTYPOINTER );
	Freeze();
	for( i=0; i<32 ; i++ )
		MoveScreen( Screen ,0, libben[i] );
	WaitTOF();
	UnFreeze();
	SetMyPointer( ARROWPOINTER );
	WBenchToBack();
}

ScreenLongDown( Screen )
register struct Screen *Screen;
{
	register BYTE i;

	WBenchToBack();
	SetMyPointer( EMPTYPOINTER );
	Freeze();
	for( i=0; i<32 ; i++ )
		MoveScreen( Screen ,0, -libben[i] );
	WaitTOF();
	UnFreeze();
	SetMyPointer( ARROWPOINTER );
	WBenchToBack();
}

/*********************************************************/


TopBottomLongUp()
{
	register BYTE i;

	WBenchToBack();
	SetMyPointer( EMPTYPOINTER );
	Freeze();

	ScreenToFront( TopScreen );
	for( i=0; i<32 ; i++ )
		MoveScreen( TopScreen ,0, libben[i] );

	ScreenToFront( BottomScreen );
	for( i=20; i<30 ; i++ )
		MoveScreen( BottomScreen ,0, libben[i] );

	WaitTOF();
	UnFreeze();
	SetMyPointer( ARROWPOINTER );
	WBenchToBack();
	TopUp = TRUE;
}

TopBottomLongDown()
{
	register BYTE i;

	WBenchToBack();
	SetMyPointer( EMPTYPOINTER );
	Freeze();

	for( i=0; i<32 ; i++ )
		MoveScreen( TopScreen ,0, -libben[i] );

	for( i=20; i<30 ; i++ )
		MoveScreen( BottomScreen ,0, -libben[i] );
	ScreenToBack( BottomScreen );


	WaitTOF();
	UnFreeze();
	SetMyPointer( ARROWPOINTER );
	WBenchToBack();
	TopUp = FALSE;
}

TopShortUp()
{
	register BYTE i;

	WBenchToBack();

	if(QPage){
		MoveScreen( TopScreen ,0, -256);
		TopUp = TRUE;
		return;
	}

	Freeze();
	for( i=0; i<27 ; i++ )
		MoveScreen( TopScreen ,0, libben2[i] );

	WaitTOF();
	UnFreeze();
	
	SetMyPointer( TopShortPointer );
	WBenchToBack();
	TopUp = TRUE;
}

TopShortDown()
{
	register BYTE i;

	WBenchToBack();

	if(QPage){
		MoveScreen( TopScreen ,0, 256);
		TopUp = FALSE;
		return;
	}

	TopShortPointer=SetMyPointer( EMPTYPOINTER );

	Freeze();
  	for( i=0; i<27 ; i++ )
		MoveScreen( TopScreen ,0, -libben2[i] );
	WaitTOF();
	UnFreeze();

	WBenchToBack();
	TopUp = FALSE;
}

SetColor( palette )
UWORD palette;
{
	LoadRGB4(&TopScreen->ViewPort,   &ColorMap[ palette ][0],8 );
	LoadRGB4(&BottomScreen->ViewPort,&ColorMap[ palette ][0],8 );

	SetAPen(TopRast,7);
	SetBPen(TopRast,2);

	SetAPen(MiddleRast,6);
	SetBPen(MiddleRast,0);

	SetAPen(BottomRast,6);
	SetBPen(BottomRast,0);

	SetDrMd( TopRast    , JAM1 );
	SetDrMd( MiddleRast , JAM1 );
	SetDrMd( BottomRast , JAM1 );
}

/*  csak a 100 v 101 -es ikont lehet törölni ! */

DeleteIcon( Num )
BYTE Num;
{
	SetAPen( TopRast,2 );
	RectFill( TopRast, IconX[Num-81],IconY[Num-81],IconX[Num-81]+IconW[Num-81]-1,IconY[Num-81]+IconH[Num-81]-1 );
}

WriteHeader( text )
char *text;
{
	register long x;

	x = 145 + ((35-strlen( text ))*10)  /2;
	SetAPen( TopRast , 2 );
	RectFill( TopRast, 145 ,5 , 494+2 , 21 );	

	Move( TopRast , x+2 , 18+2 );
	SetAPen( TopRast , 1 );
	Text( TopRast , text , strlen(text) );

	Move( TopRast , x , 18 );
	SetAPen( TopRast , 6 );
	Text( TopRast , text , strlen(text) );
}

DrawTopBorder()
{
	int i;
	DrawImage( TopRast , &BorderTop , 0 , 0 );

	for( i=0; i<( TOPBORDERHEIGHT - 2*BORDEREDGE ) ; i++ )
		DrawImage( TopRast , &BorderMiddle , 0 , BORDEREDGE + i );

	DrawImage( TopRast , &BorderBottom , 0 , BORDEREDGE + i );
}

DrawBottomBorder()
{
	int i;
	DrawImage( BottomRast , &BorderTop , 0 , 0 );

	for( i=0; i<( BOTTOMBORDERHEIGHT - 2*BORDEREDGE ) ; i++ )
		DrawImage( BottomRast , &BorderMiddle , 0 , BORDEREDGE + i );

	DrawImage( BottomRast , &BorderBottom , 0 , BORDEREDGE + i );
}

CloseScreens()
{
	if(TopWindow)		CloseWindow(TopWindow);
	if(MiddleWindow)	CloseWindow(MiddleWindow);
	if(BottomWindow)	CloseWindow(BottomWindow);

	if(TopScreen)		CloseScreen(TopScreen);
	if(BottomScreen)	CloseScreen(BottomScreen);

	TopWindow = MiddleWindow = BottomWindow = NULL;
	TopScreen = BottomScreen = NULL;
}

CleanUp()
{
	CloseScreens();

	if( HeaderFont )	CloseFont(HeaderFont);
	if( NormalFont )	CloseFont(NormalFont);

/* Tartalomjegyzék fájl memória felszabadítása */
	if( ContAddr )		UnLoadFile( ContAddr , ContSize );

/* Ha van a memóriában zene, akkor azt leállítja és a memóriát felszabadítja */
	if( IsMusicInMemory ) EjectMusic();

	if( LogoScreen ){
		ScreenLongDown( LogoScreen );
		CloseWindow(LogoWindow);
		CloseScreen(LogoScreen);
	}

	if(GfxBase)			CloseLibrary(GfxBase);
	if(IntuitionBase)	CloseLibrary(IntuitionBase);
	if(DosBase)			CloseLibrary(DosBase);

	if(DiskfontBase)	CloseLibrary(DiskfontBase);
	if(PPBase)			CloseLibrary(PPBase);
}

OpenLibs()
{
	SHORT Lines;

	IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library",0L);
	if( !IntuitionBase ){
		puts("intuition.library nyitási probléma!");
		CleanUp();
		exit( FALSE );
	}
	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0L);
	if( !GfxBase){
		puts("graphics.library nyitási probléma!");
		CleanUp();
		exit(FALSE);
	}
	DosBase = (struct DosBase *)OpenLibrary("dos.library",0L);
	if( !DosBase ){
		puts("dos.library nyitási probléma!");
		CleanUp();
		exit( FALSE );
	}

	DiskfontBase = OpenLibrary("diskfont.library",0L);
	if( !DiskfontBase ){
		puts("diskfont.library nyitási probléma!");
		CleanUp();
		exit( FALSE );
	}

	PPBase = (struct PPBase *)OpenLibrary("powerpacker.library",0L);
	if( !PPBase ){
		puts("powerpacker.library nyitási probléma!");
		CleanUp();
		exit( FALSE );
	}

#ifdef TEST
printf("DiskfontBase : %lx\n",DiskfontBase );
#endif

/*  PAL üzemmód tesztelése  */

	TestWindow = OpenWindow( &testwindow );
	if( !TestWindow ){
		puts("TestWindow nyitási probléma");
		CleanUp();
		exit( FALSE );
	}

	Lines = TestWindow->WScreen->Height;
	CloseWindow( TestWindow );

	if( Lines == 200 ){
		puts( "Sajnos a géped NTSC üzemmódban indult el, indítsd ujra a gépet!");
		CleanUp();
		exit( FALSE );
	}

}

OpenLogo()
{
	if( !( LogoScreen = LoadIFF( "Logo.PIC" )) ){
		puts("Logo nyitási probléma!");
		CleanUp();
		exit( FALSE );
	}
	else
		LogoWindow = LogoScreen->FirstWindow;

	ScreenToFront( LogoScreen );
	ScreenLongUp( LogoScreen );
}

/*   Csak az OpenLogo után lehet meghívni   */

OpenScreensAndWindows()
{

/*  fontok megnyitasa */

	textattr.ta_Name = (UBYTE *)"GuruFont.font";
	textattr.ta_YSize = 11;
	textattr.ta_Style = FS_NORMAL;
	textattr.ta_Flags = FPF_DISKFONT|FPF_TALLDOT;
	HeaderFont = (struct TextFont *)OpenDiskFont(&textattr);

#ifdef TEST
printf("HeaderFont, a OpenScreens után : %lx\n",HeaderFont );
#endif

	if(HeaderFont == NULL){
		puts("GuruFont/11 nyitási probléma!");
		CleanUp();
		exit( FALSE );
	}

	textattr.ta_Name = (UBYTE *)"GuruFont.font";
	textattr.ta_YSize = 9;
	textattr.ta_Style = FS_NORMAL;
	textattr.ta_Flags = FPF_DISKFONT|FPF_TALLDOT;
	NormalFont = (struct TextFont *)OpenDiskFont(&textattr);

#ifdef TEST
printf("NormalFont, a OpenScreens elott : %lx\n" , NormalFont );
#endif

	if(NormalFont == NULL){
		puts("GuruFont/9 nyitási probléma!");
		CleanUp();
		exit( FALSE );
	}

	OpenScreens();
	/* ScreenToFront( LogoScreen ); */
	/* ScreenToFront( TopScreen  ); */
	/* ScreenToFront( BottomScreen ); */
}

OpenScreens()
{
	if( !( TopScreen = OpenScreen( &topscreen )) ){
		puts("TopScreen nyitási probléma!");
		CleanUp();
		exit(FALSE);
	} 
	topwindow.Screen=TopScreen;
	middlewindow.Screen=TopScreen;

	if( !(BottomScreen = OpenScreen(&bottomscreen)) ){
		puts("BottomScreen nyitási probléma!");
		CleanUp();
		exit(FALSE);
	}
	bottomwindow.Screen=BottomScreen;

	if( !(TopWindow = OpenWindow(&topwindow)) ){
		puts("TopWindow nyitási probléma!");
		CleanUp();
		exit(FALSE);
	} 
	if( !(MiddleWindow = OpenWindow(&middlewindow)) ){
		puts("MiddleWindow nyitási probléma!");
		CleanUp();
		exit(FALSE);
	} 
	if( !(BottomWindow = OpenWindow(&bottomwindow)) ){
		puts("BottomWindow nyitási probléma!");
		CleanUp();
		exit(FALSE);
	}
	TopRast		= TopWindow		-> RPort;
	MiddleRast	= MiddleWindow	-> RPort;
	BottomRast	= BottomWindow	-> RPort;

	SetFont( TopRast    ,HeaderFont );
	SetFont( MiddleRast ,NormalFont );
	SetFont( BottomRast ,NormalFont );

	DrawTopBorder();
	DrawBottomBorder();

	RefreshGadgets( &Gadget1,BottomWindow,NULL );

	SetColor( Color );
	ScreenToFront( LogoScreen );
	/* ScreenToFront( TopScreen ); */
	/* ScreenToFront( BottomScreen ); */
}

/**********************************************************

InitIcon()
A 8 alsó Gadget-hez tartozó ikon IMAGE strukturáját kitölti.

***********************************************************/

InitIcon()
{
	BYTE i;

	for( i=0; i<MAXICON ;i++ ){
		icon[i].LeftEdge  = 0;		/* IconX[i]; */
		icon[i].TopEdge   = 0;		/* IconY[i]; */
		icon[i].Width	  = IconW[i];
		icon[i].Height	  = IconH[i];
		icon[i].Depth	  = 3;
		icon[i].ImageData = &id[ IconOffset[i]/2 ];
		icon[i].PlanePick = 7;
		icon[i].PlaneOnOff= 0;
		icon[i].NextImage = NULL;
	}
	/* a két felsõ nyíl */

	icon[19].LeftEdge = IconX[19];
	icon[19].TopEdge  = IconY[19];
	icon[20].LeftEdge = IconX[20];
	icon[20].TopEdge  = IconY[20];

}
