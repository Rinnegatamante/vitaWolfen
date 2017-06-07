#include "include/ff.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vitasdk.h>
#ifdef USE_DMALLOC
#include<dmalloc.h>
#endif

extern char path[256];
static signed char **dirfname;
static int dirpos;
static int dirsize;

static int wildcard(const signed char *name, const signed char *match)
{
	int i;
	int max;
	max=strlen(name);
	if(strlen(match)!=max) {
		return 0;
	}
	for(i=0;i<max;i++) {
		signed char a,b;
		a=name[i];
		b=match[i];
		if(a>='a' && a<='z') a^=32;
		if(b>='a' && b<='z') b^=32;
		if(b=='?' || a==b) {
			// continue
		} else {
			return 0;
		}
	}
	return 1;
}

int findfirst(const signed char *pathname, struct ffblk *ffblk, int attrib) {
	signed char *match=strdup(pathname);
	unsigned int i;
	if(match[0]=='*') match++;
	for(i=0;i<strlen(match);i++) { 
		if(match[i]>='a' && match[i]<='z') match[i]^=32; 
	}
	dirsize=0;
	printf("Looking for '%s' (%s)\n",match,pathname);
	int fd = sceIoDopen(path);
	SceIoDirent entry;
	while (sceIoDread(fd, &entry) > 0) {
		if(strcasestr(entry.d_name,match)==0 && wildcard(entry.d_name,match)==0) continue;
		if(dirsize==0) {
			dirfname=(signed char **)calloc(sizeof(signed char *),64);
		} else if((dirsize%64)==0) {
			dirfname=(signed char **)realloc(dirfname,sizeof(signed char *)*(dirsize+64));
		}
		dirfname[dirsize++]=strdup(entry.d_name);
	}
	sceIoDclose(fd);
	printf("Got %d matches\n",dirsize);
	if (dirsize>0) {
		dirpos=1;
		strcpy(ffblk->ff_name,dirfname[dirsize-1]);
		return 0;
	}

	return 1;

}

int findnext(struct ffblk *ffblk) {

  if (dirpos<dirsize) {
    strcpy(ffblk->ff_name,dirfname[dirpos++]);
    return 0;
  }
  return 1;

}

void resetinactivity(void) {
	//User::ResetInactivityTime();
}
