
#include <windows.h>
#include <shlwapi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "unrarlib.h"

extern int MystrCmp(char *,char *);

int UnRarToMemExec(char *fn, unsigned char **buff, unsigned long *ucsize, const char **szExtensions){
	char infn[MAX_PATH], *extp, flag;
	unsigned long datasize;
	int i, filen=0;
	ArchiveList_struct *List = NULL, *List2=NULL;

	filen = urarlib_list(fn, (ArchiveList_struct*)&List);
	if(!filen || !List){
		return 0;
	}
	for(i=filen, flag=0;i&&List;--i){
		List2=List;
		if(!flag){
			extp=PathFindExtension(List2->item.Name);
			++extp;
			for(int j=0; szExtensions[j]; ++j){
				if(MystrCmp(extp, (char *)&szExtensions[j][2])){
					strcpy(infn, List2->item.Name);
					flag=1;
					break;
				}
			}
		}
		List=List2->next;
		free(List2);
	}
	if(!flag)
		return 0;
	if(urarlib_get(buff, &datasize, infn, fn, NULL)){
		if(ucsize){
			*ucsize = datasize;
		}
		return 1;
	}
	else{
		return 0;
	}
	return 0;
}


int UnRarFindFileFromExtention(char *fn, const char **szExtensions){
	char infn[MAX_PATH], *extp, flag;
	int i, filen=0;
	ArchiveList_struct *List = NULL, *List2=NULL;

	filen = urarlib_list(fn, (ArchiveList_struct*)&List);
	if(!filen || !List){
		return 0;
	}
	for(i=filen, flag=0;i&&List;--i){
		List2=List;
		if(!flag){
			extp=PathFindExtension(List2->item.Name);
			++extp;
			for(int j=0; szExtensions[j]; ++j){
				if(MystrCmp(extp, (char *)&szExtensions[j][2])){
//					strcpy(infn, List2->item.Name);
					flag=1;
					break;
				}
			}
		}
		List=List2->next;
		free(List2);
	}
	return flag;
}

