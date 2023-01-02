#include <exec/types.h>
#include "graphics/text.h"
#include <intuition/intuition.h>
#include <functions.h>

#define	C_RETURN	0x0a
#define	C_NORM		0x01
#define	C_HIGH		0x02

extern	struct IntuitionBase	*IntuitionBase;
extern	struct GfxBase 			*GfxBase;
extern	struct DosBase			*DosBase;

extern	struct Screen	*TopScreen, *BottomScreen, *LogoScreen;
extern	struct Window	*TopWindow, *MiddleWindow, *BottomWindow, *LogoWindow;
extern	struct RastPort	*TopRast, *MiddleRast, *BottomRast;
extern	struct TextFont	*HeaderFont, *NormalFont;

struct		Window	*MessageWindow;

/***** message window *****************************************/


SHORT GadgetPairs[] =
   {
   0, 14, 0, 0, 66 , 0
   };

SHORT GadgetPairs2[] =
   {
   66, 1, 66, 14 , 1, 14
   };

struct Border NextBorder =
   {
   0, 0, 1, 0, JAM1, 3, GadgetPairs2, NULL,
   };

struct Border ButtonBorder =
   {
   0, 0, 5, 0, JAM1, 3, GadgetPairs, &NextBorder,
   };

struct IntuiText TrueText =
   {
   5, 0, JAM1, 22, 4, NULL, (UBYTE *)"Oké" ,NULL,
   };

struct IntuiText FalseText =
   {
   5, 0, JAM1, 22, 4, NULL, (UBYTE *)"Nem" ,NULL,
   };


struct Gadget button2 = /* FALSE */
	{
	NULL,
	7, -17,			/* 2 képpontsor üresen marad */
	67, 15,
	GADGHCOMP|GRELBOTTOM,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&ButtonBorder,
	NULL,
	&FalseText,
	NULL,
	NULL,
	0,
	NULL,
	};

struct Gadget button1 = /* TRUE */
	{
	NULL,
	-7-67, -17,
	67, 15,
	GADGHCOMP|GRELBOTTOM|GRELRIGHT,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&ButtonBorder,
	NULL,
	&TrueText,
	NULL,
	NULL,
	1,
	NULL,
	};

struct NewWindow messagewindow =
	{
	0,
	0,
	0,
	0,
	0,
	0,
	GADGETUP|GADGETDOWN|RAWKEY,
	BORDERLESS|RMBTRAP|ACTIVATE,
	&button1,
	NULL,
	NULL,
	NULL,
	NULL,
	640,
	215,
	640,
	215,
	CUSTOMSCREEN
	};

/*********** message window vége ******************************/

struct AlertMessage
{
   SHORT LeftEdge;
   BYTE TopEdge;
   char AlertText[36];
   BYTE Flag;
};

#define NOEND 0xFF
#define END   0x00

struct AlertMessage GaborcaMessage[] =
{
	208, 24, "Kedves szerkesztõtársaimnak:       ", NOEND,
	208, 34, "----------------------------       ", NOEND,
	208, 44, "                                   ", NOEND,
	208, 54, "A GONDOS munkához IDÕ kell!!       ", NOEND,
	208, 64, "                                   ", NOEND,
	208, 74, "                            Gaborca", END
};

/***************************************************************/

DrawMyBorder( rp, x,y , w,h )
struct	RastPort *rp;
UWORD	x,y,w,h;
{
	UWORD p1,p2;

	p1 = x+w -1;
	p2 = y+h -1;

/* sötétebb rész */

	SetAPen( rp,1 );
	Move( rp, x  , p2 );
	Draw( rp, p1 , p2 );
	Draw( rp, p1 , y  );

/* világosabb rész */

	SetAPen( rp, 5 );
	Draw( rp, x , y  );
	Draw( rp, x , p2 );
}

linelen( s )
char *s;
{
	int	n = 0;
	while( *s != '\n' ) n++;
	return( n );
}

BOOL ShowMessage( title , text , buttons , About ,Return )
char	*title;
register	char	*text;
BOOL	buttons,About,Return;
{

	UWORD		MaxW,Width,Height,TextW,TextH,TitlePosX,TitleLength;
	long		lines;
	register	unsigned	pos;
	register	UWORD		TextLength,screenline,LineStart;

	struct		RastPort	*rp;

	struct		IntuiMessage	*message = NULL;
	ULONG		MessageClass;
	USHORT		code,key1;
	int			ActGadget;

	UBYTE	WriteColor;


	TitleLength = strlen( title );

	MaxW		= TitleLength;
	lines		= 0;
	LineStart	= 0;

	TextLength = strlen( text );
	for( pos =0 ; pos<=TextLength ; pos ++){

		if( (text[ pos ] == C_RETURN) || (text[ pos ] == '\0') ){
			lines += 1;
			if( (pos-LineStart) > MaxW )
				MaxW=pos-LineStart;

			if( (text[pos+1] == '\1') || (text[pos+1] == '\2') )
				LineStart = pos+2;
			else
				LineStart = pos+1;
		}
	}
	TextW = MaxW*8+16;
	TextH = 14 + lines * 11;

	Width  = 8  + TextW +  8;
	Height = 20 + TextH + 20;

	TitleLength = strlen( title );
	TitlePosX = 16 + ( MaxW-TitleLength )*4;  /*  *8/2   */
	

	LineStart  =  0;
	screenline = 28+7;

	WriteColor = 6;
	LineStart  = 0;
	
	messagewindow.Width		= Width;
	messagewindow.Height	= Height;
	messagewindow.LeftEdge	= (640-Width)/2;
	messagewindow.TopEdge	= (200-Height)/2;
	messagewindow.Screen	= TopScreen;

	if( buttons )
		button1.NextGadget = &button2;
	else
		button1.NextGadget = NULL;

	WaitTOF();
	if( !(MessageWindow = OpenWindow( &messagewindow ) ))
		return( FALSE );

	rp = MessageWindow->RPort;
	SetFont( rp , NormalFont );
	SetBPen( rp , 2 );

	SetDrMd( rp, JAM2 );
	Move( rp, 0,0 );
	ClearScreen( rp );
	SetDrMd( rp, JAM1 );

	DrawMyBorder( rp, 0, 0, Width	,Height );			/* Ablak keret  */
	DrawMyBorder( rp, 7, 4, TextW	,15     );			/* Cím keret    */
	DrawMyBorder( rp, 7,20, TextW	,TextH  );			/* Szöveg keret */

	SetAPen( rp, 1 );
	Move( rp,TitlePosX+2,8+7+2 );
	Text( rp, title , TitleLength );

	SetAPen( rp, 7 );
	Move( rp,TitlePosX,8+7);
	Text( rp, title , TitleLength );

	for( pos=0 ; pos <= TextLength ; pos++ ){
		switch( text[pos] ){

		case C_RETURN:
		case '\0':
				SetAPen( rp, 1 );
				Move( rp, 16+2, screenline+2 );
				Text( rp, &text[LineStart], pos-LineStart );
				SetAPen( rp, WriteColor );
				Move( rp, 16, screenline );
				Text( rp, &text[LineStart], pos-LineStart );

				LineStart = pos+1;
				screenline += 11;
				break;

		case C_NORM:
				WriteColor = 6;
				LineStart += 1;
				break;

		case C_HIGH:
				WriteColor = 7;
				LineStart += 1;
				break;
		}
	}

	RefreshGadgets( &button1 , MessageWindow , NULL );

	if( Return )
		return;

	for(;;){

		if(message = (struct IntuiMessage *)GetMsg(MessageWindow->UserPort)){
			MessageClass = message->Class;
			code = message->Code;
			ReplyMsg(message);

			if( (MessageClass == GADGETUP) || (MessageClass == GADGETDOWN) ){
				ActGadget = ( (struct Gadget *)(message->IAddress) )->GadgetID;
				CloseWindow( MessageWindow );
				ActivateWindow( LogoWindow );	/* kurzor miatt */
				if( ActGadget == 1 )
					return( TRUE );
				return( FALSE );
			}
			if( (MessageClass == RAWKEY) && About ){
				if( key1 != 96 )
					key1 = code;
				else
					if( code == 95 ){
						key1 = 0;
						DisplayAlert(RECOVERY_ALERT, &GaborcaMessage, 90L);
					}
			}
		}
	}
}

/******************************************************
	CheckMessageWindow()
	Csak akkor hívható ha nyitva van A MessageWindow!!!

	Megnézi aktiválták-e a Gadget-et, ha igen akkor
	TRUE -t ad vissza különben FALSE -t.

********************************************************/

BOOL CheckMessageWindow()
{

	struct		IntuiMessage	*message = NULL;
	ULONG		MessageClass;
	USHORT		code;

	if(message = (struct IntuiMessage *)GetMsg(MessageWindow->UserPort)){
		MessageClass = message->Class;
		code = message->Code;
		ReplyMsg(message);

		if( (MessageClass == GADGETUP) || (MessageClass == GADGETDOWN) )
			return( TRUE );
	}
	return( FALSE );
}

CloseMessageWindow()
{
			CloseWindow( MessageWindow );
			WBenchToBack();
			ActivateWindow( LogoWindow );	/* kurzor miatt */
}
