/*****************************************************************

Program:	guru.c

			A GURU lemez�js�g f�modulja.
			
Szerz�:		Marinov G�bor	<<Gaborca>>

D�tum:		1990.07.29.

Utols�
m�dos�t�s:	1990.09.14

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

		/* Az ikonok figyel�se */
		if(message = (struct IntuiMessage *)GetMsg(BottomWindow->UserPort)){
			MessageClass = message->Class;
			code = message->Code;
			ReplyMsg(message);

			if( (MessageClass == GADGETUP) || (MessageClass == GADGETDOWN) ){
				ActGadget = ( (struct Gadget *)(message->IAddress) )->GadgetID;
				return( ActGadget );
			}
		}

		/* A 22 men�pont figyel�se */
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

			case 6:		/*  sz�n be�ll�t�s  */
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


			case 7:		/*  Gyorslapoz�s  */
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
			ShowMessage( "Valami sz�rny� t�rt�nt !" , Message , FALSE ,FALSE, FALSE );
		else{
			TopShortUp();
			ShowMessage( "Valami sz�rny� t�rt�nt !" , Message , FALSE ,FALSE, FALSE );
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
		puts("Illeg�lis h�v�s - Igy nem indul a GURU!\n");
		exit( FALSE );
	}
	ActPage   = atouw( argv[1] ); 
	Color	  = atouw( argv[2] );

	GuruTask  = FindTask( NULL );
	InputTask = FindTask( "input.device" );

#ifdef TEST
puts("OpenLibs() el�tt");
#endif

	OpenLibs();

#ifdef TEST
puts("OpenLibs() ut�n, InitIcon() el�tt");
#endif

	InitIcon();

#ifdef TEST
puts("InitIcon() ut�n, tartalom el�tt");
#endif

/*
  A tartalomjegyz�ket �s a zen�t a TopScreen,BottomScreen  megnyit�sa
  el�tt kell bet�lteni, mert k�l�nben a k�perny�k bez�r�sakor nem
  lenne folyamatos a szabad mem�ria.
*/

	if( !LoadFile( "PROGRAM:Tartalom",0 ) )
		exit( FALSE );
	ContAddr = FileAddr;
	ContSize = FileSize;

#ifdef TEST
puts("tartalom ut�n, OpenLogo() el�tt");
#endif

	OpenLogo();
	ScreenLongUp( LogoScreen );

#ifdef TEST
puts("OpenLogo() ut�n, zene el�tt");
#endif

/* Zene ind�t�sa */

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
