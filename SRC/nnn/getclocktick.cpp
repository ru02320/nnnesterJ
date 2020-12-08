
#include <windows.h>
#include <stdio.h>



//static DWORD sclock_hi, sclock_lo;
static _int64 starttime;
static int counter;
static FILE *f=NULL;
static int Enable=0;

//extern void InitClockTick(int );
//extern int OutClockTick();

void InitClockTick(int n){
	if(Enable)
		return;
	counter = 0;
	f = fopen("clock.txt", "a+b");
	if(f==NULL)
		return;
	{
		char *str = "start: \r\n";
		fwrite(str, strlen(str), 1, f);
	}
	_asm {
		rdtsc
		mov DWORD ptr [starttime+4], edx
		mov DWORD ptr [starttime], eax
	}
	Enable=1;
	counter=n;
}


int OutClockTick(){
	_int64 temptime;
	char str[256];
	if(f==NULL || counter==0)
		return 0;
	_asm {
		rdtsc
		mov DWORD ptr [temptime+4], edx
		mov DWORD ptr [temptime], eax
	}
	temptime -= starttime;
	sprintf(str, "Clock : %I64u \r\n", temptime);
	fwrite(str, strlen(str), 1, f);
	counter--;
	if(counter==0){
		fclose(f);
		f=NULL;
	}
	return 1;
}

