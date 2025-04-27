/*--------------------------------------------------------------------*/
/* XEX2CAR	                                                          */
/* by GienekP                                                         */
/* (c) 2025                                                           */
/*--------------------------------------------------------------------*/
#include <stdio.h>
/*--------------------------------------------------------------------*/
#include "xex2car.h"
/*--------------------------------------------------------------------*/
typedef unsigned char U8;
/*--------------------------------------------------------------------*/
U8 filexex[1024*1024-8192];
U8 filecar[1024*1024];
/*--------------------------------------------------------------------*/
unsigned int word(const U8 *pos)
{
	unsigned int ret;
	ret=pos[1];
	ret<<=8;
	ret|=pos[0];
	return ret;
}
/*--------------------------------------------------------------------*/
unsigned int kill2b(U8 *data, unsigned int pos, unsigned int size)
{
	unsigned int i,ret=(size-2);
	for (i=pos; i<ret; i++) {data[i]=data[i+2];};
	return ret;
}
/*--------------------------------------------------------------------*/
unsigned int optimize(U8 *data, unsigned int size)
{
	unsigned int ret=size,start,stop,i=0,run=0,first=0xFFFF;
	while (i<ret)
	{
		if ((data[i]==0xFF) & (data[i+1]==0xFF)) {ret=kill2b(data,i,ret);};
		start=word(&data[i]);
		stop=word(&data[i+2]);
		if (first==0xFFFF) {first=start;};
		if (start>stop) {ret=i;}
		else
			{
			if (((start<=0x02E0) && (stop>=0x02E1)) || ((start<=0x02E2) && (stop>=0x02E3))) {run=1;};
			printf("Block $%04X - $%04X (%i bytes)\n",start,stop,stop-start+1);
			i+=5;
			i+=(stop-start);
			};
	};
	if (run==0)
	{
		data[ret]=0xE0;
		data[ret+1]=0x02;
		data[ret+2]=0xE1;
		data[ret+3]=0x02;
		data[ret+4]=(first&0xFF);
		data[ret+5]=((first>>8)&0xFF);
		ret+=6;
		printf("Add RUN($%04X)\n",first);
	};
	data[ret]=0xFF;
	data[ret+1]=0xFF;
	data[ret+2]=0xFE;
	data[ret+3]=0xFF;
	ret+=4;
	return ret;
}
/*--------------------------------------------------------------------*/
unsigned int pack(const U8 *loader, unsigned int loadersize, const U8 *xex, unsigned int xexsize, U8 *car, unsigned int carmaxsize)
{
	const unsigned int pi=(0xBE00-0xA000);
	unsigned int i,ret;
	for (i=0; i<loadersize; i++)
	{
		car[i]=loader[i];
		car[carmaxsize-loadersize+i]=loader[i];
	};
	if ((loadersize+xexsize)>carmaxsize) {xexsize=(carmaxsize-loadersize);};
	if (xexsize<pi)
	{
		for (i=0; i<xexsize; i++) {car[i]=xex[i];};
	}
	else
	{
		for (i=0; i<pi; i++) {car[i]=xex[i];};
		for (i=0; i<(xexsize-pi); i++) {car[i+loadersize]=xex[i+pi];};
	};
	ret=loadersize+xexsize-pi;
	return ret;		
}
/*--------------------------------------------------------------------*/
void clear(U8 *data, unsigned int size, U8 val)
{
	unsigned int i;
	for (i=0; i<size; i++) {data[i]=val;};
}
/*--------------------------------------------------------------------*/
unsigned int loadXEX(const char *filename, U8 *data, unsigned int size)
{
	unsigned int ret=0;
	FILE *pf;
	pf=fopen(filename,"rb");
	if (pf)
	{
		ret=fread(data,sizeof(U8),size,pf);
		fclose(pf);
	};
	return ret;
}
/*--------------------------------------------------------------------*/
unsigned int saveCARtype(const char *filename, U8 *cardata, unsigned int carsize, U8 cartype)
{
	U8 header[16]={0x43, 0x41, 0x52, 0x54, 0x00, 0x00, 0x00, cartype,
		           0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};
	unsigned int i,sum=0,ret=0;
	FILE *pf;
	for (i=0; i<carsize; i++) {sum+=cardata[i];};
	header[8]=((sum>>24)&0xFF);
	header[9]=((sum>>16)&0xFF);
	header[10]=((sum>>8)&0xFF);
	header[11]=(sum&0xFF);
	pf=fopen(filename,"wb");
	if (pf)
	{
		i=fwrite(header,sizeof(U8),16,pf);
		if (i==16)
		{
			i=fwrite(cardata,sizeof(U8),carsize,pf);
			if (i==carsize) {ret=(carsize+16);};			
		};
		fclose(pf);
	};
	return ret;
}
/*--------------------------------------------------------------------*/
unsigned int saveCAR(const char *filename, U8 *cardata, unsigned int size)
{
	unsigned int ret=0;
	if (size>(512*1024)) {printf("Type J(atari)Cart1024(kB)\n");ret=saveCARtype(filename,cardata,1024*1024,111);} else
	if (size>(256*1024)) {printf("Type J(atari)Cart512(kB)\n");ret=saveCARtype(filename,cardata,512*1024,110);} else
	if (size>(128*1024)) {printf("Type J(atari)Cart256(kB)\n");ret=saveCARtype(filename,cardata,256*1024,109);} else
	if (size>(64*1024)) {printf("Type J(atari)Cart128(kB)\n");ret=saveCARtype(filename,cardata,128*1024,108);} else
	if (size>(32*1024)) {printf("Type J(atari)Cart64(kB)\n");ret=saveCARtype(filename,cardata,64*1024,107);} else
	if (size>(16*1024)) {printf("Type J(atari)Cart32(kB)\n");ret=saveCARtype(filename,cardata,32*1024,106);} else
	if (size>(8*1024)) {printf("Type J(atari)Cart16(kB)\n");ret=saveCARtype(filename,cardata,16*1024,105);} else
	{printf("Type J(atari)Cart8(kB)\n");ret=saveCARtype(filename,cardata,8*1024,104);}
	return ret;
}
/*--------------------------------------------------------------------*/
void xex2car(const char *filexex, const char *filecar, U8 *buf, unsigned int maxbuf, U8 *data, unsigned int maxsize)
{
	unsigned int size,i;
	clear(buf,maxbuf,0x00);
	clear(data,maxsize,0xFF);
	size=loadXEX(filexex,buf,maxbuf);
	if (size>0)
	{
		printf("Load \"%s\" size %i bytes.\n",filexex,size);
		size=optimize(buf,size);
		size=(((size/128)+1)*128);
		printf("Optimize to %i bytes.\n",size);
		size=pack(xex2car_bin,sizeof(xex2car_bin),buf,size,data,maxsize);
		printf("Pack to %i bytes.\n",size);
		i=saveCAR(filecar,data,size);
		printf("Save \"%s\" size %i bytes.\n",filecar,i);
	}
	else
	{
		printf("Can't read \"%s\"\n",filexex);
	};
}
/*--------------------------------------------------------------------*/
int main( int argc, char* argv[] )
{		
	printf("XEX2CAR - ver: %s\n",__DATE__);
	switch (argc)
	{
		case 3:
		{
			xex2car(argv[1],argv[2],filexex,sizeof(filexex),filecar,sizeof(filecar));
		} break;
		default:
		{
			printf("(c) GienekP\n");
			printf("use:\nxex2car file.xex file.car\n");
		} break;
	};
	return 0;
}
/*--------------------------------------------------------------------*/
