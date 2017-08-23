//file:pngtest.c  
//changed from the libpng,对照libpng中源码阅读  
//myers
#include <stdio.h>  
#include <png.h>  
#include <stdio.h>  
#include "zlib.h"
#include <stdlib.h>  
#include <string.h>  

#define required_bit_depth 4

#pragma pack(2) 

struct bmp_fileheader
{
    unsigned short    bfType;        //若不对齐，这个会占4Byte
    unsigned int    bfSize;
    unsigned short    bfReverved1;
    unsigned short    bfReverved2;
    unsigned int    bfOffBits;
};

struct bmp_infoheader
{
    unsigned int    biSize;
    unsigned int    biWidth;
    unsigned int    biHeight;
    unsigned short    biPlanes;
    unsigned short    biBitCount;
    unsigned int	biCompression;
    unsigned int    biSizeImage;
    unsigned int    biXPelsPerMeter;
    unsigned int    biYpelsPerMeter;
    unsigned int    biClrUsed;
    unsigned int    biClrImportant;
};


png_FILE_p fpin;  
FILE *fpout;   
//读：  
png_structp read_ptr;  
png_infop read_info_ptr, end_info_ptr;  
//写  
png_structp write_ptr;  
png_infop write_info_ptr,write_end_info_ptr;  
//  
png_bytep row_buf;  
png_uint_32 y;  
int num_pass, pass;  
png_uint_32 width, height;//宽度，高度  
int bit_depth, color_type;//位深，颜色类型  
int interlace_type, compression_type, filter_type;//扫描方式，压缩方式，滤波方式 

int isRGB;

png_textp text_ptr;  
int num_text;  
png_timep mod_time;
png_colorp palette;  
int num_palette;

enum ERROR_CODE{PASS,ERROR_PNGREAD,ERROR_BMPWRITE,ERROR_COLORTYPE};

int read_png_info()
{
	//读文件有high level(高层）和low level两种，我们选择从底层具体信息中读取。  
    //这里我们读取所有可选。  
    png_read_info(read_ptr, read_info_ptr);      
    //（1）IHDR  
      //读取图像宽度(width)，高度(height)，位深(bit_depth)，颜色类型(color_type)，压缩方法(compression_type)  
    //滤波器方法(filter_type),隔行扫描方式(interlace_type)  
    if (png_get_IHDR(read_ptr, read_info_ptr, &width, &height, &bit_depth,  
      &color_type, &interlace_type, &compression_type, &filter_type))  
    {  
    //我们采用默认扫描方式  
     png_set_IHDR(write_ptr, write_info_ptr, width, height, bit_depth,  
        color_type, PNG_INTERLACE_NONE, compression_type, filter_type);  
    }  
    //（2）cHRM  
    //读取白色度信息  白/红/绿/蓝 点的x,y坐标，这里采用整形，不采用浮点数  
    png_fixed_point white_x, white_y, red_x, red_y, green_x, green_y, blue_x,blue_y;  
  
    if (png_get_cHRM_fixed(read_ptr, read_info_ptr, &white_x, &white_y,  
     &red_x, &red_y, &green_x, &green_y, &blue_x, &blue_y))  
    {  
     png_set_cHRM_fixed(write_ptr, write_info_ptr, white_x, white_y, red_x,  
        red_y, green_x, green_y, blue_x, blue_y);  
    }  
        //（3）gAMA  
      png_fixed_point gamma;  
  
      if (png_get_gAMA_fixed(read_ptr, read_info_ptr, &gamma))  
         png_set_gAMA_fixed(write_ptr, write_info_ptr, gamma);  
        //（4）iCCP  
    png_charp name;  
    png_bytep profile;  
    png_uint_32 proflen;  
  
    if (png_get_iCCP(read_ptr, read_info_ptr, &name, &compression_type,  
              &profile, &proflen))  
    {  
     png_set_iCCP(write_ptr, write_info_ptr, name, compression_type,  
              profile, proflen);  
    }  
    //(5)sRGB  
      int intent;  
      if (png_get_sRGB(read_ptr, read_info_ptr, &intent))  
         png_set_sRGB(write_ptr, write_info_ptr, intent);  
    //(7)PLTE  
      //png_colorp palette;  
      //int num_palette;  
  
      if (png_get_PLTE(read_ptr, read_info_ptr, &palette, &num_palette))  
         png_set_PLTE(write_ptr, write_info_ptr, palette, num_palette);  
    //(8)bKGD  
      png_color_16p background;  
  
      if (png_get_bKGD(read_ptr, read_info_ptr, &background))  
      {  
         png_set_bKGD(write_ptr, write_info_ptr, background);  
      }  
    //(9)hist  
   
      png_uint_16p hist;  
  
      if (png_get_hIST(read_ptr, read_info_ptr, &hist))  
         png_set_hIST(write_ptr, write_info_ptr, hist);  
       //(10)oFFs  
      png_int_32 offset_x, offset_y;  
      int unit_type;  
  
      if (png_get_oFFs(read_ptr, read_info_ptr, &offset_x, &offset_y,  
          &unit_type))  
      {  
         png_set_oFFs(write_ptr, write_info_ptr, offset_x, offset_y, unit_type);  
      }  
    //(11)pCAL  
      png_charp purpose, units;  
      png_charpp params;  
      png_int_32 X0, X1;  
      int type, nparams;  
  
      if (png_get_pCAL(read_ptr, read_info_ptr, &purpose, &X0, &X1, &type,  
         &nparams, &units, &params))  
      {  
         png_set_pCAL(write_ptr, write_info_ptr, purpose, X0, X1, type,  
            nparams, units, params);  
      }  
       //(12)pHYs  
     
      png_uint_32 res_x, res_y;  
  
      if (png_get_pHYs(read_ptr, read_info_ptr, &res_x, &res_y, &unit_type))  
         png_set_pHYs(write_ptr, write_info_ptr, res_x, res_y, unit_type);  
   //(13)sBIT  
      png_color_8p sig_bit;  
  
      if (png_get_sBIT(read_ptr, read_info_ptr, &sig_bit))  
         png_set_sBIT(write_ptr, write_info_ptr, sig_bit);  
    //（14）sCAL  
      int unit;  
      png_charp scal_width, scal_height;

	/*  
      if (png_get_sCAL_s(read_ptr, read_info_ptr, &unit, &scal_width,  
          &scal_height))  
      {  
         png_set_sCAL_s(write_ptr, write_info_ptr, unit, scal_width,  
             scal_height);  
      }
	*///Dukelop:sth wrong with png_get/set_sCAL_s
        //(15)iTXt  
      //png_textp text_ptr;  
      //int num_text;  
  
      if (png_get_text(read_ptr, read_info_ptr, &text_ptr, &num_text) > 0)  
      {  
         png_set_text(write_ptr, write_info_ptr, text_ptr, num_text);  
      }  
    //(16)tIME,这里我们不支持RFC1123  
      //png_timep mod_time;  
  
      if (png_get_tIME(read_ptr, read_info_ptr, &mod_time))  
      {  
         png_set_tIME(write_ptr, write_info_ptr, mod_time);  
      }  
    //(17)tRNS  
      png_bytep trans_alpha;  
      int num_trans;  
      png_color_16p trans_color;  
  
      if (png_get_tRNS(read_ptr, read_info_ptr, &trans_alpha, &num_trans,  
         &trans_color))  
      {  
         int sample_max = (1 << bit_depth);  
         /* libpng doesn't reject a tRNS chunk with out-of-range samples */  
         if (!((color_type == PNG_COLOR_TYPE_GRAY &&  
             (int)trans_color->gray > sample_max) ||  
             (color_type == PNG_COLOR_TYPE_RGB &&  
             ((int)trans_color->red > sample_max ||  
             (int)trans_color->green > sample_max ||  
             (int)trans_color->blue > sample_max))))  
            png_set_tRNS(write_ptr, write_info_ptr, trans_alpha, num_trans,  
               trans_color);  
      }  
      
////////////////////
	printf("read file over\n");
///////////////////

    //写进新的png文件中
    //png_write_info(write_ptr, write_info_ptr);
	return PASS; 
}

int write_bmp_header()
{
	int status=PASS;

    struct bmp_fileheader bfh;
    struct bmp_infoheader bih;

    unsigned short depth;
    unsigned long headersize;
    unsigned long filesize;

    depth=required_bit_depth;

	isRGB=1;

	printf("writing bmp header...\n");

    if (color_type==0||color_type==3||color_type==4)
    {
        isRGB=0;
    }

    if (color_type==2||color_type==6)
    {
        isRGB=1;
    }
	headersize=14+40;
    filesize=headersize+width*height*depth;

    memset(&bfh,0,sizeof(struct bmp_fileheader));
    memset(&bih,0,sizeof(struct bmp_infoheader));
    
    //写入比较关键的几个bmp头参数
    bfh.bfType=0x4D42;
    bfh.bfSize=filesize;
    bfh.bfOffBits=headersize;

    bih.biSize=40;
    bih.biWidth=width;
    bih.biHeight=-height;
    bih.biPlanes=1;
    bih.biBitCount=(unsigned short)depth*8;
    bih.biSizeImage=width*height*depth;

    printf("filesize:%ld\n",filesize);
    printf("headersize:%ld\n",headersize);
    printf("width:%d\n",width);
    printf("height:%d\n",height);
    printf("depth:%d\n",depth);

    if(fwrite(&bfh,sizeof(struct bmp_fileheader),1,fpout)!=1)	return ERROR_BMPWRITE;
    if(fwrite(&bih,sizeof(struct bmp_infoheader),1,fpout)!=1)	return ERROR_BMPWRITE;
/*
    if (color_type==0||color_type==3||color_type==4)        //灰度图像要添加调色板
    {
        unsigned char *platte=malloc(sizeof(unsigned char)*256*4);
        //platte=new unsigned char[256*4];
        unsigned char j=0;
        for (int i=0;i<1024;i+=4)
        {
            platte[i]=j;
            platte[i+1]=j;
            platte[i+2]=j;
            platte[i+3]=0;
            j++;
        }
        fwrite(platte,sizeof(unsigned char)*1024,1,fpout);
        free(platte);
    }
*/
	return status;
}

int write_bmp_data()
{
    png_bytep row_pointers[height];
    int row; 
    for (row = 0; row <height; row++){
            row_pointers[row] = NULL;
    }
    for (row = 0; row <height; row++){
            row_pointers[row] = (png_bytep)png_malloc(read_ptr, png_get_rowbytes(read_ptr,read_info_ptr));
    }
    png_read_image(read_ptr, row_pointers);     
    int i, j;
    int size = (width) * (height) * 4;
    printf("malloc(%d)\n", size);
	
    unsigned int *Dst = (unsigned int*)malloc(size);//因为sizeof(unsigned long)=8
	unsigned int *pDst=NULL;

    printf("sizeof(unsigned long) = %d\n", (int)sizeof(unsigned long));

	unsigned char* pSrc;
	unsigned long pixelR,pixelG,pixelB,pixelA;
	int fillwidth;
	int status=PASS;

	pDst=Dst;
	
	switch(color_type)
	{
		case 2:
		{
			printf("RGB_24\n");
    	    for(j=0; j<height; j++)
			{
    	        pSrc = row_pointers[j];
    	        for(i=0; i<width; i++)
				{
    	            pixelR = *pSrc++;
    	            pixelG = *pSrc++;
    	            pixelB = *pSrc++;
    	            pixelA = 0xFF;
    	            pDst[i] = (pixelA<< 24) | (pixelR << 16) | (pixelG << 8) | pixelB;
    	            //printf("%06x ", pDst[i]);
					
					//fwrite(&pDst[i],sizeof(unsigned long),1,fpout);
    	        }
            	//printf("\n");

				//fillwidth=sizeof(unsigned char)*width*4;
				//printf("fillwidth=%d\n",fillwidth);
				fwrite(pDst,sizeof(unsigned int),width,fpout);
            	pDst += width;
        	}

			break;
		}

		case 3:
		{
			printf("palette\n");
			for(j=0; j<height; j++)
			{
    	        pSrc = row_pointers[j];
    	        for(i=0; i<width; i++)
				{
    	            pixelR = (palette+*pSrc++)->red;
    	            pixelG = (palette+*pSrc++)->green;
    	            pixelB = (palette+*pSrc++)->blue;
    	            pixelA = 0xFF;

					//dukelop:seems data in png with color_type(3) been copyed 3 times
    	            pDst[i++] = (pixelA<< 24) | (pixelR << 16) | (pixelG << 8) | pixelB;
					pDst[i++] = (pixelA<< 24) | (pixelR << 16) | (pixelG << 8) | pixelB;
					pDst[i] = (pixelA<< 24) | (pixelR << 16) | (pixelG << 8) | pixelB;
					//
    	            //printf("%06x ", pDst[i]);
					
					//fwrite(&pDst[i],sizeof(unsigned long),1,fpout);
    	        }
            	//printf("\n");

				//fillwidth=sizeof(unsigned char)*width*4;
				//printf("fillwidth=%d\n",fillwidth);
				fwrite(pDst,sizeof(unsigned int),width,fpout);
            	pDst += width;
        	}
			
			break;
		}

		case 6:
		{
			printf("RGB_32\n");
    	    for(j=0; j<height; j++)
			{
    	        pSrc = row_pointers[j];
    	        for(i=0; i<width; i++)
				{
    	            pixelR = *pSrc++;
    	            pixelG = *pSrc++;
    	            pixelB = *pSrc++;
    	            pixelA = *pSrc++;
    	            pDst[i] = (pixelA<< 24) | (pixelR << 16) | (pixelG << 8) | pixelB;
    	            //printf("%08x ", pDst[i]);
    	        }
            	//printf("\n");
				fwrite(pDst,sizeof(unsigned int),width,fpout);
            	pDst += width;
        	}

			break;
		}
		default:{printf("color_type:%d	not defined\n",bit_depth);status=ERROR_COLORTYPE;break;}
        
    }
	
	for (row = 0; row <height; row++)
	{
            png_free(read_ptr,row_pointers[row]);
    }
	
	free(Dst);
	Dst=NULL;
	return status;
}

int png2bmp(char *inname,char *outname)  
{  
    
    //读  
    row_buf = NULL;  
    //打开读文件  
    if ((fpin = fopen(inname, "rb")) == NULL)  
    {  
        fprintf(stderr,"Could not find input file %s\n", inname);  
        return ERROR_PNGREAD;  
    }  
    //打开写文件  
    if ((fpout = fopen(outname, "wb")) == NULL)  
    {  
        printf("Could not open output file %s\n", outname);  
        fclose(fpin);  
        return ERROR_BMPWRITE;  
    }  
    //我们这里不处理未知的块unknown chunk  
    //初始化1  
    read_ptr =  
              png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);  
    write_ptr =  
          png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);  
    read_info_ptr = png_create_info_struct(read_ptr);  
       end_info_ptr = png_create_info_struct(read_ptr);  
    write_info_ptr = png_create_info_struct(write_ptr);  
       write_end_info_ptr = png_create_info_struct(write_ptr);  
    //初始化2  
    png_init_io(read_ptr, fpin);  
    png_init_io(write_ptr, fpout);  

	int status=PASS;

	if((status=read_png_info())!=PASS)	return status;
	if((status=write_bmp_header())!=PASS)	return status;
	if((status=write_bmp_data())!=PASS)	return status;
     
/*
    //读真正的图像数据  
    num_pass = 1;  
   for (pass = 0; pass < num_pass; pass++)  
   {  
      for (y = 0; y < height; y++)  
      {  
    //分配内存  
         row_buf = (png_bytep)png_malloc(read_ptr,  
            png_get_rowbytes(read_ptr, read_info_ptr));  
    png_read_rows(read_ptr, (png_bytepp)&row_buf, NULL, 1);  
    png_write_rows(write_ptr, (png_bytepp)&row_buf, 1);  
         png_free(read_ptr, row_buf);  
         row_buf = NULL;  
      }  
   }  
    //结束  
   png_read_end(read_ptr, end_info_ptr);  

////////////////////////
	printf("png read end\n");
////////////////////////

    //  
    //tTXt  
      if (png_get_text(read_ptr, end_info_ptr, &text_ptr, &num_text) > 0)  
      {  
         png_set_text(write_ptr, write_end_info_ptr, text_ptr, num_text);  
      }  
    //tIME  
      if (png_get_tIME(read_ptr, end_info_ptr, &mod_time))  
      {  
         png_set_tIME(write_ptr, write_end_info_ptr, mod_time);  
      }  
    //  
    png_write_end(write_ptr, write_end_info_ptr);  
    //回收  

////////////////////////
	printf("png write end\n");
////////////////////////
*/
    //png_free(read_ptr, row_buf);  
    //row_buf = NULL;
/*
    png_destroy_read_struct(&read_ptr, &read_info_ptr, &end_info_ptr);  
    png_destroy_info_struct(write_ptr, &write_end_info_ptr);  
    png_destroy_write_struct(&write_ptr, &write_info_ptr);
*/  
    //  
   fclose(fpin);  
   fclose(fpout);  

//////////////////
	printf("file write over\n\n");
	printf("pic_info:\n");
	printf("pic_width:%d\npic_height:%d\n",width,height);
	printf("bit_depth:%d\ncolor_type:%d\n",bit_depth,color_type);
	printf("interlace_type:%d\ncompression_type:%d\nfilter_type:%d\n",interlace_type, compression_type, filter_type);
	printf("palette num:%d\n",num_palette);
	
//////////////////  
   return status;  
}

int main()
{
	char *pn="test.png";
	char *bn="test.bmp";
	return png2bmp(pn,bn);
}

