/*****************************************************************

Program:	files.c

			A GURU lemez�js�g f�jl t�lt�  modulja.
			
Szerz�:		Marinov G�bor	<<Gaborca>>

D�tum:		1990.10.11.

Utols�
m�dos�t�s:	1991.07.26. :PowerPacker b�v�t�s

******************************************************************/

#include <stdio.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <functions.h>
#include "ppdata.h"			/* RADon! */

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

#define EMPTYPOINTER 0
#define ARROWPOINTER 1
#define CLOCKPOINTER 2

long		FileSize		= 0;	/* Ezt az k�t v�ltoz�t �ll�totta be */
char		*FileAddr	= NULL;     /* a PP_LoadData, PP_LoadModule  */


long ExamineFile( Name )	
char	*Name;
{
	struct	FileLock		*lock 		= NULL;
	struct	FileInfoBlock	*FileInfo	= NULL;
	long	filesize;

	if ( !( lock = Lock( Name , ACCESS_READ ) )){
		return(0L);
	}
	FileInfo = (struct FileInfoBlock *)AllocMem( sizeof(struct FileInfoBlock)  , MEMF_CLEAR );
	Examine( lock , FileInfo );
	UnLock( lock );
	filesize = FileInfo->fib_Size;

#ifdef TEST
printf(" FileName : %s\n",&FileInfo->fib_FileName );
printf(" FileSize : %ld\n",filesize );
#endif

	FreeMem( FileInfo , sizeof(struct FileInfoBlock) );
	return( filesize );
}

UnLoadFile( Addr , Length )
char *Addr;
long Length;
{
	FreeMem( Addr , Length );
}

BOOL LoadFile( Name , Type )
char *Name;
UBYTE Type;
{
	char	LoadName[ 60 ];
	ULONG	MemType;

	char	*LoadAddr	= NULL;
	long	WholeSize	= 0;

	ULONG	ppError;
	int oldpointer;

	oldpointer=SetMyPointer( CLOCKPOINTER );

/* A bet�ltend� f�jl k�nyvt�rnev�nek hozz�ragaszt�sa  */
/* �s Mem�ria tipus�nak meghat�roz�sa  */

	switch( Type ){
		case T_ARTICLE:
					strcpy( LoadName , "CIKKEK:" );
					strcat( LoadName , Name );
					MemType = MEMF_CLEAR;
					break;

		case T_LIST:
					strcpy( LoadName , "LIST�K:" );
					strcat( LoadName , Name );
					MemType = MEMF_CLEAR;
					break;

		case T_MUSIC:
					strcpy( LoadName , "ZEN�K:" );
					strcat( LoadName , Name );
					MemType = MEMF_CHIP|MEMF_CLEAR;
					break;

		default:
					strcpy( LoadName , Name );
					MemType = MEMF_CLEAR;
					break;
	}

	if( Type == T_MUSIC )
		ppError = ppLoadModule( LoadName, DECR_POINTER, MemType, &FileAddr, &FileSize, NULL );
	else
		ppError = ppLoadData( LoadName, DECR_POINTER, MemType, &FileAddr, &FileSize, NULL );

	if( ppError == PP_LOADOK ){
		SetMyPointer( oldpointer );
		return( TRUE );
	}
	else
		switch( ppError ){
			case PP_OPENERR:
				Warning("PP: Nem tudom megnyitni a f�jlt !");
				break;

			case PP_READERR:
				Warning("PP: Hiba a f�jl olvas�sa k�zben !");
				break;

			case PP_NOMEMORY:
				Warning("PP: Nincs el�g mem�ria a f�jl bet�lt�s�hez !");
				break;

			case PP_CRYPTED:
				Warning("PP: A f�jl k�dolt !");
				break;

			case PP_PASSERR:
				Warning("PP: �rv�nytelen jelsz� !");
				break;

			default:
				Warning("PP: Nem tudom bet�lteni a f�jlt !");
				break;

		}
		FileAddr = NULL;
		FileSize = 0L;

		SetMyPointer( oldpointer );
		return( FALSE );
}
