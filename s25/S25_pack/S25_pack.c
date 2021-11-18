/*
���ڷ��S25�ļ���
made by Darkness-TX
2018.05.08
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <direct.h>
#include <Windows.h>
#include <locale.h>
#include <png.h>

typedef unsigned char  unit8;
typedef unsigned short unit16;
typedef unsigned int   unit32;

unit32 FileNum = 0;//���ļ�������ʼ����Ϊ0

struct header
{
	unit8 magic[4];//S25\0
	unit32 index_num;
}S25_header;

struct index
{
	char filename[150];
	unit32 size;
	unit32 offset;
	unit32 width;
	unit32 height;
	unit32 x;
	unit32 y;
	unit32 unk;//0
}S25_Index[10000];

void ReadIndex(FILE *src, char *fname)
{
	unit32 i = 0;
	int p = -1;
	fread(S25_header.magic, 1, 4, src);
	fread(&S25_header.index_num, 1, 4, src);
	if (strncmp(S25_header.magic, "S25\0", 4) != 0)
	{
		printf("��֧�ֵ��ļ����ͣ���ȷ���ļ�ͷΪS25\n");
		system("pause");
		exit(0);
	}
	for (i = 0; i < S25_header.index_num; i++)
	{
		sprintf(S25_Index[i].filename, "%s_%04d.png", fname, i);
		fread(&S25_Index[i].offset, 4, 1, src);
		if (S25_Index[i].offset == 0)
			S25_Index[i].size = 0;
		else
		{
			fseek(src, S25_Index[i].offset, SEEK_SET);
			fread(&S25_Index[i].width, 4, 1, src);
			fread(&S25_Index[i].height, 4, 1, src);
			fread(&S25_Index[i].x, 4, 1, src);
			fread(&S25_Index[i].y, 4, 1, src);
			fread(&S25_Index[i].unk, 4, 1, src);
			fseek(src, i * 4 + 4 + 8, SEEK_SET);
			if (S25_Index[i].height == 0 || S25_Index[i].width == 0)
			{
				printf("����Ϊ0��height:%d width:%d\n", S25_Index[i].height, S25_Index[i].width);
				system("pause");
			}
			if (S25_Index[i].unk != 0)
			{
				printf("unk�ֶβ�����0��unk:0%X\n", S25_Index[i].unk);
				system("pause");
			}
			if (p != -1)
				S25_Index[p].size = S25_Index[i].offset - S25_Index[p].offset;
			p = i;
			FileNum++;
		}
	}
	if (p != -1)
	{
		fseek(src, 0, SEEK_END);
		S25_Index[p].size = ftell(src) - S25_Index[p].offset;
	}
}

unit8* ReadPng(FILE* src)
{
	png_structp png_ptr;
	png_infop info_ptr, end_ptr;
	png_bytep *rows;
	unit32 i, width = 0, height = 0, bpp = 0, format = 0;
	unit8 buff, *data;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL)
	{
		printf("PNG��Ϣ����ʧ��!\n");
		exit(0);
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		printf("info��Ϣ����ʧ��!\n");
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		exit(0);
	}
	end_ptr = png_create_info_struct(png_ptr);
	if (end_ptr == NULL)
	{
		printf("end��Ϣ����ʧ��!\n");
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		exit(0);
	}
	png_init_io(png_ptr, src);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, (png_uint_32*)&width, (png_uint_32*)&height, &bpp, &format, NULL, NULL, NULL);
	if (bpp == 8 && format == PNG_COLOR_TYPE_RGB_ALPHA)
	{
		data = malloc(width * height * 4);
		rows = (png_bytep*)malloc(height * sizeof(char*));
		for (i = 0; i < height; i++)
			rows[i] = (png_bytep)(data + width*i * 4);
		png_read_image(png_ptr, rows);
		free(rows);
		png_read_end(png_ptr, info_ptr);
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_ptr);
		for (i = 0; i < height * width; i++)
		{
			buff = data[i * 4];
			data[i * 4] = data[i * 4 + 2];
			data[i * 4 + 2] = buff;
			buff = data[i * 4 + 3];
			data[i * 4 + 3] = data[i * 4 + 2];
			data[i * 4 + 2] = data[i * 4 + 1];
			data[i * 4 + 1] = data[i * 4];
			data[i * 4] = buff;
		}
	}
	else
	{
		printf("Region���ͼƬ����bppΪ8����32λRGBAͼƬ�������Ϲ淶��\n");
		system("pause");
		exit(0);
	}
	return data;
}

void Pack(char *fname)
{
	FILE *src, *dst;
	unit8 dirname[200], *cdata, *udata, *adata;
	unit32 i = 0, k = 0, l = 0, savepos = 0;
	unit16 line_size = 0, flag = 0x8001;
	src = fopen(fname, "rb");
	ReadIndex(src, fname);
	sprintf(dirname, "%s.new", fname);
	dst = fopen(dirname, "wb");
	fwrite(S25_header.magic, 4, 1, dst);
	fwrite(&S25_header.index_num, 4, 1, dst);
	sprintf(dirname, "%s_unpack", fname);
	_mkdir(dirname);
	_chdir(dirname);
	fclose(src);
	fseek(dst, 8 + S25_header.index_num * 4, SEEK_SET);
	for (i = 0; i < S25_header.index_num; i++)
	{
		if (S25_Index[i].size != 0)
		{
			printf("name:%s offset:0x%X size:0x%X width:%d height:%d x:%d y:%d\n", S25_Index[i].filename, S25_Index[i].offset, S25_Index[i].size, S25_Index[i].width, S25_Index[i].height, S25_Index[i].x, S25_Index[i].y);
			S25_Index[i].offset = ftell(dst);
			src = fopen(S25_Index[i].filename, "rb");
			adata = ReadPng(src);
			fwrite(&S25_Index[i].width, 4, 1, dst);
			fwrite(&S25_Index[i].height, 4, 1, dst);
			fwrite(&S25_Index[i].x, 4, 1, dst);
			fwrite(&S25_Index[i].y, 4, 1, dst);
			fwrite(&S25_Index[i].unk, 4, 1, dst);
			fseek(dst, S25_Index[i].height * 4, SEEK_CUR);
			for (k = 0; k < S25_Index[i].height; k++)
			{
				udata = malloc(S25_Index[i].width * 4);
				memcpy(udata, adata + S25_Index[i].width * 4 * k, S25_Index[i].width * 4);
				cdata = malloc(S25_Index[i].width * 6);
				if (S25_Index[i].width * 6 > 0xFFFF)
				{
					printf("ͼƬ��ȳ���ÿ������%d!\n", 0xFFFF / 6);
					system("pause");
					exit(0);
				}
				line_size = S25_Index[i].width * 6;
				for (l = 0; l < S25_Index[i].width; l++)
				{
					memcpy(cdata + l * 6, &flag, 2);
					memcpy(cdata + l * 6 + 2, udata + l * 4, 4);
				}
				free(udata);
				savepos = ftell(dst);
				fseek(dst, S25_Index[i].offset + 0x14, SEEK_SET);
				fseek(dst, k * 4, SEEK_CUR);
				fwrite(&savepos, 4, 1, dst);
				fseek(dst, savepos, SEEK_SET);
				fwrite(&line_size, 2, 1, dst);
				fwrite(cdata, S25_Index[i].width * 6, 1, dst);
				free(cdata);
			}
			free(adata);
			fclose(src);
		}
	}
	fseek(dst, 8, SEEK_SET);
	for (i = 0; i < S25_header.index_num; i++)
		fwrite(&S25_Index[i].offset, 4, 1, dst);
	fclose(dst);
}

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "chs");
	printf("project��Niflheim-BLOODY RONDO\n���ڷ��S25�ļ���\n��S25�ļ��ϵ������ϡ�\nby Darkness-TX 2018.05.08\n\n");
	Pack(argv[1]);
	printf("����ɣ����ļ���%d\n", FileNum);
	system("pause");
	return 0;
}