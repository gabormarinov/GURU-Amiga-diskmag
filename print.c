#include <stdio.h>
#include <exec/types.h>
#include <exec/exec.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/preferences.h>
#include <devices/printer.h>
#include <functions.h>


#define T_ARTICLE	4		/*  Cikk 					*/
#define T_LIST		5		/*  Programlista			*/

extern	BOOL	CheckMessageWindow();

extern	UBYTE		*LineLength;
extern	unsigned	*LinePos;
extern	ULONG		Lines;

#define	C_RETURN 0x0a
#define RESERVED_LINES 7

char	*PrintAddr;
long	PrintLength;

struct Preferences		*Prefs;
struct Preferences		*PrefsBuf;

struct MsgPort		*printerPort;
struct IOStdReq		PrintRequest;
struct IOPrtCmdReq	PrefRequest;

char	*PrintHeader	= "[1m                                                                                [22m\n";
char	*PrintUnderLine	= "--------------------------------------------------------------------------------\n";
char	*PrintPageNumber= "                                      .oldal\n";
char	*PageNumber		= "     ";
char	*Spaces			= "                                                               ";
char	*FormFeed		= "\014";

SCopy( from, to, count )
register	char	*from,*to;
register	unsigned count;
{
	for(;count!=0;count--)
		*to++=*from++;
}

char *PutTo( from,to, count )
register char *from,*to;
register unsigned count;
{
	for( ;count!=0 ; count-- )
		*to++ = *from++;

	return( to );
}

char *PutReturn( to )
register char *to;
{
	*to++ = C_RETURN;
	return( to );
}



/*****************************************
FormList() - nyomtat�sra k�sz form�ra hoz
		   egy list�t.

hasz�lja a 
	LinePos,LineLength t�mb�ket,
	Lines glob�lis v�ltoz�t

ki:
	*PrintAddr,PrintLength - glob�lis v�ltoz�k;

	TRUE  ha siker�lt fel�p�teni,
	FALSE ha nem.
******************************************/


BOOL FormList( Date, Title, Addr, ActPaperLines )
char	*Date,*Title;
char	*Addr;
UWORD	ActPaperLines;
{

	UWORD	PageNum,PrintPages,LastReturns;
	ULONG	line;
	register	ULONG	i;
	register	char	*BuffPointer;


/* A foglaland�mem�ria m�ret�nek kisz�m�t�sa */

	PrintPages	= Lines/ActPaperLines;


/* Mennyi lapot fog nyomtatni, LastReturn -> utols� sor ut�n mennyi RETURN kell a lap alj�ig  */

	LastReturns = ActPaperLines - (Lines % ActPaperLines);
	if( LastReturns != 0 )
		PrintPages += 1;

	PrintLength = 0;
	for(line=0;line<Lines;line++)
		PrintLength += (LineLength[line]+1);		/* +1 a RETURN miatt */

	PrintLength	+= PrintPages*( strlen( PrintHeader )+strlen( PrintUnderLine )+strlen( PrintPageNumber )+5 ); /* 2+2+1 */
	PrintLength	+= LastReturns;

#ifdef TEST
printf("Header      :%d\n",strlen(PrintHeader) );
printf("LastReturns :%d\n",LastReturns );
printf("PrintPages  :%ld\n",PrintPages  );
printf("PrintLength :%ld\n",PrintLength );
#endif

	PrintAddr = (char *)AllocMem( PrintLength,MEMF_PUBLIC );
	if( !PrintAddr ){
		Warning("Nics el�g mem�ria a nyomtat� pufferhez!");
		return( FALSE );
	}

/* A nyomtat�si ter�let felt�lt�se SPACE-karakterekkel */

	for( i=0;i<PrintLength;i++)
		PrintAddr[i] = ' ';

/*	fejl�c elk�sz�t�se */

	SCopy( Date , &PrintHeader[4], strlen( Date ) );
	SCopy( Title, &PrintHeader[ 84-strlen(Title) ],strlen(Title) );


	BuffPointer = PrintAddr;
	line	= 0;

	for( PageNum=1;PageNum<PrintPages+1;PageNum++ ){

		BuffPointer = PutTo( PrintHeader, BuffPointer , strlen( PrintHeader ) );
		BuffPointer = PutTo( PrintUnderLine  , BuffPointer , strlen( PrintUnderLine ) );
		BuffPointer = PutReturn( BuffPointer ); 
		BuffPointer = PutReturn( BuffPointer ); 


		for( i=0;i<ActPaperLines;i++ )
			if(line<Lines){
				BuffPointer = PutTo( &Addr[ LinePos[line] ],BuffPointer, LineLength[line]<81 ? (LineLength[line]+1) : 80 );
				line += 1;
			}
			else
				BuffPointer = PutReturn( BuffPointer );


/* lapsorsz�m */
		BuffPointer = PutReturn( BuffPointer );
		BuffPointer = PutReturn( BuffPointer ); 
		uwtoa( PageNum, PageNumber );
		SCopy( PageNumber , &PrintPageNumber[ 38-strlen(PageNumber) ] , strlen(PageNumber) );
		BuffPointer = PutTo( PrintPageNumber, BuffPointer ,strlen( PrintPageNumber ));
		BuffPointer = PutTo( FormFeed, BuffPointer ,1 );
	}

/*  fejl�c �s lapsz�m t�rl�se */

	SCopy( Spaces , &PrintPageNumber[ 38-strlen(PageNumber) ] , strlen(PageNumber) );
	SCopy( Spaces , &PrintHeader[ 84-strlen(Title) ],strlen(Title) );
	return( TRUE );
}



/*****************************************
FormArticle() - nyomtat�sra k�sz form�ra hoz
		   egy cikket.

haszn�lja a
	LinePos,LineLength t�mb�ket,
	Lines glob�lis v�ltoz�t

ki:
	*PrintAddr,PrintLength - glob�lis v�ltoz�k;

	TRUE  ha siker�lt fel�p�teni,
	FALSE ha nem.
******************************************/


BOOL FormArticle( Date, Title, Addr, ActPaperLines )
char	*Date,*Title;
char	*Addr;
UWORD	ActPaperLines;

{

	UWORD	PageNum,PrintPages,LastPageLines;
	ULONG	line,PagePos,PageLength;
	register	ULONG	i;
	register	char	*BuffPointer;


	PrintPages    = Lines / (ActPaperLines*2);

	if( (LastPageLines = Lines % (ActPaperLines*2)) != 0 )
		PrintPages += 1;


/* A foglaland�mem�ria m�ret�nek kisz�m�t�sa */

	PageLength  = ActPaperLines*81 + strlen( PrintHeader )+strlen( PrintUnderLine )+strlen( PrintPageNumber )+5; /* 5=2+2+1 */
	PrintLength = PrintPages * PageLength;

#ifdef TEST
printf("Header        :%d\n",strlen(PrintHeader) );
printf("LastPageLines :%d\n",LastPageLines );
printf("PrintPages    :%ld\n",PrintPages  );
printf("PrintLength   :%ld\n",PrintLength );
#endif

	PrintAddr = (char *)AllocMem( PrintLength,MEMF_PUBLIC );
	if( !PrintAddr ){
		Warning("Nics el�g mem�ria a nyomtat� pufferhez!");
		return( FALSE );
	}

/* A nyomtat�si ter�let felt�lt�se SPACE-karakterekkel */

	for( i=0;i<PrintLength;i++)
		PrintAddr[i] = ' ';

/*	fejl�c elk�sz�t�se */

	SCopy( Date , &PrintHeader[4], strlen( Date ) );
	SCopy( Title, &PrintHeader[ 84-strlen(Title) ],strlen(Title) );


	BuffPointer = PrintAddr;
	line	= 0;

	for( PageNum=1;PageNum<PrintPages+1;PageNum++ ){

	PagePos = (PageNum-1)*PageLength;   /* nyomtat� pufferben a lap els� karakter�nek pozici�ja */
	BuffPointer = &PrintAddr[ PagePos ];

		BuffPointer = PutTo( PrintHeader, BuffPointer , strlen( PrintHeader ) );
		BuffPointer = PutTo( PrintUnderLine  , BuffPointer , strlen( PrintUnderLine ) );
		BuffPointer = PutReturn( BuffPointer ); 
		BuffPointer = PutReturn( BuffPointer ); 


		for( i=0;i<ActPaperLines;i++ ){
			if(line<Lines){
				SCopy( &Addr[ LinePos[line] ],BuffPointer, LineLength[line]<40 ? LineLength[line]:39 );

				if( line+ActPaperLines < Lines )
					SCopy( &Addr[ LinePos[line+ActPaperLines] ],BuffPointer+41, LineLength[line+ActPaperLines]<40 ? LineLength[line+ActPaperLines]:39 );

				line += 1;
			}
			PutReturn( BuffPointer+80 );
			BuffPointer += 81;
		}
		line += ActPaperLines;
			

/* lapsorsz�m */
		BuffPointer = PutReturn( BuffPointer );
		BuffPointer = PutReturn( BuffPointer ); 
		uwtoa( PageNum, PageNumber );
		SCopy( PageNumber , &PrintPageNumber[ 38-strlen(PageNumber) ] , strlen(PageNumber) );
		BuffPointer = PutTo( PrintPageNumber, BuffPointer ,strlen( PrintPageNumber ));
		BuffPointer = PutTo( FormFeed, BuffPointer ,1 );
	}

/*  fejl�c �s lapsz�m t�rl�se */

	SCopy( Spaces , &PrintPageNumber[ 38-strlen(PageNumber) ] , strlen(PageNumber) );
	SCopy( Spaces , &PrintHeader[ 84-strlen(Title) ],strlen(Title) );
	return( TRUE );
}


Print( GuruDate, PrintName, Addr, Type )
char	*GuruDate,*PrintName;
char	*Addr;
UBYTE	Type;
{

	UWORD	PaperLines,ActPaperLines;
	BOOL	AbortPrint;
	int		PrintError;
	
	if( !ShowMessage( "Ellen�rizd a nyomtat�t !" , "Biztosan ki akarod nyomtatni ezt a cikket ?" , TRUE ,FALSE, FALSE ))
		return;	

	PrefsBuf=(struct Preferences *)AllocMem( sizeof( struct Preferences ),MEMF_PUBLIC|MEMF_CLEAR );
	if( !PrefsBuf ){
		Warning("Nincs el�g mem�ria a Preferences beolvas�s�ra !");
		return;
	}

	Prefs      = GetPrefs( PrefsBuf, sizeof( struct Preferences ) );
	PaperLines = Prefs->PaperLength;


#ifdef TEST
printf("Paper Length    : %d\n",Prefs->PaperLength );
#endif

	FreeMem( PrefsBuf,sizeof( struct Preferences ) );
	if( PaperLines <= RESERVED_LINES ){
		Warning("T�l kev�s sor van be�ll�tva a Preferences -ben!");
		return;
	}

	ActPaperLines = PaperLines-RESERVED_LINES;

	switch( Type ){
		case T_ARTICLE:
					if( !FormArticle( GuruDate, PrintName, Addr, ActPaperLines ) )
						return;
					break;

		case T_LIST:
					if( !FormList( GuruDate, PrintName, Addr, ActPaperLines ) )
						return;
					break;
		default:
					Warning("Mit akarsz kinyomtatni ?");
					return;
					break;
	}


/*****************************************************************************/


	printerPort = CreatePort("GuruPrinter.port",0);
	if( !printerPort ){
		Warning(" Nem tudom l�trehozni a printerPort -ot !");
		FreeMem( PrintAddr, PrintLength );
		return;
	}

	PrintError = OpenDevice("printer.device",0,&PrintRequest,0);
	if(PrintError != 0){
		Warning("Nem tudom megnyitni a printer.device -t!");
		DeletePort(printerPort);
		FreeMem( PrintAddr, PrintLength );
		return;
	}

	PrintRequest.io_Message.mn_ReplyPort = printerPort;
	PrintRequest.io_Command = CMD_WRITE;
	PrintRequest.io_Data    = (APTR)PrintAddr;
	PrintRequest.io_Length  = PrintLength;

	ShowMessage( "Nyomtat�s folyamatban...","Meg akarod szak�tani a nyomtat�st ?",FALSE,FALSE,TRUE );

	SendIO(&PrintRequest);

	AbortPrint = CheckMessageWindow();

	while( (!CheckIO(&PrintRequest)) && (!AbortPrint) )
		AbortPrint = CheckMessageWindow();

	if( AbortPrint ){
		PrintError = AbortIO( &PrintRequest );
		WaitIO( &PrintRequest );
	}

	CloseMessageWindow();
	CloseDevice( &PrintRequest );
	DeletePort( printerPort );
	FreeMem( PrintAddr, PrintLength );

}
