/*****************************************************************

Program:	guru.c

			A GURU lemezújság cikk-kiíró modulja.
			
Szerzõ:		Marinov Gábor	<<Gaborca>>

Dátum:		1990.07.29.

Utolsó
módosítás:	1991.04.01

******************************************************************/

#include <stdio.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <intuition/intuition.h>

#include <functions.h>

#define T_ARTICLE	4		/*  Cikk 					*/

#define	C_RETURN	0x0a
#define	C_SPACE		0x20
#define	C_TAB		0x09

#define	C_NORM		0x01
#define	C_HIGH		0x02

extern	BOOL		CheckMessageWindow();
extern	BOOL		ShowMessage();

extern	BOOL		LoadFile();
extern	long		FileSize;			/* Ezt az két változót állította be */
extern	char		*FileAddr;	    	/* a LoadFile(), ha TRUE volt a visszt.érték  */
extern	char		*Date;

extern	struct	RPort *TopRast;
extern	struct	RPort *MiddleRast;
extern	struct	RPort *BottomRast;

extern	struct	Image icon[];

BOOL		NextPage,PrevPage;

UBYTE		*LineLength	= NULL;
unsigned	*LinePos	= NULL;
ULONG		Lines		= 0;

char		*text		= NULL;
long		textlength	= 0;


/***********************************************************************
 CountLines()

 Megszámolja a Text-ben levõ sorokat,és azok hosszát.
 Memóriát foglal a LinePos ( sorok elsõ karakterének pozíciója ),
 és a LineLength ( sor hossza ) tömböknek.
 A memória felszabadításáról gondoskodni kell!

 be: Text , Length
 ki: Lines, LinePos, LineLength

************************************************************************/

BOOL CountLines( Text , Length )
char		*Text;
long		Length;
{
	register	unsigned	pos;
	register	ULONG		line = 0;
	register	long		RealLength;  /* az utolsó RETURN utáni karakterek nélkül */

/*  sorok megszámlálása  */

	Lines = 0;
	for( pos = 0; pos<Length; pos ++ )
		if( Text[ pos ] == C_RETURN )
			Lines += 1;

#ifdef TEST
printf("Lines : %ld\n",Lines );
#endif

	LineLength	= NULL;
	LinePos		= NULL;

/* memória foglalása a LineLength tömbhöz */

	if (!(LineLength = ( UBYTE * )AllocMem( Lines * sizeof(UBYTE) , MEMF_PUBLIC|MEMF_CLEAR )) ){
		Warning("Nincs elég memória a LineLength tömbhöz !");
		return( FALSE );
	}

/* memória foglalása a LinePos tömbhöz */

	if (!(LinePos = ( unsigned * )AllocMem( Lines * sizeof(unsigned) , MEMF_PUBLIC|MEMF_CLEAR )) ){
		FreeMem( LineLength ,Lines * sizeof(UBYTE));
		Warning(" Nincs elég memória a LinePos tömbhöz ! ");
		return( FALSE );
	}



/*  az utolsó RETURN utáni karakterek figyelmen kívül hagyása */

	RealLength = Length;

	if( Lines!=0 ){
		pos = Length-1;					/* utolsó karakter */
		while( Text[pos--] != C_RETURN )
			RealLength -= 1;
	}

/*  a sorok hosszának és kezdõcímének tárolása a LineLength,LinePos tömbben  */	
/*  valamint a TAB karakterek lecserélése SPACE-re */

	line = 0;
	LinePos[line] = 0;

	for( pos = 0; pos<RealLength ; pos++ ){   /* Biztos hogy RETURN az utolsó */
		switch( Text[pos] ){

		case C_RETURN:
				line += 1;			/* mindig a következõ sorra mutat */
				if( line<Lines )
					LinePos[ line ] = pos + 1;
				break;

		case C_TAB:
				Text[pos] = C_SPACE;

		default:	
				LineLength[line] += 1;  
				break;
		}
	}
	return( TRUE );
}


DrawPage( Number )
unsigned Number;
{
	register UBYTE i,row;
	register unsigned lineindex;

	Move( MiddleRast , 0,0 );
	ClearScreen( MiddleRast );

/*******************   kéthasábos cikkek  ************************/

	lineindex = 30 * Number;
	row = 0 + 7;
	for( i=0; i<15; i++ ){
		if( LineLength[ lineindex ] != 0 ){
			SetAPen( MiddleRast , 1 );
			Move( MiddleRast , 0+2 , row+2 );
			Text( MiddleRast , &text[ LinePos[lineindex] ] , LineLength[ lineindex ]	);
			SetAPen( MiddleRast , 6 );
			Move( MiddleRast , 0 , row );
			Text( MiddleRast , &text[ LinePos[lineindex] ] , LineLength[ lineindex ]	);
		}
		row += 11;
		lineindex += 1;
		if( lineindex == Lines ){
			NextPage = FALSE;
			DeleteIcon( 101 );

		if( PrevPage )
			DrawImage( TopRast,&icon[19],0,0 );	/* elõzõ lap */
		else
			DeleteIcon( 100 );					/* következõ lap */

		return;
		}
	}

#ifdef TEST
printf(" lineindex : %d\n" , lineindex );
#endif

	row = 0 + 7;
	for( i=0; i<15; i++ ){
		if( LineLength[ lineindex ] != 0 ){
			SetAPen( MiddleRast, 1 );
			Move( MiddleRast , 328+2 , row +2);
			Text( MiddleRast , &text[ LinePos[lineindex] ] , LineLength[ lineindex ]	);
			SetAPen( MiddleRast, 6 );
			Move( MiddleRast , 328 , row );
			Text( MiddleRast , &text[ LinePos[lineindex] ] , LineLength[ lineindex ]	);
		}
		row += 11;
		lineindex += 1;
		if( lineindex == Lines ){
			NextPage = FALSE;
			DeleteIcon( 101 );

		if( PrevPage )
			DrawImage( TopRast,&icon[19],0,0 );	/* elõzõ lap */
		else
			DeleteIcon( 100 );					/* következõ lap */

			return;
		}
	}

	if( PrevPage )
		DrawImage( TopRast,&icon[19],0,0 );		/* elözõ lap */
	else
		DeleteIcon( 100 );						/* követlezõ lap */

	DrawImage( TopRast,&icon[20],0,0 );			/* következõ lap */
}


/**************************************************************/

LoadAndShowArticle( LoadName , ArtName )
char *LoadName;
char *ArtName;
{
	unsigned	PageNumber;
	register	ULONG pos;

	ULONG	i;

	int		ActGadget;

	if ( !(LoadFile( LoadName, T_ARTICLE ) ) )
		return;
	text 		= FileAddr;
	textlength 	= FileSize;

	if ( !CountLines( text , textlength ))
		return;

	PrevPage = FALSE;
	if( Lines > 30 )		/* Lines>30 kéthasábnál , Lines>15 egyhasábnál */
		NextPage = TRUE;
	else
		NextPage = FALSE;

	PageNumber = 0;

	WriteHeader( ArtName );
	DrawPage( PageNumber );
	TopShortUp();

	for(;;){
		ActGadget = GetGadget();
		switch( ActGadget ){

			case 1:
					if( PrevPage ){
						PageNumber -= 1;
						if( PageNumber == 0 )
							PrevPage = FALSE;
						NextPage = TRUE;
						TopShortDown();
						DrawPage( PageNumber );
						TopShortUp();
					}
					break;

			case 2:
					if( NextPage ){
						PageNumber += 1;
						PrevPage = TRUE;
						TopShortDown();
						DrawPage( PageNumber );
						TopShortUp();
					}
					break;

			case 3:
					UnLoadFile( text , textlength );
					FreeMem( LineLength , Lines * sizeof(UBYTE)   );
					FreeMem( LinePos    , Lines * sizeof(unsigned));
					TopShortDown();
					return;
					break;
			case 4:
					Print( Date, ArtName, text, T_ARTICLE );
					break;

			default:
					Preferences( ActGadget );
					break;		

		}
	}
}
