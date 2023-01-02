#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfxbase.h>
#include <intuition/intuition.h>

#include <functions.h>

struct Screen			*ListScreen		= NULL;
struct Window			*ListWindow		= NULL;
struct RasInfo			*ListRasInfo	= NULL;
struct ViewPort			*ListView		= NULL;
struct RastPort			*ListRast		= NULL;

char	*ListAddr = NULL;
long	ListLength	= 0L;

#define T_LIST		5		/*  Programlista			*/

extern	struct TextFont		*NormalFont;

extern	struct	Screen		*BottomScreen;
extern	struct	Window		*BottomWindow;
extern	struct	RastPort	*MiddleRast;

extern struct Gadget Gadget1;
extern struct Gadget Gadget2;
extern struct Image  icon[];

extern	BOOL	LoadFile();
extern	char	*FileAddr;
extern	long	FileSize;
extern	BOOL	CountLines();

extern	UBYTE		*LineLength;
extern	unsigned	*LinePos;
extern	ULONG		Lines;
extern	char		*Date;

extern	UWORD	ColorMap[6][8];
extern	UWORD	Color;

#define VISLINES 17
#define	TOPLINE 0
#define BOTTOMLINE (2*VISLINES)
#define	FONTSIZE 9
#define	FONTOFFSET 7
#define NEXTLINE 10
#define SCROLL 10

#define	UP   TRUE
#define	DOWN FALSE

#define EMPTYPOINTER 0
#define ARROWPOINTER 1
#define CLOCKPOINTER 2

struct NewScreen listscreen =
{
	0,
	268,  /* 38 */
	640,
	360,
	1,
	1,
	0,
	HIRES,
	CUSTOMSCREEN,
	NULL,
	NULL,
	NULL,
	NULL,
};

struct NewWindow listwindow =
{
	0,
	0,
	640,
	360,
	0,
	0,
	NULL,
	BORDERLESS|RMBTRAP,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	640,
	360,
	640,
	360,
	CUSTOMSCREEN
};

/*
	Gaborca különbejáratú FASTFONT-ja : QuickFont
	CSAK 8 széles, és FONTSIZE magas betûkre!
*/

struct BitMap	QuickBitMap;
struct RastPort	QuickRast;

/*
	Egy 16x16 karakteres RasterPort-ot épít fel, ahol 0-255 állnak a karakterek
*/

BOOL InitQuickFont()
{
	PLANEPTR Plane;
	register UWORD	col,row;
	unsigned char	tmp;

	if( !(Plane = (PLANEPTR)AllocMem( 16*16*FONTSIZE , MEMF_CHIP|MEMF_CLEAR )))
		return( FALSE );

#ifdef TEST
printf( "Plane: %x\n",Plane );
#endif

	InitBitMap( &QuickBitMap , 1 , 16*8 , 16*FONTSIZE );
	QuickBitMap.Planes[0] = Plane;
	InitRastPort( &QuickRast );
	QuickRast.BitMap = &QuickBitMap;

#ifdef TEST
printf("BytesPerRow: %d\n",QuickBitMap.BytesPerRow);
#endif

	SetFont( &QuickRast, NormalFont );

	for( row=0;row<16;row++)
		for( col=0;col<16;col++){
			tmp = row*16+col;
			Move( &QuickRast, col*8 , row*FONTSIZE+FONTOFFSET );
			Text( &QuickRast, &tmp , 1 );
		}
	return( TRUE );
}

FreeQuickFont()
{
	FreeMem( QuickBitMap.Planes[0] , 16*16*FONTSIZE );
}

MyText( rp, text , length )
struct	RastPort *rp;
char	*text;
UWORD	length;
{
	#asm
	move.l	a0,-(a7)
	move.l	a1,-(a7)
	move.l	a2,-(a7)
	move.l	a3,-(a7)
	move.l	d0,-(a7)
	move.l	d1,-(a7)
	move.l	d2,-(a7)
	move.l	d3,-(a7)

	clr.l	d0
	move.w	18(a5),d0	;length -> d0

	cmp.w	#0,d0
	beq.s	end

	move.l	8(a5),a0	;rp -> a5

	clr.l	d3			; x pozició bállítása a RastPort-on
	move.w	d0,d3
	mulu	#8,d3
	move.w	d3,36(a0)

	clr.l	d2
	move.w	38(a0),d2	;d2: cél rp Y-poz
						; csak az Y-poziciót veszi figyelembe, x=0

	sub.l	#7,d2		; FONTOFFSET

	move.l	4(a0),a0	;Cél BitMap kezdõcíme
	move.l	8(a0),a0	;elsõ és egyetlen Cél bitplane
	mulu.w	#80,d2
	add.l	d2,a0		; a0: Cél bitplane, ahol a kiírás kezdõdik

	move.l	#_QuickBitMap,a1
	move.l	8(a1),a1	; QuickFont elsõ bitplane

	move.l	12(a5),a2	;text   -> a2

write:
	move.l	a1,a3		;QuickFont bitplane
	clr.l	d1
	clr.l	d3
	move.b	(a2)+,d1	; d1: karakter kód
	move.b	d1,d3

	lsr.b	#4,d1
	mulu	#144,d1		; (16*9) hányadik sor

	and.b	#$0f,d3		; soron belül hányadik
	add.l	d3,d1

	add.l	d1,a3		; a3: QuickFont kezdõcíme

	clr.l	d2
	move.l	#8,d3

char:
	move.b	(a3),(a0,d2.w)
	add.l	#16,a3
	add.w	#80,d2
	dbf		d3,char

	addq.l	#1,a0
	dbf	d0,write

end:
	move.l	(a7)+,d3
	move.l	(a7)+,d2
	move.l	(a7)+,d1
	move.l	(a7)+,d0
	move.l	(a7)+,a3
	move.l	(a7)+,a2
	move.l	(a7)+,a1
	move.l	(a7)+,a0
	#endasm
}

ListScreenUp()
{
	register int i;
	register int j=-NEXTLINE;

	Freeze();
	for( i=0;i<23; i++)
		MoveScreen( ListScreen , 0, j );
	UnFreeze();
}

ListScreenDown()
{
	register int i;
	register int j=NEXTLINE;

	Freeze();
	for( i=0;i<23; i++)
		MoveScreen( ListScreen , 0, j );
	UnFreeze();
}

ShowList( LoadName,ListName )
char *LoadName,*ListName;
{
	struct IntuiMessage	*message = NULL;
	ULONG	MessageClass;
	USHORT	code;
	int		i,ActGadget;

	BOOL	MoreScrollUp,MoreScrollDown,ScrollUp,ScrollDown;	
	BOOL	Last;

	register	WORD	ActMemLine,ActWriteLine,ScrPos;
	register	WORD	Accel;
	UWORD	tmpcolor;
	WORD	tmppos,tmpline;
	WORD	Line[ VISLINES*2 +1 ];


	if(!InitQuickFont()){
		Warning( "Quickfont installálási hiba!");
		return;
	}

	if(!LoadFile( LoadName , T_LIST )){
		FreeQuickFont();
		return;
	}
	
	ListAddr   = FileAddr;
	ListLength = FileSize;

	if(!CountLines( ListAddr , ListLength )){
		UnLoadFile( ListAddr , ListLength );
		FreeQuickFont();
		return(FALSE);
	}

	WBenchToBack();		/* csak más képernyõ ne legyen nyitva ! */

	if(!( ListScreen = OpenScreen( &listscreen )) ){
		UnLoadFile( ListAddr , ListLength );
		FreeQuickFont();
		FreeMem( LineLength , Lines * sizeof(UBYTE));
		FreeMem( LinePos , Lines * sizeof(unsigned));

		Warning( "Nincs elég memória a ListScreen megnyitásához!" );
		return;
	}
	listwindow.Screen = ListScreen;

#ifdef TEST
printf( "ListScreen -- > planes[0] : %x\n",ListScreen->BitMap.Planes[0] );
#endif

	if(!( ListWindow = OpenWindow( &listwindow )) ){
		UnLoadFile( ListAddr , ListLength );
		FreeQuickFont();
		FreeMem( LineLength , Lines * sizeof(UBYTE));
		FreeMem( LinePos , Lines * sizeof(unsigned));
		CloseScreen( ListScreen );

		Warning( "Nincs elég memória a ListWindow megnyitásához!" );
		return;
	}
	ListRast    = &ListScreen->RastPort;
	ListView    = &ListScreen->ViewPort;
	ListRasInfo =  ListScreen->ViewPort.RasInfo;

	ScreenToFront( BottomScreen );

/*  TopScreen beállítása */

	Move( MiddleRast , 0,0 );
	ClearScreen( MiddleRast );
	DeleteIcon( 100 );
	DeleteIcon( 101 );
	WriteHeader( ListName );

/* ListScreen beállítása  */

	Move( ListWindow->RPort , 0,0 );		/* Ha a ListRast -ot törlöm akkor hibásan mûködik */
	ClearScreen( ListWindow->RPort );	/* A Bitplane elõtt lévõ memóriaterületbõl is töröl */

 	SetFont( ListRast, NormalFont );

/* lecseréli az elsõ két ikont, módosítja a FLAG-eket  */

	Gadget1.GadgetRender = (APTR)&icon[3];
	Gadget1.SelectRender = (APTR)&icon[12];
	Gadget1.Activation   = GADGIMMEDIATE|RELVERIFY;

	Gadget2.GadgetRender = (APTR)&icon[4];
	Gadget2.SelectRender = (APTR)&icon[13];
	Gadget2.Activation   = GADGIMMEDIATE|RELVERIFY;

	RefreshGadgets( &Gadget1,BottomWindow,NULL );


/* Színek beállítása */
	tmpcolor = ColorMap[Color][0];
	SetRGB4( ListView, 0, (tmpcolor>>8)& 0xf, (tmpcolor>>4) & 0xf, tmpcolor & 0xf);
	tmpcolor = ColorMap[Color][6];
	SetRGB4( ListView, 1, (tmpcolor>>8)& 0xf, (tmpcolor>>4) & 0xf, tmpcolor & 0xf);



/*  scroll inicializálás  */

	ScrollUp = ScrollDown = FALSE;
	MoreScrollDown = TRUE;
	MoreScrollUp   = FALSE;
	Last = DOWN;

	for( ActMemLine =0,ActWriteLine=0 ; ActWriteLine<=VISLINES ; ActWriteLine++,ActMemLine++){

		if( ActMemLine < Lines ){
			Move( ListRast, 0 , ActWriteLine*NEXTLINE+FONTOFFSET );
			MyText( ListRast, &ListAddr[ LinePos[ ActMemLine ] ] , LineLength[ ActMemLine ] );
			ClearEOL( ListRast );
			Line[ ActWriteLine ] = ActMemLine;
		}
		else{
			MoreScrollUp   = FALSE;
			MoreScrollDown = FALSE;
		}
	}

	TopShortUp();
	ListScreenUp();

	Accel  = 0;
	ScrPos = 0;

	for(;;){

		if( ScrollUp && MoreScrollUp ){

			if( Accel < VISLINES+VISLINES/2 )
				Accel += 1;

			MoreScrollDown = TRUE;
			if( ScrPos == 1 ){

				if( Accel < VISLINES+VISLINES/2 ){

					ListRasInfo->RyOffset -= SCROLL/5;
					ScrollVPort( ListView );
					WaitTOF();

					ListRasInfo->RyOffset -= SCROLL/5;
					ScrollVPort( ListView );
					WaitTOF();

					ListRasInfo->RyOffset -= SCROLL/5;
					ScrollVPort( ListView );
					WaitTOF();

					ListRasInfo->RyOffset -= SCROLL/5;
					ScrollVPort( ListView );
					WaitTOF();
				}

				ListRasInfo->RyOffset = (VISLINES+1)*NEXTLINE;
				ScrollVPort( ListView );
				WaitTOF();

				ScrPos       = VISLINES+1;
				ActWriteLine = VISLINES;

				if( ActMemLine == -1 )
					MoreScrollUp = FALSE;
				else{

					Move( ListRast, 0 , ActWriteLine*NEXTLINE+FONTOFFSET );
					MyText( ListRast, &ListAddr[ LinePos[ ActMemLine ] ], LineLength[ ActMemLine ] );
					ClearEOL( ListRast );

					Line[ ActWriteLine ] = ActMemLine;

					ActMemLine   -= 1;
					ActWriteLine -= 1;
				}
			}
			else{
				if( Accel < VISLINES+VISLINES/2 ){

					ListRasInfo->RyOffset -= SCROLL/5;
					ScrollVPort( ListView );
					WaitTOF();

					ListRasInfo->RyOffset -= SCROLL/5;
					ScrollVPort( ListView );
					WaitTOF();

					ListRasInfo->RyOffset -= SCROLL/5;
					ScrollVPort( ListView );
					WaitTOF();

					ListRasInfo->RyOffset -= SCROLL/5;
					ScrollVPort( ListView );
					WaitTOF();

					ListRasInfo->RyOffset -= SCROLL/5;
					ScrollVPort( ListView );
					WaitTOF();
				}
				else{
					ListRasInfo->RyOffset -= SCROLL;
					ScrollVPort( ListView );
					WaitTOF();
				}

				if( ActMemLine == -1 ){
					MoreScrollUp = FALSE;
					ScrPos -= 1;
				}
				else{
					Move( ListRast, 0 , ActWriteLine*NEXTLINE+FONTOFFSET );
					MyText( ListRast, &ListAddr[ LinePos[ ActMemLine ] ], LineLength[ ActMemLine ] );
					ClearEOL( ListRast );
					Line[ ActWriteLine ] = ActMemLine;

					ClipBlit( ListRast, 0, ActWriteLine*NEXTLINE, ListRast, 0 , ( ActWriteLine+VISLINES+1 )*NEXTLINE , 640 , NEXTLINE , 0xc0 );
					Line[ ActWriteLine+VISLINES+1 ] = ActMemLine;

					ScrPos       -= 1;
					ActWriteLine -= 1;
					ActMemLine   -= 1;
				}
			}
		}

		if( ScrollDown && MoreScrollDown ){

			if( Accel < VISLINES+VISLINES/2 )
				Accel += 1;

			MoreScrollUp = TRUE;

			if(ScrPos == VISLINES){

				if( Accel < VISLINES+VISLINES/2 ){
					ListRasInfo->RyOffset += SCROLL/5;
					ScrollVPort( ListView );
					WaitTOF();

					ListRasInfo->RyOffset += SCROLL/5;
					ScrollVPort( ListView );
					WaitTOF();

					ListRasInfo->RyOffset += SCROLL/5;
					ScrollVPort( ListView );
					WaitTOF();

					ListRasInfo->RyOffset += SCROLL/5;
					ScrollVPort( ListView );
					WaitTOF();
				}

				ListRasInfo->RyOffset = TOPLINE;
				ScrollVPort( ListView );
				WaitTOF();

				ScrPos = 0;
				ActWriteLine = VISLINES;

				if( ActMemLine == Lines )
					MoreScrollDown = FALSE;
				else{

					Move( ListRast, 0 , ActWriteLine*NEXTLINE+FONTOFFSET );
					MyText( ListRast, &ListAddr[ LinePos[ ActMemLine ] ], LineLength[ ActMemLine ] );
					ClearEOL( ListRast );

					Line[ ActWriteLine ] = ActMemLine;

					ActWriteLine += 1;
					ActMemLine   += 1;
				}
			}
			else{        /* ScrPos != VISLINES */		

				if( Accel < VISLINES+VISLINES/2 ){

					ListRasInfo->RyOffset += SCROLL/5;
					ScrollVPort( ListView );
					WaitTOF();

					ListRasInfo->RyOffset += SCROLL/5;
					ScrollVPort( ListView );
					WaitTOF();

					ListRasInfo->RyOffset += SCROLL/5;
					ScrollVPort( ListView );
					WaitTOF();

					ListRasInfo->RyOffset += SCROLL/5;
					ScrollVPort( ListView );
					WaitTOF();

					ListRasInfo->RyOffset += SCROLL/5;
					ScrollVPort( ListView );
					WaitTOF();
				}
				else{
					ListRasInfo->RyOffset += SCROLL;
					ScrollVPort( ListView );
					WaitTOF();
				}

				if( ActMemLine == Lines ){
					MoreScrollDown = FALSE;
					ScrPos += 1;
				}
				else{

					Move( ListRast, 0 , ActWriteLine*NEXTLINE+FONTOFFSET );
					MyText( ListRast, &ListAddr[ LinePos[ ActMemLine ] ], LineLength[ ActMemLine ] );
					ClearEOL( ListRast );
					Line[ ActWriteLine ] = ActMemLine;

					ClipBlit( ListRast, 0, ActWriteLine*NEXTLINE, ListRast, 0 , ScrPos*NEXTLINE , 640 , NEXTLINE , 0xc0 );
					Line[ ScrPos ] = ActMemLine;

					ScrPos       += 1;
					ActWriteLine += 1;
					ActMemLine   += 1;
				}
			}
		}
		if (message = (struct IntuiMessage *)GetMsg(BottomWindow->UserPort)){
			MessageClass = message->Class;
			code = message->Code;
			ReplyMsg(message);
			ActGadget = ( (struct Gadget *)(message->IAddress) )->GadgetID;

			switch( ActGadget ){

				case 1: /*  Lefelé, ez a képernyõn felfelé  */
					if( !ScrollDown ){

						Accel = 0;
						if( (Last == UP) && MoreScrollDown ){   /* FEL -> LE forduló */

							tmpline = Line[ScrPos]-ScrPos;
							tmpline += VISLINES+1;
							for( tmppos=TOPLINE; tmppos < ScrPos ; tmppos++,tmpline++ ){

								Move( ListRast, 0 , tmppos*NEXTLINE+FONTOFFSET );
								MyText( ListRast, &ListAddr[ LinePos[ tmpline ] ], LineLength[ tmpline ] );
								ClearEOL( ListRast );

								Line[ tmppos ] = tmpline;
							}
							if( ScrPos != (VISLINES+1) ){

								tmppos  = ScrPos+VISLINES;
								tmpline = Line[tmppos-1]+1;
	
								Move( ListRast, 0 , tmppos*NEXTLINE+FONTOFFSET );
								MyText( ListRast, &ListAddr[ LinePos[ tmpline ] ], LineLength[ tmpline ] );
								ClearEOL( ListRast );

								Line[ tmppos ] = tmpline;

								ActWriteLine = tmppos +1;
								ActMemLine   = Line[tmppos]+1;
							}
							else{
								ScrPos = TOPLINE;

								ListRasInfo->RyOffset = TOPLINE;
								ScrollVPort( ListView );
								WaitTOF();

								ActMemLine   = tmpline;
								ActWriteLine = tmppos;
							}
							Last = DOWN;
							MoreScrollUp = TRUE;
							MoreScrollDown = TRUE;
						}
						ScrollDown = TRUE;
						ScrollUp   = FALSE;
					}
					else{
						ScrollDown = FALSE;
						ScrollUp   = FALSE;
					}
					break;

				case 2:
					if( !ScrollUp ){

						Accel = 0;
						if( (Last == DOWN) && MoreScrollUp ){   /* LE -> FEL forduló */

							tmpline = Line[ScrPos]-ScrPos;
							tmpline += VISLINES-1;
							tmppos  = BOTTOMLINE;

							for( tmppos=BOTTOMLINE; tmppos >= ScrPos+VISLINES ; tmppos--,tmpline-- ){

								Move( ListRast, 0 , tmppos*NEXTLINE+FONTOFFSET );
								MyText( ListRast, &ListAddr[ LinePos[ tmpline ] ], LineLength[ tmpline ] );
								ClearEOL( ListRast );

								Line[ tmppos ] = tmpline;
							}

							if( ScrPos != TOPLINE ){
								tmpline = Line[ScrPos]-1;
								tmppos  = ScrPos -1;

								Move( ListRast, 0 , tmppos*NEXTLINE+FONTOFFSET );
								MyText( ListRast, &ListAddr[ LinePos[ tmpline ] ], LineLength[ tmpline ] );
								ClearEOL( ListRast );

								Line[ tmppos ] = tmpline;
	
								ActWriteLine = tmppos-1;
								ActMemLine   = Line[tmppos]-1;
							}
							else{
								ScrPos = VISLINES+1;

								ListRasInfo->RyOffset = (VISLINES+1)*NEXTLINE;
								ScrollVPort( ListView );
								WaitTOF();

								ActMemLine    = tmpline;
								ActWriteLine  = tmppos;
							}
							Last	       = UP;
							MoreScrollDown = TRUE;
							MoreScrollUp   = TRUE;
						}
						ScrollUp   = TRUE;
						ScrollDown = FALSE;
					}
					else{
						ScrollUp   = FALSE;
						ScrollDown = FALSE;
					}
					break;

				case 3:

					ListScreenDown();
					TopShortDown();

					Gadget1.GadgetRender = (APTR)&icon[0];
					Gadget1.SelectRender = (APTR)&icon[9];
					Gadget1.Activation   = RELVERIFY;

					Gadget2.GadgetRender = (APTR)&icon[1];
					Gadget2.SelectRender = (APTR)&icon[10];
					Gadget2.Activation   = RELVERIFY;

					RefreshGadgets( &Gadget1,BottomWindow,NULL );

					FreeQuickFont();
					UnLoadFile( ListAddr , ListLength );
					FreeMem( LineLength , Lines * sizeof(UBYTE));
					FreeMem( LinePos , Lines * sizeof(unsigned));
					CloseWindow( ListWindow );
					CloseScreen( ListScreen );
					ListScreen = NULL;
					return;
					break;

				case 4:
					MoveScreen( ListScreen, 0 ,  256 );
					Print( Date, ListName, ListAddr, T_LIST );
					MoveScreen( ListScreen, 0 , -256 );
					break;

				default:
					Preferences( ActGadget );
					break;		
			}
			           
		}
	}
}
