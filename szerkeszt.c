/*****************************************************************

Program:	szerkeszt.c

			A GURU lemez�js�g tartalomjegyz�kgener�l� programja.
			
Szerz�:		Marinov G�bor	<<Gaborca>>

D�tum:		1990.07.06

Utols�
m�dos�t�s:	1990.10.24

******************************************************************/

#include <stdio.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>

#include <libraries/dosextens.h>
#include <libraries/dos.h>
#include "ext/reqbase.h"

#include <functions.h>

#define MAXGADS	 22
#define MAXPAGE	 50
#define MAXELEM	(MAXPAGE*MAXGADS)


struct	Elem{
			UWORD	Page;
			UWORD	Type;
			char	Name[35];
			char	Title[35];
			};

struct	Page{
			UWORD	Prev;
			UWORD	Next;
			UWORD	Parent;
			UWORD	Number;
			UWORD	Elem[22];
			};


/*********************************************************

 Az Elem t�pusainak definici�ja 	 / UWORD Type / 

**********************************************************/

#define T_FREE		NULL	/*  fel nem t�lt�tt Elem	*/

#define	T_EMPTY		1		/*  �res sor				*/
#define	T_HEADLINE	2		/*  Nem kiv�laszthat� c�m	*/
#define	T_DOOR		3		/*  Kivalaszthat�c�m		*/
#define T_ARTICLE	4		/*  Cikk 					*/
#define T_LIST		5		/*  Programlista			*/
#define	T_MUSIC		6		/*  Zenemodul				*/
#define T_PICTURE	7		/*  K�p						*/
#define T_PROGRAM	8		/*  Egyszer� futtathat� prg.*/
#define	T_INTRO		9		/*  �letvesz�lyes intro 	*/

char	*ElemType[]=
			{
			"Nem Haszn�lt",			/* 0 */
			"�res sor",				/* 1 */
			"Fejezetc�m",			/* 2 */
			"Ajt�",					/* 3 */
			"Cikk",					/* 4 */
			"Programlista",			/* 5 */
			"Zenemodul",			/* 6 */
			"K�p",					/* 7 */
			"Egyszer� program",		/* 8 */
			"�letvesz�lyes intro"	/* 9 */
			};

struct	IntuitionBase	*IntuitionBase	= NULL;
struct	GfxBase			*GfxBase 		= NULL;	
struct	ReqBase			*ReqBase		= NULL;
struct	DosBase			*DosBase		= NULL;

struct	Window			*window  = NULL;
struct	IntuiMessage	*message = NULL;
struct	RastPort		*rp		 = NULL;

struct	Elem			*Elem = NULL;
struct	Page			*Page = NULL;

ULONG	ElemSize;
ULONG	PageSize;

UWORD ActPage   = 0;		/* az �ppen a k�perny�n l�tsz� lap sorsz�ma */
UWORD ActElem   = 0;		/* az utolj�ra kijel�lt elem */

UWORD SumPage;
UWORD SumElem;

UWORD Level;

char	*Space = "                                   ";

#define GADGWIDTH	280
#define GADGHEIGHT	 10

SHORT GadgetPosX[] =
	{
		25,25,25,25,25,25,25,25,25,25,25,330,330,330,330,330,330,330,330,330,330,330
	};
SHORT GadgetPosY[] =
	{
		40,55,70,85,100,115,130,145,160,175,190,40,55,70,85,100,115,130,145,160,175,190
	};

SHORT GadgetPairs[] =
	{
	0, 0, 281, 0, 281, 11, 0, 11, 0, 0,
	};

struct Border GadgetBorder =
	{
	-1, -1, 1, 0, JAM1, 5, GadgetPairs, NULL,
	};

struct	Gadget BoolGadget[22];

struct NewWindow newwindow =
	{
	0, 0, 					  /* LeftEdge, TopEdge	 */
	640, 256,				  /* Width, Height		 */
	0, 1, 					  /* DetailPen, BlockPen */

	CLOSEWINDOW |			  /* IDCMP Flags			 */
	GADGETUP |
	GADGETDOWN |
	MENUPICK,

	WINDOWDEPTH |			  /* Flags					 */
	WINDOWDRAG |
	WINDOWCLOSE |
	ACTIVATE |
	SMART_REFRESH,

	&BoolGadget[0],		  /* First Gadget 		 */
	NULL, 					  /* CheckMark 			 */
	(UBYTE *)" GURU Szerkeszt�s�g ",
	NULL, 					  /* Screen 				 */
	NULL, 					  /* BitMap 				 */
	640, 100,				  /* Min Width, Height	 */
	640, 256,				  /* Max Width, Height	 */
	WBENCHSCREEN,			  /* Type					 */
	};


/*********************************************************
 Menu 
**********************************************************/

struct IntuiText text52 =
	{
	 0,
	 1,
	 JAM2,
	 0,
	 1,
	 NULL,
	 (UBYTE *) " L�pj vissza ",
	 NULL
	};

struct IntuiText text51 =
	{
	 0,
	 1,
	 JAM2,
	 0,
	 1,
	 NULL,
	 (UBYTE *) " L�pj be ",
	 NULL
	};

struct MenuItem item52 =
	{
	 NULL,
	  0,
	 15,
	 13*8,
	 10,
	 ITEMTEXT|ITEMENABLED|HIGHCOMP,
	 0,
	 (APTR) &text52,
	 NULL,
	 NULL,
	 NULL
	};  

struct MenuItem item51 =
	{
	 &item52,
	  0,
	  5,
	 13*8,
	 10,
	 ITEMTEXT|ITEMENABLED|HIGHCOMP,
	 0,
	 (APTR) &text51,
	 NULL,
	 NULL,
	 NULL
	};

struct Menu menu5 =					
	{										
	 NULL,
	 35*8,
	 0,
	 7*8,
	 10,
	 MENUENABLED,
	 " Szint ",
	 &item51
	};

/***********************************/

struct IntuiText text42 =
	{
	 0,
	 1,
	 JAM2,
	 0,
	 1,
	 NULL,
	 (UBYTE *) " H�tra ",
	 NULL
	};

struct IntuiText text41 =
	{
	 0,
	 1,
	 JAM2,
	 0,
	 1,
	 NULL,
	 (UBYTE *) " El�re ",
	 NULL
	};

struct MenuItem item42 =
	{
	 NULL,
	  0,
	 15,
	 7*8,
	 10,
	 ITEMTEXT|ITEMENABLED|HIGHCOMP,
	 0,
	 (APTR) &text42,
	 NULL,
	 NULL,
	 NULL
	};  

struct MenuItem item41 =
	{
	 &item42,
	  0,
	 5,
	 7*8,
	 10,
	 ITEMTEXT|ITEMENABLED|HIGHCOMP,
	 0,
	 (APTR) &text41,
	 NULL,
	 NULL,
	 NULL 	 
	};

struct Menu menu4 =					
	{										
	 &menu5,
	 25*8,
	 0,
	 8*8,
	 10,
	 MENUENABLED,
	 " Lapozz ",
	 &item41
	};

/*******************************************************/

struct IntuiText text32 =
	{
	 0,
	 1,
	 JAM2,
	 0,
	 1,
	 NULL,
	 (UBYTE *) " T�r�lj ",
	 NULL
	};

struct IntuiText text31 =
	{
	 0,
	 1,
	 JAM2,
	 0,
	 1,
	 NULL,
	 (UBYTE *) " Adj ",
	 NULL
	};

struct MenuItem item32 =
	{
	 NULL,
	  0,
	 15,
	 8*8,
	 10,
	 ITEMTEXT|ITEMENABLED|HIGHCOMP,
	 0,
	 (APTR) &text32,
	 NULL,
	 NULL,
	 NULL
	};  

struct MenuItem item31 =
	{
	 &item32,
	  0,
	 5,
	 8*8,
	 10,
	 ITEMTEXT|ITEMENABLED|HIGHCOMP,
	 0,
	 (APTR) &text31,
	 NULL,
	 NULL,
	 NULL 	 
	};

struct Menu menu3 =					
	{										
	 &menu4,
	 18*8,
	 0,
	 5*8,
	 10,
	 MENUENABLED,
	 " Lap ",
	 &item31
	};

/*******************************************************/

struct IntuiText text21 =
	{
	 0,
	 1,
	 JAM2,
	 0,
	 1,
	 NULL,
	 (UBYTE *) " M�dos�tsd ",
	 NULL
	};

struct MenuItem item21 =
	{
	 NULL,
	  0,
	 5,
	 11*8,
	 10,
	 ITEMTEXT|ITEMENABLED|HIGHCOMP,
	 0,
	 (APTR) &text21,
	 NULL,
	 NULL,
	 NULL
	};

struct Menu menu2 =					
	{										
	 &menu3,
	 10*8,
	 0,
	 6*8,
	 10,
	 MENUENABLED,
	 " Elem ",
	 &item21
	};

/**************************************/


struct IntuiText text14 =
	{
	 0,
	 1,
	 JAM2,
	 0,
	 1,
	 NULL,
	 (UBYTE *) " Kil�p�s ",
	 NULL
	};

struct IntuiText text13 =
	{
	 0,
	 1,
	 JAM2,
	 0,
	 1,
	 NULL,
	 (UBYTE *) " R�lam ",
	 NULL
	};

struct IntuiText text12 =
	{
	 0,
	 1,
	 JAM2,
	 0,
	 1,
	 NULL,
	 (UBYTE *) " Ment�s ",
	 NULL
	};

struct IntuiText text11 =
	{
	 0,
	 1,
	 JAM2,
	 0,
	 1,
	 NULL,
	 (UBYTE *) " T�lt�s ",
	 NULL
	};

struct MenuItem item14 =
	{
	 NULL,
	  0,
	 45,
	 9*8,
	 10,
	 ITEMTEXT|ITEMENABLED|HIGHCOMP,
	 0,
	 (APTR) &text14,
	 NULL,
	 NULL,
	 NULL
	};  

struct MenuItem item13 =
	{
	 &item14,
	  0,
	 30,
	 9*8,
	 10,
	 ITEMTEXT|ITEMENABLED|HIGHCOMP,
	 0,
	 (APTR) &text13,
	 NULL,
	 NULL,
	 NULL
	};  

struct MenuItem item12 =
	{
	 &item13,
	  0,
	 15,
	 9*8,
	 10,
	 ITEMTEXT|ITEMENABLED|HIGHCOMP,
	 0,
	 (APTR) &text12,
	 NULL,
	 NULL,
	 NULL
	};  

struct MenuItem item11 =
	{
	 &item12,
	  0,
	  5,
	 9*8,
	 10,
	 ITEMTEXT|ITEMENABLED|HIGHCOMP,
	 0,
	 (APTR) &text11,
	 NULL,
	 NULL,
	 NULL 	 
	};

struct Menu menu1 =					
	{										
	 &menu2,
	 2*8,
	 0,
	 6*8,
	 10,
	 MENUENABLED,
	 " F�jl ",
	 &item11
	};

/*********************************************************
 Menu v�ge 
**********************************************************/


/*********************************************************
 InputWindow 
**********************************************************/

struct Window 		 *InputWindow = NULL;

#define TYPENUM 9
#define STRINGSIZE 36
unsigned char StringBuffer1[STRINGSIZE];
unsigned char UndoBuffer1  [STRINGSIZE];
unsigned char StringBuffer2[STRINGSIZE];
unsigned char UndoBuffer2  [STRINGSIZE];

unsigned	TypePosX[] = { 24,24,24,24,24,187,187,187,187 };
unsigned	TypePosY[] = { 15,30,45,60,75 ,15, 30 ,45 ,60 };

struct StringInfo StringInfo2 =
	{
	StringBuffer2, 		 /* Buffer					*/
	UndoBuffer2,			 /* Undo Buffer			*/
	0, 						/* Buffer Position	  */
	STRINGSIZE, 			/* MaxChars 			  */
	0, 						/* Display Positoin	  */
	0, 						/* Undo Position		  */
	0, 						/* NumChars 			  */
	0, 						/* Display Counter	  */
	0, 0, 					/* CLeft, CTop 		  */
	NULL, 					/* LayerPtr 			  */
	0, 						/* LongInt				  */
	NULL, 					/* AltKeyMap			  */
	};

struct StringInfo StringInfo1 =
	{
	StringBuffer1, 		 /* Buffer					*/
	UndoBuffer1,			 /* Undo Buffer			*/
	0, 						/* Buffer Position	  */
	STRINGSIZE, 			/* MaxChars 			  */
	0, 						/* Display Positoin	  */
	0, 						/* Undo Position		  */
	0, 						/* NumChars 			  */
	0, 						/* Display Counter	  */
	0, 0, 					/* CLeft, CTop 		  */
	NULL, 					/* LayerPtr 			  */
	0, 						/* LongInt				  */
	NULL, 					/* AltKeyMap			  */
	};

SHORT StringGadgetPairs[] =
	{
	0, 0, 289, 0, 289, 11, 0, 11, 0, 0,
	};

SHORT TypePairs[] =
	{
	0, 0, 17, 0, 17, 9, 0, 9, 0, 0,
	};

struct Border StringGadgetBorder =
	{
	-1, -2, 1, 0, JAM1, 5, StringGadgetPairs, NULL,
	};

struct Border TypeBorder =
	{
	-1, -1, 1, 0, JAM1, 5, TypePairs, NULL,
	};

struct IntuiText InputText1 =
	{
	3, 0, JAM2, -80, 0, NULL, (UBYTE *)"C�m     :" ,NULL,
	};

struct IntuiText InputText2 =
	{
	3, 0, JAM2, -80, 0, NULL, (UBYTE *)"F�jln�v :" ,NULL,
	};

struct	Gadget		TypeGadget[TYPENUM];
struct	IntuiText	TypeIText[TYPENUM];

struct Gadget StringGadget2 =
	{
	&TypeGadget[0],
	88, 110,
	288,10,
	GADGHCOMP,
	RELVERIFY,
	STRGADGET,
	(APTR)&StringGadgetBorder,
	NULL,
	&InputText2,
	NULL,
	(APTR)&StringInfo2,
	102,
	NULL,
	};

struct Gadget StringGadget1 =
	{
	&StringGadget2,
	88, 95,
	288,10,
	GADGHCOMP,
	RELVERIFY ,
	STRGADGET,
	(APTR)&StringGadgetBorder,
	NULL,
	&InputText1,
	NULL,
	(APTR)&StringInfo1,
	101,
	NULL,
	};

struct NewWindow InputNewWindow =
	{
	140, 30,
	385, 130,
	0, 1,
	CLOSEWINDOW |
	GADGETUP |
	GADGETDOWN,

	WINDOWDEPTH |
	WINDOWDRAG |
	WINDOWCLOSE |
	ACTIVATE |
	SMART_REFRESH,

	&StringGadget1,
	NULL,
	(UBYTE *)" Elem m�dos�t�s ",
	NULL,
	NULL,
	640, 120,
	640, 256,
	WBENCHSCREEN,
	};
/*********************************************************
 InputWindow v�ge
**********************************************************/

/*********************************************************
 Requester tartoz�kok
**********************************************************/

struct	Process	*myprocess = NULL;
APTR	olderrorwindow 		= NULL;

struct TRStructure	MyReq = 
		{
		NULL,
		NULL,
		NULL,

		NULL,
		NULL,
		NULL,

		NULL,
		AMIGAKEYS,
		1,
		0,
		1,
		0,
		0,
		0
		};

char	*About = 
				"\n"
				"  GURU - a legjobb magyar lemezujs�g  \n"
				"  ==================================  \n"
				"    Tartalomjegyz�k gener�l� v0.00    \n"
				"\n"
				"  Az eg�sz motorja : BEAR\n"
				"                     Ber�nyi Zolt�n   \n"
				"\n"
				"  A fantasztikus\n"
				"  programoz�       : GABORCA\n"
				"                     Marinov G�bor\n"
				"\n";

char	*igen	= " Igen ";
char	*nem	= " Nem ";
char	*sajna  = " Sajn�lom... ";

struct	FileRequester	MyFileReqStruct;
char	filename[FCHARS];
char	directoryname[DSIZE];
char	answerarray[DSIZE+FCHARS];


/*********************************************************
 Requester tartoz�kok v�ge
**********************************************************/

HInfo( s )
char *s;
{
	puts( s );
	printf("============\nSumPage : %u\nActPage : %u\nSumElem : %u\nActElem : %u\n",SumPage,ActPage,SumElem,ActElem );
	printf("Page-----------\nNext   : %u\nPrev   : %u\nParent : %u\n",Page[ActPage].Next,Page[ActPage].Prev,Page[ActPage].Parent);
	printf("Elem-----------\nType : %d\nPage : %u\n\n",Elem[ActElem].Type,Elem[ActElem].Page);

}


reverse(s)
char s[];
{
	int c,i,j;
	for(i=0,j=strlen(s)-1;i<j;i++,j--){
		c=s[i];
		s[i]=s[j];
		s[j]=c;
	}
}

itoa(n,s)
char s[];
int	n;
{
	int i,sign;
	if((sign = n) < 0)
		n=-n;
	i=0;
	do{
		s[i++]=n%10 + '0';
	}while((n/=10)>0);
	if(sign<0)
		s[i++]='-';
	s[i]='\0';
	reverse(s);
}

BOOL ShowMyReq(yes,no,text )
char *yes;
char *no;
char *text;
{
	MyReq.Text			= text;
	MyReq.PositiveText = yes;
	MyReq.NegativeText = no;

	return( (BOOL)TextRequest( &MyReq ) );
}

BOOL SelectFile()
{

	answerarray[0] = 0;

	MyFileReqStruct.PathName = answerarray;
	MyFileReqStruct.Dir = directoryname;
	MyFileReqStruct.File = filename;

 	MyFileReqStruct.Flags = FRQCACHINGM;

	MyFileReqStruct.dirnamescolor = 2;
	MyFileReqStruct.devicenamescolor = 2;

	return( FileRequester(&MyFileReqStruct) );
}

SaveFile()
{
	struct FileHandle *file = NULL;
	char	date[34];

	date[0]='\0';
	if( GetString( &date, " K�rem a d�tumot : ", 0, 35, 35 ) ){
		if( SelectFile() ){
			if ( file = Open( answerarray , MODE_NEWFILE ) ){

				Write( file, "GURU\0" , 5 );
				Write( file, &date  ,35 );

				Write( file, &SumPage , sizeof( UWORD ) );
				Write( file, &SumElem , sizeof( UWORD ) );

				Write( file, Page , sizeof( struct Page )*(SumPage + 1) );
				Write( file, Elem , sizeof( struct Elem )*(SumElem + 1) );

				Close( file );
			}
			else
				ShowMyReq( NULL , sajna ," Nem tudom a ment�shez megnyitni a f�jlt ! ");
		}
	}
}

LoadFile()
{
	struct FileHandle *file = NULL;
	UWORD i;
	char	buffer[34];

	if( SelectFile() ){

		if ( file = Open( answerarray , MODE_OLDFILE ) ){

			Read( file, &buffer[0] , 5 );

			if( !strcmp(buffer, "GURU" ) ){	   /* 0 ha egyform�k  */


				for(i=0; i<=SumPage ; i++){
					Page[ i ].Next   = 0;
					Page[ i ].Prev   = 0;
					Page[ i ].Parent = 0;
					Page[ i ].Number = 0;
					FillPageToEmpty( i );
				}
				for(i=0; i<=SumElem ; i++){
					Elem[i].Type = 0;
					Elem[i].Page = 0;
					strcpy( Elem[i].Title , "" );
					strcpy( Elem[i].Name  , "" );
				}

				Read( file, &buffer[0] , 35 );

				Read( file, &SumPage , sizeof( UWORD ) );
				Read( file, &SumElem , sizeof( UWORD ) );

				Read( file, Page , sizeof( struct Page )*(SumPage + 1) );
				Read( file, Elem , sizeof( struct Elem )*(SumElem + 1) );

			}
			else
				ShowMyReq( NULL , sajna ," Ez nem GURU tartalomjegyz�k f�jl ! ");

			Close( file );
		}
		else
			ShowMyReq( NULL , sajna ," Nem tudom a t�lt�shez megnyitni a f�jlt ! ");
		

	}
}


/*********************************************************
 InitPageStruct()

Az Page �s az Elem t�mb sz�m�ra mem�ria foglal�sa
**********************************************************/

void InitPageStruct()
{
	PageSize = sizeof( struct Page )*MAXPAGE;
	ElemSize = sizeof( struct Elem )*MAXELEM;

	Page = (struct Page *)AllocMem( PageSize , MEMF_CLEAR );
	if(!Page){
		CleanUp();
		exit(FALSE);
	}
	Elem = (struct Elem *)AllocMem( ElemSize , MEMF_CLEAR );
	if(!Elem){
		CleanUp();
		exit(FALSE);
	}
	Elem[0].Type = T_EMPTY;	  /*** Az �sszes �res elem (T_EMPTY) a Lapokon ide mutat majd ***/
}

/***************************************************************
 InitGads()
 
 InputWindow �s a t�bbi gadget elhelyez�se, bek�t�se a l�ncba.
***************************************************************/

void InitGads()
{
	UWORD i;


/*************  InputWindow	******************/

	for(i=0 ; i<TYPENUM ; i++){
		TypeIText[i].FrontPen		= 1;
		TypeIText[i].BackPen		= 0;
		TypeIText[i].DrawMode		= JAM2;
		TypeIText[i].LeftEdge		= 24;
		TypeIText[i].TopEdge		= 0;
		TypeIText[i].ITextFont		= NULL;
		TypeIText[i].IText			= (UBYTE *)ElemType[i+1];
		TypeIText[i].NextText		= NULL;

		if( i != (TYPENUM-1) )
			TypeGadget[i].NextGadget=(struct Gadget *)&TypeGadget[i+1];

		TypeGadget[i].LeftEdge		= TypePosX[i];
		TypeGadget[i].TopEdge		= TypePosY[i];
		TypeGadget[i].Width			= 16;
		TypeGadget[i].Height		= 8;
		TypeGadget[i].Flags			= GADGHCOMP;
		TypeGadget[i].Activation	= RELVERIFY;
		TypeGadget[i].GadgetType	= BOOLGADGET;
		TypeGadget[i].GadgetRender	= (APTR)&TypeBorder;
		TypeGadget[i].SelectRender	= NULL;
		TypeGadget[i].GadgetText	= &TypeIText[i];
		TypeGadget[i].MutualExclude	= NULL;
		TypeGadget[i].SpecialInfo	= NULL;
		TypeGadget[i].GadgetID		= 200+i+1;
		TypeGadget[i].UserData		= NULL;
	}
		TypeGadget[TYPENUM-1].NextGadget = NULL;

/*************  InputWindow v�ge ******************/


	for(i=0;i<MAXGADS;i++){

		if( i != (MAXGADS-1) )
			BoolGadget[i].NextGadget=(struct Gadget *)&BoolGadget[i+1];

		BoolGadget[i].LeftEdge		= GadgetPosX[i];
		BoolGadget[i].TopEdge		= GadgetPosY[i];
		BoolGadget[i].Width			= 280;
		BoolGadget[i].Height		= 10;
		BoolGadget[i].Flags			= GADGHCOMP;
		BoolGadget[i].Activation	= RELVERIFY;
		BoolGadget[i].GadgetType	= BOOLGADGET;
		BoolGadget[i].GadgetRender	= (APTR)&GadgetBorder;
		BoolGadget[i].SelectRender	= NULL;
		BoolGadget[i].GadgetText	= NULL;
		BoolGadget[i].MutualExclude	= NULL;
		BoolGadget[i].SpecialInfo	= NULL;
		BoolGadget[i].GadgetID		= i;
		BoolGadget[i].UserData		= NULL;
	}
		BoolGadget[MAXGADS-1].NextGadget = NULL;

}

/***************************************************************
 DrawPage()

 T�rli a gadget feliratokat, majd az ActPage �ltal 
 meghat�rozott lap c�meit ki�rja a t�pus figyelembev�tel�vel.
***************************************************************/

DrawPage()
{
	UWORD i;
	UWORD Type;
	char *Title;
	char Number[10];

	for(i=0;i<MAXGADS;i++){
		Type	= Elem[ Page[ ActPage ].Elem[i] ].Type;
		Title = Elem[ Page[ ActPage ].Elem[i] ].Title;

		Move( rp ,(long) GadgetPosX[i] ,(long) (GadgetPosY[i]+7) );
		Text( rp , Space , 35 );
		if(	Type == T_HEADLINE )
			SetAPen( rp, 2 );
		else
			SetAPen( rp, 1 );

		Move( rp ,(long) GadgetPosX[i] ,(long) (GadgetPosY[i]+7) );
		Text( rp , Title ,strlen(Title) );
	}
	SetAPen( rp, 1 );

		Move( rp ,16, 240+7 );
		Text( rp ,Space,19);

	if( Page[ActPage].Next ){
		Move( rp ,16, 240+7 );
		Text( rp ,"Van k�vetkez� lap !" , 19 );
	}

	itoa( Level , Number );
	Move( rp, 96 , 220+7 );
	Text( rp, Space, 5 );
	Move( rp, 96 , 220+7 );
	Text( rp, Number, strlen(Number) );

	itoa( Page[ActPage].Number , Number );
	Move( rp, 96 , 230+7 );
	Text( rp, Space, 5 );
	Move( rp, 96 , 230+7 );
	Text( rp, Number, strlen(Number) );



}

/***************************************************************
 ElemInfo()

	ActElem -r�l az als� info sorba �r.
***************************************************************/

ElemInfo()
{

/******  Sorok t�rl�se **********/

	Move( rp, 330 , 220+7 );
	Text( rp, Space, 35 );
	Move( rp, 330 , 230+7 );
	Text( rp, Space, 35 );
	Move( rp, 330 , 240+7 );
	Text( rp, Space ,35 );

/****** info ki�r�s *********/

	Move( rp, 330 , 220+7 );
	Text( rp, ElemType[ Elem[ActElem].Type ], strlen(ElemType[ Elem[ActElem].Type ]) );
	Move( rp, 330 , 230+7 );
	Text( rp, Elem[ActElem].Title, strlen(Elem[ActElem].Title) );
	Move( rp, 330 , 240+7 );
	Text( rp, Elem[ActElem].Name ,strlen(Elem[ActElem].Name) );
}

/***************************************************************
 InputElem()

 A ActElem -et bek�ri, frissiti a
 statuszsort es a lapot is.
***************************************************************/

InputElem( Gnum )
UWORD Gnum;
{
	struct IntuiMessage	*message = NULL;
	ULONG MessageClass;
	USHORT code;
	UWORD ActGad = 0;
	UWORD Type,OldType;

	OldType = Elem[ ActElem ].Type;
	strcpy( StringBuffer1 , Elem[ ActElem ].Title );
	strcpy( StringBuffer2 , Elem[ ActElem ].Name  );

	if (!(InputWindow = (struct Window *)OpenWindow(&InputNewWindow)))
		return;

	FOREVER	{
		if ((message = (struct IntuiMessage *)GetMsg(InputWindow->UserPort)) == NULL){
			Wait(1L << InputWindow->UserPort->mp_SigBit);
			continue;
		}
		MessageClass = message->Class;
		code = message->Code;
		ReplyMsg(message);
		
		switch (MessageClass){
		case GADGETUP	  :
		case GADGETDOWN  :	ActGad = ( (struct Gadget *)(message->IAddress) )->GadgetID;		

							if( ActGad > 200)
								Elem[ ActElem ].Type = ActGad-200;
							else{
								strcpy( Elem[ ActElem ].Title , StringBuffer1 );
								strcpy( Elem[ ActElem ].Name  , StringBuffer2 );
							}
							ElemInfo();
							break;

		case CLOSEWINDOW : 
							CloseWindow( InputWindow );
							Type = Elem[ActElem].Type;

/*  Ha valamit �resre �ll�tott akkor azt t�r�lni kell */
							if( (OldType != T_EMPTY) && (Type == T_EMPTY ) ){
								DeleteElem( ActElem );
								Page[ActPage].Elem[Gnum] = 0;
								ElemInfo();
							}

/* Ha �resr�l �ll�totta �t m�sra akkor l�tre kell hozni az elemet */					
							if( (OldType == T_EMPTY) && (Type != T_EMPTY) ){
								SumElem += 1;
								ActElem = SumElem;
								Elem[ActElem].Type = Type;
								strcpy( Elem[ActElem].Title, StringBuffer1 );
								strcpy( Elem[ActElem].Name,  StringBuffer2 );
								Page[ActPage].Elem[Gnum] = ActElem;

								if( (Type != T_DOOR) && (Type != T_HEADLINE) )
									Elem[ActElem].Page = ActPage;

								strcpy( Elem[0].Title, "" );
								strcpy( Elem[0].Name, "" );
								Elem[0].Type = T_EMPTY;
							}

/* ha ajt�volt �s most nem az akkor el�sz�r t�r�lni kell, majd l�trehozni */
							if( (OldType == T_DOOR) && (Type != T_DOOR) ){
								DeleteElem( ActElem );

								SumElem += 1;
								ActElem = SumElem;
								Elem[ActElem].Type = Type;
								strcpy( Elem[ActElem].Title, StringBuffer1 );
								strcpy( Elem[ActElem].Name,  StringBuffer2 );
								Page[ActPage].Elem[Gnum] = ActElem;

								if( Type != T_HEADLINE )
									Elem[ActElem].Page = ActPage;
							}


/* �resr�l �resre -> stringeket t�rli */
							if( (OldType == T_EMPTY) && (Type == T_EMPTY) ){
								strcpy( Elem[0].Title, "" );
								strcpy( Elem[0].Name, "" );
								ElemInfo();
							}
							DrawPage();
							return;
							break;
		}
	}
}

FillPageToEmpty( Pagenum )
UWORD Pagenum;
{
	UWORD i;

	for(i=0;i<MAXGADS;i++)
		Page[Pagenum].Elem[i] = 0;

}

DeleteElem( Elemnum )
UWORD Elemnum;
{
	UWORD i,k = 0;

	if(Elemnum == 0)
		return;

/* ha ebb�l az elemb�l ny�lik m�g egy lap akkor azt t�rli */
/* a l�ncol�sban szerepl� Next,Prev Lapokat is t�rli  */


	if( Elem[Elemnum].Type == T_DOOR ){

		i = Elem[Elemnum].Page;

/* elmegy a l�ncon a  v�g�re */

			while( k = Page[i].Next )
				i = k;
			k = i;

/* visszafel� t�rli a lapokat */

			do{
				i = Page[k].Prev;
				DeletePage( k );
				k = i;
			}while( k );
	}

/* lapokon a hivatkoz�sok �t�r�sa, ha az elem sorsz�ma v�ltozna */

	for( i=1 ; i<=SumPage ; i++ )
		for( k=0 ; k<MAXGADS ; k++ )
			if( Page[i].Elem[k] > Elemnum )
				Page[i].Elem[k] -= 1;

/* t�mb �tmozgat�sa, ha a torlendo elem nem az utolso */

	if( Elemnum != SumElem )
		for(i=Elemnum ; i< SumElem ; i++ ){
			Elem[i].Type = Elem[i+1].Type;
			Elem[i].Page = Elem[i+1].Page;
			strcpy( Elem[i].Title , Elem[i+1].Title );
			strcpy( Elem[i].Name  , Elem[i+1].Name  );
		}
	else
		i = Elemnum;

	Elem[i].Type = 0;
	Elem[i].Page = 0;
	strcpy( Elem[i].Title , "" );
	strcpy( Elem[i].Name  , "" );

	ActElem = 0;
	SumElem -= 1;
	
}

DeletePage( Pagenum )
UWORD Pagenum;
{
	UWORD tmppage,i = 0;

	if( (Pagenum == 0) || (Pagenum == 1) )
		return;

/* a lap elemeit t�r�lni */

	for(i=0;i<MAXGADS;i++)
		DeleteElem( Page[Pagenum].Elem[i] );

/* azonos szinten a l�ncban l�v� lapokat �tsorsz�mozni */

	tmppage = Pagenum;

	while( tmppage = Page[tmppage].Next )
		Page[tmppage].Number -= 1;

/* a l�ncolasb�l kit�r�lni : Next,Prev */	

	if( Page[Pagenum].Prev )
		if( Page[Pagenum].Next )
			Page[ Page[Pagenum].Prev ].Next = Page[Pagenum].Next;
		else
			Page[ Page[Pagenum].Prev ].Next = 0;

	if( Page[Pagenum].Next )
		if( Page[Pagenum].Prev )
			Page[ Page[Pagenum].Next ].Prev = Page[Pagenum].Prev;
		else
			Page[ Page[Pagenum].Next ].Prev = 0;

/* Ha volt Parent lap , akkor az azon levo ide mutato elemet be kell allitani 0-ra */

	if( Page[Pagenum].Parent ){
		tmppage = Page[Pagenum].Parent;
		for(i=0;i<MAXGADS;i++)
			if( (Elem[ Page[ tmppage ].Elem[i] ].Type == T_DOOR ) && (Elem[ Page[ tmppage ].Elem[i] ].Page == Pagenum) )
				Elem[ Page[ tmppage ].Elem[i] ].Page = 0;
	}


/* a l�ncol�sb�l kit�r�lni : Parent,Next,Prev ! */

	for( i=1 ; i<=SumPage ; i++){

		if( Page[i].Parent > Pagenum )
			Page[i].Parent -= 1;
		if( Page[i].Next > Pagenum )
			Page[i].Next -= 1;
		if( Page[i].Prev > Pagenum )
			Page[i].Prev -= 1;
	}

/* az elem t�mbben a Page mez�t is be kell �ll�tani */

	for( i=1 ; i<=SumElem ; i++)
		if( Elem[i].Page > Pagenum )
			Elem[i].Page -= 1;


/*  ActPage,ActElem be�ll�t�sa */

	if( ActPage == Pagenum ){

		if( Page[ Pagenum ].Prev )
			ActPage = Page[ Pagenum ].Prev;

		if( Page[ Pagenum ].Next )
			ActPage = Page[ Pagenum ].Next;

		if( Pagenum == ActPage ){				/* Ha ezen a szinten o volt az utolso, akkor visszalep */
			ActPage = Page[ Pagenum ].Parent;
			Level -= 1;
		}
	}
	if( ActPage > Pagenum )
		ActPage -= 1;

	ActElem = Page[ ActPage ].Elem[0];

/* Lapok �tmozgat�sa */

	for( Pagenum ; Pagenum < SumPage ; Pagenum++){
		Page[ Pagenum ].Next   = Page[ Pagenum+1 ].Next;
		Page[ Pagenum ].Prev   = Page[ Pagenum+1 ].Prev;
		Page[ Pagenum ].Parent = Page[ Pagenum+1 ].Parent;
		Page[ Pagenum ].Number = Page[ Pagenum+1 ].Number;
		for(i=0 ; i<MAXGADS ; i++ )
			Page[ Pagenum ].Elem[i] = Page[ Pagenum+1 ].Elem[i];
	}

/* Utols� lap t�rl�se */
	Page[ Pagenum ].Next   = 0;
	Page[ Pagenum ].Prev   = 0;
	Page[ Pagenum ].Parent = 0;
	Page[ Pagenum ].Number = 0;
	FillPageToEmpty( Pagenum );

	SumPage -= 1;

}

AddPage()
{
	UWORD NewPage,tmppage;

	if( SumPage != (MAXPAGE-1) ){
		SumPage += 1;
		NewPage = SumPage;

		if( Page[ActPage].Next ){
			Page[NewPage].Next = Page[ActPage].Next;
			Page[ Page[ActPage].Next ].Prev = NewPage;
		}
		Page[ ActPage ].Next = NewPage;
		Page[ NewPage ].Prev = ActPage;
		Page[ NewPage ].Parent = Page[ ActPage ].Parent;
		Page[ NewPage ].Number = Page[ ActPage ].Number + 1;
		FillPageToEmpty( NewPage );

/*  �tsorsz�mozza az ut�na k�vetkez�ket	*/

		tmppage = NewPage;

		while( tmppage = Page[tmppage].Next )
			Page[tmppage].Number += 1;

		ActPage = NewPage;
		ActElem = 0;

		DrawPage();
		ElemInfo();

	}
	else
		ShowMyReq( NULL, sajna , " Nincs t�bb hely ! ");
}

AddSubPage()
{
UWORD NewPage;

	if( SumPage != (MAXPAGE+1) ){
		SumPage += 1;
		NewPage = SumPage;

		Page[ NewPage ].Parent = ActPage;
		Page[ NewPage ].Number = 1;

		FillPageToEmpty( NewPage );
		Elem[ ActElem ].Page = NewPage;

		ActPage = NewPage;
		ActElem = 0;

	}
	else
		ShowMyReq( NULL, sajna , " Nincs t�bb hely ! ");
}


Menu( code ,Gnum )
USHORT code;
UWORD Gnum;
{

	ClearMenuStrip( window );

	if(MENUNUM(code) != MENUNULL)
		switch(MENUNUM(code)){
		case	0:
			switch(ITEMNUM(code)){
			case	0:
					LoadFile();
					ActPage = 1;
					ActElem = Page[ ActPage ].Elem[0];
					Level   = 1;
					DrawPage();
					ElemInfo();
					break;
	
			case	1:
					SaveFile();
					break;

			case	2:
					ShowMyReq( NULL," Kedvencem a GURU !!! ",About);
					break;

			case	3:
					if ( ShowMyReq( igen, nem ," Biztosan ki akarsz l�pni ? ") ){
						CleanUp();
						exit( TRUE );
					}
					break;
			}
			break;


		case	1:
			InputElem( Gnum );
			break;


		case	2:
			switch(ITEMNUM(code)){
			case	0:
					AddPage();
					break;
			case	1:
					if( ShowMyReq( igen,nem," Biztosan t�r�lni akarod ezt a lapot ? ") ){
						DeletePage( ActPage );
						DrawPage();
						ElemInfo();
					}
					break;
			}
			break;

		case	3:
			switch(ITEMNUM(code)){
			case	0:

					if( Page[ActPage].Next ){
						ActPage = Page[ActPage].Next;
						ActElem = Page[ActPage].Elem[0];
						DrawPage();
						ElemInfo();
					}
					else
						ShowMyReq( NULL,sajna," Nincs t�bb lap ezut�n ! ");
					break;

			case	1:
					if( Page[ActPage].Prev ){
						ActPage = Page[ActPage].Prev;
						ActElem = Page[ActPage].Elem[0];

						DrawPage();
						ElemInfo();
					}
					else
						ShowMyReq( NULL,sajna," Nincs t�bb lap ezel�tt ! ");
					break;
			}
			break;

		case	4:
			switch(ITEMNUM(code)){
			case	0:
					if( Elem[ActElem].Type == T_DOOR ){	
						if( Elem[ActElem].Page ){
							ActPage = Elem[ActElem].Page;
							ActElem = Page[ActPage].Elem[0];
						}
						else{
							AddSubPage();
						}
					Level += 1;
					DrawPage();
					ElemInfo();
					}
					else
						ShowMyReq( NULL,sajna," Ez nem ajt� ! ");
					break;

			case	1:
					if( Page[ActPage].Parent ){
						ActPage = Page[ActPage].Parent;
						ActElem = Page[ActPage].Elem[0];
						Level -= 1;
						DrawPage();
						ElemInfo();
					}

					else
						ShowMyReq( NULL,sajna," Ez a legfels� ! ");
					break;
			}
			break;

	}
	SetMenuStrip(window,&menu1);

}


/***************************************************************
 CleanUp(),OpenLibs(),OpenWorkWindow()

 Rendszerhivasok
***************************************************************/

CleanUp()
{
	if (Page)			FreeMem( (ULONG *)Page ,PageSize );
	if (Elem)			FreeMem( (ULONG *)Elem ,ElemSize );
	if (window){
		myprocess->pr_WindowPtr = olderrorwindow;
		CloseWindow(window);
	}
	if (ReqBase){
		CloseLibrary(ReqBase);
		PurgeFiles(&MyFileReqStruct);
	}
	if (DosBase)		CloseLibrary(DosBase);
	if (IntuitionBase)	CloseLibrary(IntuitionBase);
	if (GfxBase)		CloseLibrary(GfxBase);
}

OpenLibs()
{
	if (!(IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library", 0L))){
		CleanUp();
		exit(FALSE);
	}
	if (!(GfxBase = (struct GfxBase *) OpenLibrary("graphics.library", 0L))){
		CleanUp();
		exit(FALSE);
	}

	if (!(DosBase = (struct DosBase *) OpenLibrary("dos.library", 0L))){
		CleanUp();
		exit(FALSE);
	}

	if (!(ReqBase = (struct ReqBase *)OpenLibrary("req.library", 0L))){
		CleanUp();
		exit(FALSE);
	}
}

OpenWorkWindow()
{
	if (!(window = (struct Window *)OpenWindow(&newwindow))){
		CleanUp();
		exit(FALSE);
	}

/********	req.library miatt , kil�p�sn�l vissza�l�tani *************/

	myprocess = (struct Process *)FindTask((char *)0);
	olderrorwindow = myprocess->pr_WindowPtr;
	myprocess->pr_WindowPtr = (APTR)window;

	rp = window->RPort;

	SetMenuStrip(window,&menu1);


	strcpy( filename, "Tartalom");

	SetDrMd( rp , JAM2 );
	SetAPen( rp , 1 );
	SetBPen( rp , 0 );

	SetAPen( rp , 3 );

	Move( rp, 10 , 210 );
	Draw( rp, 630 , 210 );

	Move( rp, 16 , 220 +7 );
	Text( rp, "Szint   : ",10 );

	Move( rp, 16 , 230 +7 );
	Text( rp, "Lapsz�m : ",10 );

	Move( rp, 330 - 10*8 , 220+7 );
	Text( rp, "T�pus   : ",10 );
	Move( rp, 330 - 10*8 , 230+7 );
	Text( rp, "C�m     : ",10 );
	Move( rp, 330 - 10*8 , 240+7 );
	Text( rp, "F�jln�v : ",10 );

	SetAPen( rp , 1 );
}


main()
{
	ULONG MessageClass;
	USHORT code;

	UWORD ActGadget = 0;		/* �rt�ke 0-21 az utolj�ra kijel�lt gadget */

	Level = 1;

	OpenLibs();
	InitGads();
	OpenWorkWindow();

	InitPageStruct();   

	ActPage = 0;
	ActElem = 0;

	SumPage = 0;
	SumElem = 0;

	AddPage();
	 
	 
	FOREVER
	{
		if ((message = (struct IntuiMessage *) GetMsg(window->UserPort)) == NULL){
			Wait(1L << window->UserPort->mp_SigBit);
			continue;
		}
		MessageClass = message->Class;
		code = message->Code;
		ReplyMsg(message);
		switch (MessageClass){

			case GADGETUP	  :
		 	case GADGETDOWN  :
								ActGadget = ( (struct Gadget *)(message->IAddress) )->GadgetID;
								ActElem	 = Page[ActPage].Elem[ActGadget];

								ElemInfo( ActElem );

								if( Elem[ ActElem ].Type == T_EMPTY )
									InputElem( ActGadget );
								break;

		case MENUPICK	  :
								Menu( code , ActGadget );
								break;
		 
		case CLOSEWINDOW :
								if ( ShowMyReq( igen, nem ," Biztosan ki akarsz l�pni ? ") ){
									CleanUp();
									exit( TRUE );
								}
								break;

		}
	}
}
