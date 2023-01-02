/*****************************************************************

Program:	iff.c

			A GURU lemezújság IFF-ILBM kép töltõ modulja.
			
Szerzõk:	Ujhelyi Gyula	<<API>>
			Marinov Gábor	<<Gaborca>>

Dátum:		1990.10.12.

Utolsó
módosítás:	1990.10.13.

******************************************************************/

#include <intuition/intuition.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <functions.h>

#define MAXBUFFER 32768
#define TR 0x1
#define FS 0x0
#define MAKE_HEADER(a,b,c,d) (((unsigned long)(a)<<24)+((unsigned long)(b)<<16)+((unsigned long)(c)<<8)+(unsigned long)(d))
#define FORM MAKE_HEADER('F','O','R','M')
#define ILBM MAKE_HEADER('I','L','B','M')
#define BMHD MAKE_HEADER('B','M','H','D')
#define CMAP MAKE_HEADER('C','M','A','P')
#define CAMG MAKE_HEADER('C','A','M','G')
#define BODY MAKE_HEADER('B','O','D','Y')

#define EMPTYPOINTER 0
#define ARROWPOINTER 1
#define CLOCKPOINTER 2

extern	long	ExamineFile();

extern	struct IntuitionBase	*IntuitionBase;
extern	struct GfxBase			*GfxBase;
extern	struct DosBase			*DosBase;
extern	UBYTE	Color;

extern	struct	Screen	*TopScreen, *BottomScreen, *LogoScreen;

struct FileLock			*lock		= NULL;
struct FileInfoBlock	*fileinfo	= NULL;
struct FileHandle		*FileHandle	= NULL;

struct Screen		*PicScreen = NULL;
struct Window		*PicWindow = NULL;

/* API IFF loader */
struct Screen		*ShowScreen = NULL;
struct Window		*ShowWindow = NULL;
struct NewScreen	ns;
struct NewWindow	nw;

long GlobBuf=0,ScBufferSize; 
char *ScBufPoi=NULL,*LocBufPoi;

char	LoadName[60];
long	PicSize = 0;

CloseIt(argh)
char	*argh;
{
	Warning( argh );

	if(FileHandle)	Close(FileHandle);
	if(lock)		UnLock(lock);
	if(ScBufPoi)	FreeMem(ScBufPoi,ScBufferSize);
	if(ShowScreen)	CloseScreen( ShowScreen );
	ShowScreen		= NULL;
	ShowWindow		= NULL;
}

BOOL Load(buf,number_of_bytes)
char *buf;
int number_of_bytes;
{
	register long a;

	for(a=1;a<=number_of_bytes;a++)
	{
		if(GlobBuf==0)
		{
			if((GlobBuf=Read(FileHandle,ScBufPoi,ScBufferSize))==-1)
				return( FALSE );
			LocBufPoi=ScBufPoi;
		}
		*buf++=*LocBufPoi++;
		GlobBuf--;
	}
	return( TRUE );
}

void Convert(source,destination)
char *source,*destination;
{
	register char a;

	for(a=0;a<=3;a++) *(destination+a)=*(source+a);
}

struct Screen *LoadIFF( Name )
char *Name;
{
	unsigned long ChunkLen,ChunkHeader;
	char BMHDFlag=FS,CMAPFlag=FS,CAMGFlag=FS,BODYFlag=FS;
	register char b;
	register int y;
	register long i;
	register unsigned char ByteCount,BytesPerRow;
	char *WohinDamit;
	static char Mask[7]={0,2,4,8,16,32,16};
	unsigned char Buffer[12];

	struct BitMapHeader
	{
		UWORD w,h;
		WORD  x,y;
		UBYTE Bitplanes;
		UBYTE Masking;
		UBYTE Compression;
		UBYTE PadByte;
		UWORD TransCol;
		UBYTE XAspect,YAspect;
		WORD  Width,Height;
	} Bmhd;
	struct CommodoreAmiga
	{
		UWORD PadWord;
		UWORD ViewModes;
	} Camg;
	unsigned char Colors[32*3];

	lock		= NULL;
	fileinfo	= NULL;
	FileHandle	= NULL;

	ShowScreen = NULL;
	ShowWindow = NULL;

	GlobBuf  = ScBufferSize = 0; 
	ScBufPoi = LocBufPoi = NULL;

	PicSize = 0;
	Buffer[0] = '\0';

		strcpy( LoadName , "KÉPEK:" );
		strcat( LoadName , Name );

		PicSize = ExamineFile( LoadName );
		if( PicSize==0 ){
			CloseIt("Nem tudom megnyitni a fájlt!");
			return( NULL );
		}

		ScBufferSize=PicSize>MAXBUFFER?MAXBUFFER:PicSize;

		if(!(FileHandle=Open(LoadName,MODE_OLDFILE))){
			CloseIt("Nem tudom megnyitni a fájlt!");
			return( NULL );
		}
		if(!(ScBufPoi=AllocMem(ScBufferSize,MEMF_CHIP|MEMF_CLEAR))){
			CloseIt("Nincs elég memória a kép betöltéséhez!");
			return( NULL );
		}
		if( !Load(Buffer,12) ){
			CloseIt("Hibás a fájl!");
			return( NULL );
		}
		Convert(Buffer,&ChunkHeader);
		if(ChunkHeader!=FORM){
			CloseIt("Nem IFF-fájl!");
			return( NULL );
		}
		Convert(&Buffer[8],&ChunkHeader);

		if(ChunkHeader!=ILBM){
			CloseIt("Nem ILBM-fájl!");
			return( NULL );
		}

		while(1)
		{
			if( !Load(Buffer,8)){
				CloseIt("Hibás a fájl!");
				return( NULL );
			}
			if(GlobBuf<=0)
				if(BMHDFlag==TR && BODYFlag==TR && CMAPFlag==TR)
				{

#ifdef TEST
puts(" Minden OK. az IFF töltõ vége elött ");
#endif

					if(FileHandle)	Close(FileHandle);
					if(lock)		UnLock(lock);
					if(ScBufPoi)	FreeMem(ScBufPoi,ScBufferSize);
					return( ShowScreen );
				}
				else
				{
					if (BMHDFlag==FS){
						CloseIt("Nem találom a BMHD-t!");
						return( NULL );
					}
					if (BODYFlag==FS){
						CloseIt("Nem találom a BODY-t!");
						return( NULL );
					}
					if (CMAPFlag==FS){
						CloseIt("Nem találom a CMAP-ot!");
						return( NULL );
					}
				}
			Convert(&Buffer[4],&ChunkLen);
			Convert(Buffer,&ChunkHeader);

			switch(ChunkHeader)
			{
				case BMHD: /* BitMapHeaDer */
					if(BMHDFlag==TR){
						CloseIt("Két BMHD!");
						return( NULL );
					}
					if(!Load(&Bmhd,ChunkLen)){
						CloseIt("Hibás a fájl!");
						return( NULL );
					}

					ns.LeftEdge = 0;
					ns.TopEdge  = 284;
					ns.Width    = Bmhd.Width;
					ns.Height   = Bmhd.Height;
					ns.Depth    = Bmhd.Bitplanes;
					ns.DetailPen= 1;
					ns.BlockPen = 0;
					ns.ViewModes = 0;
					if(ns.Width>320)  ns.ViewModes |= HIRES;
					if(ns.Height>256) ns.ViewModes |= LACE;
					ns.Type          = CUSTOMSCREEN|SCREENBEHIND;
					ns.Font          = NULL;
					ns.DefaultTitle  = NULL;
					ns.Gadgets       = NULL;
					ns.CustomBitMap  = NULL;

					if(!(ShowScreen=OpenScreen(&ns))){
						CloseIt("Nincs elég memória a képernyõhöz!");
						return( NULL );
					}

					nw.LeftEdge 	= 0;
					nw.TopEdge  	= 0;
					nw.Width    	= Bmhd.Width;
					nw.Height   	= Bmhd.Height;
					nw.DetailPen	= 0;
					nw.BlockPen 	= 0;
					nw.IDCMPFlags	= MOUSEBUTTONS;
					nw.Flags		= BORDERLESS|RMBTRAP;
					nw.FirstGadget	= NULL;
					nw.CheckMark	= NULL;
					nw.Title		= NULL;
					nw.Screen		= ShowScreen;
					nw.BitMap		= NULL;
					nw.MinWidth		= Bmhd.Width;
					nw.MinHeight	= Bmhd.Height;
					nw.MaxWidth		= Bmhd.Width;
					nw.MaxHeight	= Bmhd.Height;
					nw.Type			= CUSTOMSCREEN;

					if(!( ShowWindow = OpenWindow( &nw ) )){
						CloseIt("Nincs elég memória a kép-ablakhoz!");
						return( NULL );
					}

					BytesPerRow=Bmhd.Width>>3;
					BMHDFlag=TR;
					break;

				case CMAP: /* ColorMAP */
					if(CMAPFlag==TR){
						CloseIt("Két CMAP!");
						return( NULL );
					}
					if(BMHDFlag==FS){
						CloseIt("CMAP BMHD nélkül!");
						return( NULL );
					}
					if(!Load(Colors,ChunkLen)){
						CloseIt("Hibás a fájl!");
						return( NULL );
					}
					for(i=0;i<Mask[Bmhd.Bitplanes];i++)
						SetRGB4(&ShowScreen->ViewPort,i,Colors[i*3]>>4,Colors[i*3+1]>>4,Colors[i*3+2]>>4);

					CMAPFlag=TR;
					break;

				case CAMG: /* Commodore AMiGa */
					if(CAMGFlag==TR){
						CloseIt("Két CAMG!");
						return( NULL );
					}
					if(BMHDFlag==FS){
						CloseIt("CAMG BMHD nélkül!");
						return( NULL );
					}
					if( !Load(&Camg,ChunkLen)){
						CloseIt("Hibás a fájl!");
						return( NULL );
					}
					ShowScreen->ViewPort.Modes=Camg.ViewModes;
					RemakeDisplay();
					CAMGFlag=TR;
					break;

				case BODY: /* BODY */
					if(BODYFlag==TR){
						CloseIt("Két BODY!");
						return( NULL );
					}
					if(BMHDFlag==FS){
						CloseIt("BODY BMHD nélkül!");
						return( NULL );
					}
					for (y=0;y<Bmhd.Height;y++)
						for (b=0;b<Bmhd.Bitplanes;b++)
						{
							ByteCount=0;
							WohinDamit=(char *)ShowScreen->RastPort.BitMap->Planes[b]+y*BytesPerRow;

							if(Bmhd.Compression==0)
								if( !Load(WohinDamit,BytesPerRow)){
									CloseIt("Hibás a fájl!");
									return( NULL );
								}
							if(Bmhd.Compression==1)
								while(ByteCount<BytesPerRow)
								{
									if( !Load(Buffer,1)){
										CloseIt("Hibás a fájl!");
										return( NULL );
									}
									if(Buffer[0]<128)
									{
										if( !Load(WohinDamit+ByteCount,Buffer[0]+1)){
											CloseIt("Hibás a fájl!");
											return( NULL );
										}
										ByteCount+=Buffer[0]+1;
									}

									/* Buffer[0] == 128 => Nop */

									if(Buffer[0]>128)
									{
										if( !Load(&Buffer[1],1)){
											CloseIt("Hibás a fájl!");
											return( NULL );
										}
										for(i=ByteCount;i<(ByteCount+257-Buffer[0]);i++)
											*(WohinDamit+i)=Buffer[1];
										ByteCount+=257-Buffer[0];
									}
								}
						}
						BODYFlag=TR;
						break;

				default:	 /* other chunkheaders are ignored */
					if(!Load(LocBufPoi,ChunkLen)){
						CloseIt("Hibás a fájl!");
						return( NULL );
					}
			}
		}
}
ShowPic( PicName )
char *PicName;
{

	struct	IntuiMessage	*message = NULL;
	ULONG	MessageClass	= 0L;
	USHORT	code			= 0;
	USHORT	NullPointer = 0;

	CloseScreens();

	PicScreen = NULL;
	PicWindow = NULL;

	SetMyPointer( CLOCKPOINTER );
	if( !(PicScreen = LoadIFF( PicName )) ){
		OpenScreens();
		SetMyPointer( ARROWPOINTER );
		return;
	}
	PicWindow = PicScreen->FirstWindow;
	SetMyPointer( ARROWPOINTER );

	ScreenToFront( PicScreen );
	ScreenLongUp(  PicScreen );

	do{
		if(message = (struct IntuiMessage *)GetMsg(PicWindow->UserPort)){
				MessageClass = message->Class;
				code = message->Code;
				ReplyMsg(message);
		}
	}while( MessageClass!=MOUSEBUTTONS );

	ScreenLongDown( PicScreen );
	CloseWindow( PicWindow );
	CloseScreen( PicScreen );

	PicScreen = NULL;
	PicWindow = NULL;

	OpenScreens();
}
