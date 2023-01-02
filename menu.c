/*****************************************************************

Program:	menu.c

			A GURU lemezújság menu rutinjai.
			
Szerzõ:		Marinov Gábor	<<Gaborca>>

Dátum:		1990.10.24.

Utolsó
módosítás:	1991.10.05.

******************************************************************/

#include <exec/types.h>
#include <intuition/intuition.h>
#include <functions.h>

/*********************************************************

 Az Elem típusainak definiciója 	 / UWORD Type / 

**********************************************************/

#define T_FREE		NULL	/*  fel nem töltött Elem	*/

#define	T_EMPTY		1		/*  Üres sor				*/
#define	T_HEADLINE	2		/*  Nem kiválasztható cím	*/
#define	T_DOOR		3		/*  Kivalasztható cím		*/
#define T_ARTICLE	4		/*  Cikk 					*/
#define T_LIST		5		/*  Programlista			*/
#define	T_MUSIC		6		/*  Zenemodul				*/
#define T_PICTURE	7		/*  Kép						*/
#define T_PROGRAM	8		/*  Egyszerû futtatható prg.*/
#define	T_INTRO		9		/*  Életveszélyes intro 	*/

struct	Elem{
			UWORD		Page;
			UWORD		Type;
			char		Name[35];
			char		Title[35];
			};

struct	Page{
			UWORD	Prev;
			UWORD	Next;
			UWORD	Parent;
			UWORD	Number;
			UWORD	Elem[22];
			};

/**************************************************************/

#define EMPTYPOINTER 0
#define ARROWPOINTER 1
#define CLOCKPOINTER 2

extern	BOOL	ShowMessage();

extern	struct	Task *GuruTask;

extern	BOOL	LoadFile();
extern	char	*FileAddr;
extern	long	FileSize;
extern	long	ExamineFile();

extern	char	ModulName[];

extern	struct	RPort 	*MiddleRast,*TopRast;
extern	struct	Window	*MiddleWindow,*BottomWindow;
extern	struct	Screen	*LogoScreen;

extern	UWORD	Color;
extern	struct	Image icon[];
extern	struct	Gadget Gadget1,Gadget8;
extern	BOOL	PlayMusic;
BOOL	IsMusicInMemory;

long	 ContSize = 0;
char	*ContAddr = NULL;

UWORD SumPage;
UWORD SumElem;

struct	Page	*Page;
struct	Elem	*Elem;
char	*Date;

UWORD	Level  = 0;

struct	Gadget	MenuBar[22];

SHORT	BarX[] = { 12,12,12,12,12,12,12,12,12,12,12, 330,330,330,330,330,330,330,330,330,330,330 };
SHORT	BarY[] = {  0,15,30,45,60,75,90,105,120,135,150, 0,15,30,45,60,75,90,105,120,135,150 };

SHORT	pairtop[]    = { 0,14 , 0,0 , 296,0 };
SHORT	pairbottom[] = { 296,1 , 296,14 , 1,14 };

struct	Border border2 =
{
	0, 0, 1, 0, JAM1, 3, pairbottom, NULL
};
struct	Border border1 =
{
	0, 0, 5, 0, JAM1, 3, pairtop, &border2
};

UWORD CallModul();


char	*CopyRight = 

"\2Program & Design:\n"
"\1                 Gaborca\n"
  "\n"
"\2Szerkesztõk:\n"
"\1   Bear      of 23 °C Crew\n"
  "   Shy Guy   of Flatliners\n"
  "   Masell    of Flatliners\n"
  "   FricForce of FI-RE Crew\n"
  "   Gaborca   of Gaborca\n"
  "\n"
"\2Címünk:\n"
"\1 GURU 1399 Budapest, Pf. 701";

reverse( s )
char s[];
{
	int i,j;
	char	c;

	for( i=0,j=strlen( s )-1;i<j;i++,j--){
		c=s[i];
		s[i]=s[j];
		s[j]=c;
	}
}

/* UWORD -> ASCII */

uwtoa( n,s )
char s[];
UWORD n;
{
	UWORD i = 0;

	do{
		s[i++] = n%10+'0';
	}while( (n/=10) >0 );

	s[i]='\0';
	reverse( s );
}


BOOL InitMenu()
{
	BYTE i;

	for( i=0; i<22 ;i++ ){

		MenuBar[i].NextGadget		= (i<21) ? &MenuBar[i+1]:NULL;
		MenuBar[i].LeftEdge			= BarX[i];
		MenuBar[i].TopEdge			= BarY[i];
		MenuBar[i].Width			= 297;
		MenuBar[i].Height			= 15;

		MenuBar[i].Flags			= GADGHCOMP;
		MenuBar[i].Activation		= RELVERIFY;
		MenuBar[i].GadgetType		= BOOLGADGET;
		MenuBar[i].GadgetRender		= NULL;
		MenuBar[i].SelectRender		= NULL;
		MenuBar[i].GadgetText		= NULL;
		MenuBar[i].MutualExclude	= NULL;
		MenuBar[i].SpecialInfo		= NULL;
		MenuBar[i].GadgetID			= 100+i;
		MenuBar[i].UserData			= NULL;
	}

/*
   A Tartalom jegyzék betöltését, ContAddr,ContSize beállítása 
   a guru.c-ben van a memória folytonosságának
   megõrzése miatt ( bõvebben lásd ott!).
*/

	SumPage = *(UWORD *)&ContAddr[40];	/* a legmagasabb Page index!, mert a 0. Page és Elem nem használt*/
	SumElem = *(UWORD *)&ContAddr[40+sizeof( UWORD )];
	Date	= &ContAddr[5];

	Page = (struct Page *)&ContAddr[40+2*sizeof( UWORD )];
	Elem = (struct Elem *)&ContAddr[40+2*sizeof( UWORD ) + (SumPage+1) * sizeof(struct Page)];

#ifdef TEST
printf("Sumpage : %d\nSumElem : %d\n",SumPage,SumElem);
printf("Page[1].Elem[0] : %d\n",Page[1].Elem[0]);
printf("Elem[1].Title : %s\n,Elem[1].Type : %d\nElem[1].Page : %d\n",Elem[1].Title,Elem[1].Type,Elem[1].Page);
#endif
}

DrawMenuBar( x,y,color )
SHORT	x,y;
UBYTE	color;
{
	DrawBorder( MiddleRast ,&border1 ,x ,y );
	SetAPen( MiddleRast, color );
	RectFill( MiddleRast, x+1,y+1, x+295, y+13 );
}

DrawMenuPage( pagenumber )
UWORD pagenumber;
{
	register BYTE i;
	UBYTE	 Type;
	char	*text;
	UWORD	ParentPage;

	Move( MiddleRast , 0,0 );
	ClearScreen( MiddleRast );

	ParentPage = Page[pagenumber].Parent;

	if( ParentPage )
		WriteHeader( Elem[ Page[ ParentPage ].Number ].Title );
	else
		WriteHeader( Date );


	if( Page[pagenumber].Prev )
		DrawImage( TopRast,&icon[19],0,0 );
	else
		DeleteIcon(100);

	if( Page[pagenumber].Next )
		DrawImage( TopRast,&icon[20],0,0 );
	else
		DeleteIcon(101);

	for(i=0;i<22;i++){

		Type = Elem[ Page[pagenumber].Elem[i] ].Type;
		text = Elem[ Page[pagenumber].Elem[i] ].Title;

		switch( Type ){

			case T_EMPTY:
						MenuBar[i].Flags = GADGHNONE;
						break;

			case T_HEADLINE:
						MenuBar[i].Flags = GADGHNONE;

						DrawMenuBar( BarX[i],BarY[i], 3 );
						SetAPen( MiddleRast, 1 );
						Move( MiddleRast , BarX[i]+10 ,BarY[i]+12 );
						Text( MiddleRast, text , strlen( text ) );

						SetAPen( MiddleRast, 7 );
						Move( MiddleRast , BarX[i]+8 ,BarY[i]+10 );
						Text( MiddleRast, text , strlen( text ) );
						break;

			default:
						MenuBar[i].Flags = GADGHCOMP;

						DrawMenuBar( BarX[i],BarY[i], 2 );
						SetAPen( MiddleRast, 1 );
						Move( MiddleRast , BarX[i]+10 ,BarY[i]+12 );
						Text( MiddleRast, text , strlen( text ) );

						SetAPen( MiddleRast, 5 );
						Move( MiddleRast , BarX[i]+8 ,BarY[i]+10 );
						Text( MiddleRast, text , strlen( text ) );
						break;
		}
	}
	RefreshGadgets( &MenuBar[0], MiddleWindow, NULL );
}


Menu( ActPage )
UWORD ActPage;
{
	int ActGadget;

	if( !InitMenu() )
		return();

#ifdef TEST
puts("InitMenu után");
#endif

	AddGList( MiddleWindow, &MenuBar[0], 1, 22, NULL );

#ifdef TEST
puts("AddGList után");
#endif

	DrawMenuPage( ActPage );

#ifdef TEST
puts("DrawMenuPage után");
#endif

	TopBottomLongUp();

	for(;;){

		ActGadget = GetGadget();
		switch( ActGadget ){

			case 1:
					if( Page[ActPage].Prev ){
						ActPage = Page[ActPage].Prev;
						TopShortDown();
						DrawMenuPage( ActPage );
						TopShortUp();
					}
					break;

			case 2:
					if( Page[ActPage].Next ){
						ActPage = Page[ActPage].Next;
						TopShortDown();
						DrawMenuPage( ActPage );
						TopShortUp();
					}
					break;

			case 3:
					if( Page[ActPage].Parent ){
						ActPage = Page[ActPage].Parent;
						TopShortDown();
						DrawMenuPage( ActPage );
						TopShortUp();
					}
					else{
						if( ShowMessage( "A GURU megfontoltságra int !" , "Biztosan ki akarsz lépni a GURU-ból ?" , TRUE ,FALSE, FALSE )){
	
							RemoveGList( MiddleWindow, &MenuBar[0], 22 );

							TopBottomLongDown();
							return;
						}
					}
					break;
			case 4:
					ShowMessage("Ez nem fog menni !","A menüt sajnos nem lehet kinyomtatni." , FALSE , FALSE, FALSE );
					break;

			case 5:
					ShowMessage("GURU v1.31",CopyRight ,FALSE,TRUE, FALSE);
					break;
			case 6:
			case 7:
			case 8:
					Preferences( ActGadget );
					break;

			default:
					ActPage = CallModul( ActGadget-100, ActPage );
					break;		

		}
	}
}

UWORD CallModul( GadgetNumber , ActPage )
int GadgetNumber;
UWORD ActPage;
{
	UWORD	SelectedElem;
	char	StartName[35+20];
	char	CommandLine[150];
	char	tmp[5];
	struct	FileHandle	*input,*output;
	BOOL	OldPlayMusic;

	SelectedElem = Page[ActPage].Elem[GadgetNumber];
	switch( Elem[ SelectedElem ].Type ){

		case T_DOOR:
					if( Elem[SelectedElem].Page ){		/* Ha nem lenne kitöltve, nem csinál semmit  */
						Page[ActPage].Number = SelectedElem;
						ActPage = Elem[SelectedElem].Page;
						TopShortDown();
						DrawMenuPage( ActPage );
						TopShortUp();
					}
					break;

		case T_ARTICLE:
					TopShortDown();

					RemoveGList( MiddleWindow, &MenuBar[0], 22 );
					LoadAndShowArticle( Elem[ SelectedElem ].Name , Elem[ SelectedElem ].Title );
					AddGList( MiddleWindow, &MenuBar[0], 1, 22, NULL );

					DrawMenuPage( ActPage );
					TopShortUp();
					break;

		case T_LIST:
					TopShortDown();
					RemoveGList( MiddleWindow, &MenuBar[0], 22 );

					ShowList( Elem[ SelectedElem ].Name , Elem[ SelectedElem ].Title );

					AddGList( MiddleWindow, &MenuBar[0], 1, 22, NULL );
					DrawMenuPage( ActPage );
					TopShortUp();
					break;

		case T_MUSIC:
					TopShortDown();
					IsMusicInMemory = LoadAndPlayMusic( Elem[ SelectedElem ].Name  );
					RefreshGadgets( &Gadget1,BottomWindow,NULL );
					TopShortUp();
					break;

		case T_PICTURE:
					TopBottomLongDown();

					ShowPic( Elem[ SelectedElem ].Name  );
					AddGList( MiddleWindow, &MenuBar[0], 1, 22, NULL );
					DrawMenuPage( ActPage );

					TopBottomLongUp();
					break;

		case T_PROGRAM:

					input  = Input();
					output = Output();

					TopBottomLongDown();

					if( ExamineFile( "PROGRAM:RunProgram" )!=0L ){

						CloseScreens();
						OldPlayMusic = PlayMusic;
						if( PlayMusic ){
							PlayMusic = FALSE;
							Gadget8.GadgetRender=(APTR)&icon[17];
							PauseMusic();
						}
						strcpy( StartName, "Execute PROGRAM:RunProgram PROGRAMOK:" );
						strcat( StartName, Elem[ SelectedElem ].Name );
					
						WBenchToFront();

						Execute( StartName , input ,output );

						OpenScreens();
						if( OldPlayMusic ){
							PlayMusic = TRUE;
							Gadget8.GadgetRender=(APTR)&icon[8];
							RefreshGadgets( &Gadget8,BottomWindow,NULL );
							PauseMusic();
						}

						AddGList( MiddleWindow, &MenuBar[0], 1, 22, NULL );
						DrawMenuPage( ActPage );

						TopBottomLongUp();
					}
					else{
						TopBottomLongUp();
						Warning( "Nem találom a PROGRAM:RunProgram indító fájlt!" );
					}
					break;


		case T_INTRO:
					break;

/* Vigyázz a ScreensLongUp,ScreensLongDown -nál a pointer-re ! */

					input  = Input();
					output = Output();

					strcpy( CommandLine, "RunBack C:Execute PROGRAM:RunIntro " );
/* az intro neve */
					strcat( CommandLine, "INTRÓK:" );
					strcat( CommandLine, Elem[ SelectedElem ].Name );
					strcat( CommandLine, " " );
/* lapszám */
					uwtoa( ActPage , tmp );
					strcat( CommandLine, tmp );
					strcat( CommandLine, " " );
/* paletta száma */
					uwtoa( Color , tmp );
					strcat( CommandLine, tmp );
					strcat( CommandLine, " " );
/* zenemodul neve */
					if( IsMusicInMemory )
						strcat( CommandLine, ModulName );
					else
						strcat( CommandLine, "NEM" );


					TopBottomLongDown();
					ScreenLongDown( LogoScreen );

					WBenchToFront();
					puts( CommandLine );

					if( ExamineFile( "PROGRAM:RunIntro" )!=0L ){
						Execute( CommandLine , input ,output );
						CleanUp();
						exit( TRUE );
					}
					else{
						Warning( "Nem találom a PROGRAM:RunIntro indító fájlt!" );
						ScreenLongUp( LogoScreen );
						TopBottomLongUp();
					}
					break;
	}
	return( ActPage );
}
