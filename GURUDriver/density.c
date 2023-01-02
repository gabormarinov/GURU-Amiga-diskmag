#include <exec/types.h>
#include <devices/printer.h>
#include <devices/prtbase.h>

char
SetDensity(density_code)
ULONG density_code;
{
	extern struct PrinterData *PD;
	extern struct PrinterExtendedData *PED;

	/* SPECIAL_DENSITY     0    1    2    3    4    5    6    7 */
	static int XDPI[8] = {120, 120, 120, 240, 120, 240, 240, 240};
	static int YDPI[8] = {72, 72, 144, 72, 216, 144, 216, 216};
	static char codes[8] = {'L', 'L', 'L', 'Z', 'L', 'Z', 'Z', 'Z'};

	PED->ped_MaxColumns = 
		PD->pd_Preferences.PaperSize == W_TRACTOR ? 136 : 80;
	density_code /= SPECIAL_DENSITY1;
	/* default is 80 chars (8.0 in.), W_TRACTOR is 136 chars (13.6 in.) */
	PED->ped_MaxXDots =
		(XDPI[density_code] * PED->ped_MaxColumns) / 10;
	PED->ped_XDotsInch = XDPI[density_code];
	PED->ped_YDotsInch = YDPI[density_code];
	if ((PED->ped_YDotsInch = YDPI[density_code]) == 216) {
		PED->ped_NumRows = 24;
	}
	else if (PED->ped_YDotsInch == 144) {
		PED->ped_NumRows = 16;
	}
	else {
		PED->ped_NumRows = 8;
	}
	return(codes[density_code]);
}
