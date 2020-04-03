#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

int infile;
int halt=1;

int32_t rev4(int32_t val)
{
	return ((val&0xff)<<24) | ((val&0xff00)<<8)
	 | ((val&0xff0000)>>8) | ((val&0xff000000)>>24);
}

int cchit(void) {return 0;}

int32_t read4(void)
{
	int32_t val;

	halt=read(infile,&val,4);
	return rev4(val);
}
void symbol(int32_t len) {
	char temp[80];
	char *p1;

	int res=read(infile,temp,(int)len<<2);res=res;
	len<<=2;
	p1=temp;
	while(len-- && *p1) putchar(*p1++);
}


void dataout(int32_t len) {
	int x=0;
	int32_t v;

	len&=0xffffffL;
	while(len--)
	{
		if(cchit()) exit(0);
		v=read4();
		printf(" %08x",v);
		x++;
		if(x==8) {x=0;putchar('\n');}
	}
	if(x) putchar('\n');
}

void relocout(char *str) {
	int32_t v1,v2;
	printf("HUNK_%s\n",str);
	while((v1=read4()))
	{
		v2=read4();
		printf("hunk=%d\n",v2);
		dataout(v1);
	}
}

int main(int argc, char **argv) {
	int32_t v1,v2;
	int t;

	if(argc<2) {printf("Use: od <objectfile>\n");return 0;}
	infile=open(argv[1],O_RDONLY);
	if(infile<0) {printf("Cannot open input file %s\n",argv[1]);return 1;}
	for(;;)
	{
		v1=read4();
		if(!halt) return 0;
		if(cchit()) exit(0);
printf("[%08x] ",v1);
		switch((int)v1)
		{
		case 0x3f3:
			printf("HUNK_HEADER\n ");
			while((v1=read4()))
			{
				putchar('"');
				symbol(v1);
				printf("\",");
			}
			v1=read4();printf(" [%d]",v1);
			v1=read4();printf(" [%d]",v1);
			v2=read4();printf(" [%d]",v1);
			putchar('\n');
			dataout(v2-v1+1);
			break;
		case	0x3eb:
			printf("HUNK_BSS [%08x]\n",read4());
			break;
		case 0x3f1:
			v1=read4();
			printf("HUNK_DEBUG [%08x]\n",v1);
			dataout(v1);
			break;
		case 0x3f2:
			printf("HUNK_END\n");
			break;
		case 0x3e8:
			printf("HUNK_NAME \"");
			symbol(read4());
			printf("\"\n");
			break;
		case 0x3e9:
			v1=read4();
			printf("HUNK_CODE [%08x]\n",v1);
			dataout(v1);
			break;
		case 0x3ea:
			v1=read4();
			printf("HUNK_DATA [%08x]\n",v1);
			dataout(v1);
			break;
		case 0x3cd:
			relocout("16RELOC");
			break;
		case 0x3cc:
			relocout("32RELOC");
			break;
		case 0x3ed:
			relocout("RELOC16");
			break;
		case 0x3ec:
			relocout("RELOC32");
			break;
		case 0x3f0:
			printf("HUNK_SYMBOL\n");
			while((v1=read4()))
			{
				if(cchit()) exit(0);
				putchar(' ');
				symbol(v1);
				printf("=%x\n",read4());
			}
			break;
		case 0x3e7:
			printf("HUNK_UNIT \"");
			if((v1=read4())) symbol(v1);
			printf("\"\n");
			break;
		case 0x3ef:
			printf("HUNK_EXT\n");
			while((v1=read4()))
			{
				if(cchit()) exit(0);
				t=(unsigned long)v1>>24L;
				v1&=0xffffffL;
				printf(" [EXT_");
				switch(t)
				{
				case 0x00: printf("SYMB] ");break;
				case 0x01: printf("DEF] ");break;
				case 0x02: printf("ABS] ");break;
				case 0x81: printf("REF32] ");break;
				case 0x82: printf("COMMON] ");break;
				case 0x83: printf("REF16] ");break;
				case 0x84: printf("REF8] ");break;
				case 0x86: printf("CODE86 ");break;
				case 0x8b: printf("REL16] ");break;
				case 0x91: printf("32REF] ");break;
				case 0x93: printf("16REF] ");break;
				case 0x99: printf("32REL] ");break;
				case 0x9b: printf("16REL] ");break;
				default: printf("%02x?] ",t);break;
				}
				symbol(v1);
				if(t==1 || t==2 || t==3)
					printf("=%08x\n",read4());
				else
				if(t&0x80)
					{putchar('\n');dataout(read4());}
				else
					{v1=read4();printf(" [%08x]\n", v1);
						dataout(read4());}
			}
			break;
		default:
			printf("Unknown code %08x\n",v1);
			return 1;
		}
	}
}
