#ifndef	__ID_US_H__
#define	__ID_US_H__

#define	MaxX	400	//320
#define	MaxY	240	//200

#define	MaxHighName	57
#define	MaxScores	7
typedef	struct {
	signed char name[MaxHighName + 1];
	int score;
	int completed, episode;
} HighScore;

#define	MaxString	128	// Maximum input string size

extern	boolean NoWait;
extern	word		PrintX,PrintY;	// Current printing location in the window
extern	word		WindowX,WindowY,// Current location of window
			WindowW,WindowH;// Current size of window

#define USL_MeasureString       VW_MeasurePropString
#define USL_DrawString          VW_DrawPropString


extern	HighScore	Scores[];

void US_Startup(),
				US_Shutdown(),
				US_InitRndT(boolean randomize),
				US_DrawWindow(word x,word y,word w,word h),
				US_ClearWindow(void),
				US_PrintCentered(signed char *s),
				US_CPrint(signed char *s),
				US_CPrintLine(signed char *s),
				US_Print(signed char *s),
				US_PrintUnsigned(longword n);
boolean	US_LineInput(int x,int y,signed char *buf,signed char *def,boolean escok,
				int maxchars,int maxwidth);
int				US_RndT();

void	USL_PrintInCenter(signed char *s,Rect r);

#endif
