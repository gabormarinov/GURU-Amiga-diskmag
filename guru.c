/*****************************************************************

Program:	guru.c

			A GURU lemezújság fõmodulja.
			
Szerzõ:		Marinov Gábor	<<Gaborca>>

Dátum:		1990.07.29.

Utolsó
módosítás:	1990.09.14

******************************************************************/


#include <stdio.h>
#include <intuition/intuition.h>
#include <exec/tasks.h>
#include <functions.h>

extern	struct Window	*MiddleWindow, *BottomWindow, *LogoWindow;
extern  struct Screen	*TopScreen, *BottomScreen, *LogoScreen, *ListScreen;

extern	BOOL	TopUp;
extern	UWORD	ColorMap[6][8];
UWORD	Color;


/****************  Music.c **************/

extern	BOOL	LoadAndPlayMusic();
extern	void	PauseMusic();

extern	BOOL	PlayMusic;
extern	BOOL	IsMusicInMemory;

/****************************************/

extern	BOOL	LoadFile();
extern	long	 ContSize,FileSize;
extern	char	*ContAddr,*FileAddr;

BOOL	QPage	= FALSE;
#define MAXCOLOR (6-1)

struct	Task	*InputTask = NULL;
struct	Task	*GuruTask = NULL;

extern struct Gadget Gadget7;
extern struct Gadget Gadget8;
extern struct Image  icon[];

#define EMPTYPOINTER 0
#define ARROWPOINTER 1
#define CLOCKPOINTER 2


GetGadget()
{
	struct	IntuiMessage	*message = NULL;

	ULONG  MessageClass;
	USHORT code;

	int ActGadget;

	for(;;){

		/* Az ikonok figyelése */
		if(message = (struct IntuiMessage *)GetMsg(BottomWindow->UserPort)){
			MessageClass = message->Class;
			code = message->Code;
			ReplyMsg(message);

			if( (MessageClass == GADGETUP) || (MessageClass == GADGETDOWN) ){
				ActGadget = ( (struct Gadget *)(message->IAddress) )->GadgetID;
				return( ActGadget );
			}
		}

		/* A 22 menüpont figyelése */
		if(message = (struct IntuiMessage *)GetMsg(MiddleWindow->UserPort)){
			MessageClass = message->Class;
			code = message->Code;
			ReplyMsg(message);

			if ( (MessageClass == GADGETUP) || (MessageClass == GADGETDOWN) ){
				ActGadget = ( (struct Gadget *)(message->IAddress) )->GadgetID;
				return( ActGadget );
			}
		}
	}
}

Preferences( ActGadget )
int ActGadget;
{
	UWORD OldColor;
	register int i,k;
	UWORD tmpcolor[8];

	switch( ActGadget ){

			case 6:		/*  szín beállítás  */
					OldColor = Color;
					Color += 1;
					if(Color > MAXCOLOR)
						Color = 0;

					for(i=0;i<8;i++)
						tmpcolor[i]=ColorMap[OldColor][i];

					for(k=1;k<16;k++){
						for(i=0;i<8;i++){
							if( (tmpcolor[i]&0x000f) != 0 )
								tmpcolor[i] -= 0x0001;

							if( (tmpcolor[i]&0x00f0) != 0 )
								tmpcolor[i] -= 0x0010;

							if( (tmpcolor[i]&0x0f00) != 0 )
								tmpcolor[i] -= 0x0100;
						}
						WaitTOF();
						LoadRGB4(&TopScreen   ->ViewPort,&tmpcolor[0],8 );
						LoadRGB4(&BottomScreen->ViewPort,&tmpcolor[0],8 );
						if( ListScreen ){
							SetRGB4( &ListScreen->ViewPort, 0, (tmpcolor[0]>>8)& 0xf, (tmpcolor[0]>>4) & 0xf, tmpcolor[0] & 0xf);
							SetRGB4( &ListScreen->ViewPort, 1, (tmpcolor[6]>>8)& 0xf, (tmpcolor[6]>>4) & 0xf, tmpcolor[6] & 0xf);
						}

					}

					for(k=1;k<16;k++){
						for(i=0;i<8;i++){
							if( (tmpcolor[i]&0x000f) != (ColorMap[Color][i]&0x000f) )
								tmpcolor[i] += 0x0001;

							if( (tmpcolor[i]&0x00f0) != (ColorMap[Color][i]&0x00f0) )
								tmpcolor[i] += 0x0010;

							if( (tmpcolor[i]&0x0f00) != (ColorMap[Color][i]&0x0f00) )
								tmpcolor[i] += 0x0100;
						}
						WaitTOF();
						LoadRGB4(&TopScreen   ->ViewPort,&tmpcolor[0],8 );
						LoadRGB4(&BottomScreen->ViewPort,&tmpcolor[0],8 );
						if( ListScreen ){
							SetRGB4( &ListScreen->ViewPort, 0, (tmpcolor[0]>>8)& 0xf, (tmpcolor[0]>>4) & 0xf, tmpcolor[0] & 0xf);
							SetRGB4( &ListScreen->ViewPort, 1, (tmpcolor[6]>>8)& 0xf, (tmpcolor[6]>>4) & 0xf, tmpcolor[6] & 0xf);
						}
					}
					break;


			case 7:		/*  Gyorslapozás  */
					if(QPage){
						QPage = FALSE;
						Gadget7.GadgetRender=(APTR)&icon[7];
					}
					else{
						QPage = TRUE;
						Gadget7.GadgetRender=(APTR)&icon[16];
					}
					RefreshGadgets( &Gadget7,BottomWindow,NULL );
					break;


			case 8:		/*  Zene  */
				if( IsMusicInMemory ){

					if(PlayMusic){
						PlayMusic = FALSE;
						Gadget8.GadgetRender=(APTR)&icon[17];
					}
					else{
						PlayMusic = TRUE;
						Gadget8.GadgetRender=(APTR)&icon[8];
					}
					RefreshGadgets( &Gadget8,BottomWindow,NULL );
					PauseMusic();
					break;
				}
	}

}

Warning( Message )
char *Message;
{
	if( !TopScreen )
		puts( Message );
	else{
		if( TopUp )
			ShowMessage( "Valami szörnyû történt !" , Message , FALSE ,FALSE, FALSE );
		else{
			TopShortUp();
			ShowMessage( "Valami szörnyû történt !" , Message , FALSE ,FALSE, FALSE );
			TopShortDown();
		}
	}
}

UWORD atouw( s )
char s[];
{
	UWORD n,i;

	for(i=0;s[i]==' ' || s[i]=='\n' || s[i]=='\t';i++ );

	for( n=0; s[i]>='0' && s[i]<='9' ; i++ )
		n = 10*n +s[i]-'0';

	return( n );
}

main( argc,argv )
int argc;
char *argv[];
{
	BYTE i;
	UWORD ActPage = 1;
	UWORD Color   = 0;

	if( argc!=4 ){
		puts("Illegális hívás - Igy nem indul a GURU!\n");
		exit( FALSE );
	}
	ActPage   = atouw( argv[1] ); 
	Color	  = atouw( argv[2] );

	GuruTask  = FindTask( NULL );
	InputTask = FindTask( "input.device" );

#ifdef TEST
puts("OpenLibs() elött");
#endif

	OpenLibs();

#ifdef TEST
puts("OpenLibs() után, InitIcon() elött");
#endif

	InitIcon();

#ifdef TEST
puts("InitIcon() után, tartalom elött");
#endif

/*
  A tartalomjegyzéket és a zenét a TopScreen,BottomScreen  megnyitása
  elõtt kell betölteni, mert különben a képernyõk bezárásakor nem
  lenne folyamatos a szabad memória.
*/

	if( !LoadFile( "PROGRAM:Tartalom",0 ) )
		exit( FALSE );
	ContAddr = FileAddr;
	ContSize = FileSize;

#ifdef TEST
puts("tartalom után, OpenLogo() elött");
#endif

	OpenLogo();
	ScreenLongUp( LogoScreen );

#ifdef TEST
puts("OpenLogo() után, zene elött");
#endif

/* Zene indítása */

	if( !strcmp( argv[3] , "NEM" )){
		PlayMusic = FALSE;
		IsMusicInMemory = FALSE;
		Gadget8.GadgetRender=(APTR)&icon[17];
	}
	else
		IsMusicInMemory = LoadAndPlayMusic( argv[3] );

	OpenScreensAndWindows();
	Menu( ActPage );

	CleanUp();
}
