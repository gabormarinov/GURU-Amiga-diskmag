/********************************************************************
*                                                                   *
*  'PP_LoadData' PowerPacker DATA file support function V2.0        *
*                                                                   *
*  You may use this code for non-commercial purposes provided this  *  
*  copyright notice is left intact !                                *
*                                                                   *
*               Copyright (c) Apr 1990 by Nico François (PowerPeak) *
*																	*
*  'PP_LoadModule'													*
*  modifyed for GURU by Gaborca (Sorry Mr. PowerPeak!)  Jul 1991    *
*  																	*
*********************************************************************/

/*
  Miért kellett módosítani eme remek függvényt?
  Az Intuitiontracker zenelejátszó egy rendkívül sajátos trükkje miatt:
  A betöltött modul elé kell kerülnie, mert benne midnen címzés PC relatív, 
  és feltételezi, hogy maga után ( 0x09ec bytes ) a modul következik...
*/

#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <functions.h>

#include "ppdata.h"

#define MUSIC_PLAYER_SIZE 0x09ecL
#define SAFETY_MARGIN	MUSIC_PLAYER_SIZE

#define SIZEOF			(ULONG)sizeof
#define myRead(to,len)	if (Read (pp_lock, (char *)to, len) != len) {\
									pp_FreeStuff(); return (PP_READERR); }

extern UWORD ppCalcChecksum();
extern ULONG ppCalcPasskey();
extern void ppDecrunchBuffer();
extern void ppDecrypt();


static BPTR pp_lock;
static struct FileInfoBlock *pp_FileInfoBlock;
static UBYTE *pp_filestart;
static ULONG pp_bufferlen;
static ULONG pp_coladdr[5] = { 0xdff180, 0xdff182, 0xdff1a2, 0xdff102, 0 };

void pp_FreeStuff();

ppLoadModule (pp_file, color, typeofmem, buffer, length, pw)		/* Version 2.0 */
char *pp_file;
UBYTE color;
ULONG typeofmem;
UBYTE **buffer;
ULONG *length;
char *pw;
{
	ULONG pp_passchecksum, pp_filelen, pp_crunlen, pp_efficiency;
	UBYTE pp_crunched, pp_crypt = FALSE;
	ULONG pp_seek, pp_hdr, dummy;

	pp_coladdr[4] = (ULONG)&dummy;		/* dummy for decrunch color 'None' */
	pp_filestart = NULL;
	if (!(pp_FileInfoBlock = (struct FileInfoBlock *)AllocMem
		(SIZEOF(*pp_FileInfoBlock), MEMF_PUBLIC))) return (PP_NOMEMORY);

	if (!(pp_lock = (BPTR)Lock (pp_file, ACCESS_READ))) {
		pp_FreeStuff();
		return (PP_LOCKERR);
		}
	Examine (pp_lock, pp_FileInfoBlock);
	UnLock (pp_lock);
	pp_crunlen = pp_FileInfoBlock->fib_Size;

	/* read decrunched length */
	if (!(pp_lock = (BPTR)Open (pp_file, MODE_OLDFILE))) {
		pp_FreeStuff();
		return (PP_OPENERR);
		}
	myRead (&pp_hdr, 4L);

	/* check if crunched */
	if (pp_hdr == 'PX20' || pp_hdr == 'PP20') {
		if (pp_hdr == 'PX20') {
			if (!pw) {
				pp_FreeStuff();
				return (PP_CRYPTED);
				}
			myRead (&pp_passchecksum, 2L);
			if (ppCalcChecksum (pw) != pp_passchecksum) {
				pp_FreeStuff();
				return (PP_PASSERR);
				}
			pp_crypt = TRUE;
			pp_seek = 6L;
			}
		else pp_seek = 4L;
		Seek (pp_lock, pp_crunlen - 4L, OFFSET_BEGINNING);
		myRead (&pp_filelen, 4L);
		pp_filelen >>= 8L;
		pp_crunlen -= 4L + pp_seek;
		Seek (pp_lock, pp_seek, OFFSET_BEGINNING);
		myRead (&pp_efficiency, 4L);
		pp_bufferlen = pp_filelen + SAFETY_MARGIN;
		pp_crunched = TRUE;
		}
	else {
		Seek (pp_lock, 0L, OFFSET_BEGINNING);

/* HACK #1, akkor is helyesen mûködik, ha a modul nincs tömörítve */

		pp_filelen   = pp_crunlen;
		pp_bufferlen = pp_filelen + MUSIC_PLAYER_SIZE;
		pp_crunched  = FALSE;
		}
	if (!(pp_filestart=(UBYTE *)AllocMem (pp_bufferlen, typeofmem))) {
		pp_FreeStuff();
		return (PP_NOMEMORY);
		}

/* load file */
/* HACK #2 */

	if (pp_crunched){
		myRead ( pp_filestart, pp_crunlen);
	}
	else{
		myRead ( (pp_filestart+MUSIC_PLAYER_SIZE) , pp_crunlen);
	}

	Close (pp_lock);
	FreeMem (pp_FileInfoBlock, SIZEOF(*pp_FileInfoBlock));

	if (pp_crunched) {
		if (pp_crypt)
			ppDecrypt (pp_filestart, pp_crunlen-4L, ppCalcPasskey (pw));
		ppDecrunchBuffer (pp_filestart + pp_crunlen,
					pp_filestart + SAFETY_MARGIN, &pp_efficiency, pp_coladdr[color]);

/*  HACK #3  

		FreeMem (pp_filestart, SAFETY_MARGIN);
		pp_filestart += SAFETY_MARGIN;
*/
		}
	*buffer = pp_filestart;
	*length = pp_filelen+MUSIC_PLAYER_SIZE;
	return (PP_LOADOK);
}

void pp_FreeStuff()
{
	if (pp_lock) Close (pp_lock);
	if (pp_filestart) FreeMem (pp_filestart, pp_bufferlen);
	if (pp_FileInfoBlock) FreeMem (pp_FileInfoBlock, SIZEOF(*pp_FileInfoBlock));
	pp_lock = NULL; pp_filestart = NULL; pp_FileInfoBlock = NULL;
}
