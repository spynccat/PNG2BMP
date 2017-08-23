FUNCTION:PNG2BMP
SOURCE CODE:main.c
LIBRARY USE:libpng
RUNNING:p2b
/////////////////////////////////////////
0.api:
int png2bmp(char *pngname,char *bmpname)

input parameters:
pngname:name of input png file.eg:*pngname="test.png"
bmpname:name of output bmp file.eg:*bmpname="test.bmp"

output parameters:
return Error Code(see more in No.7)
/////////////////////////////////////////

1.function:translate png image(test.png) to bmp file(test.bmp)
2.input:test.png
3.output:test.bmp
4.function:
(if not listed,see libjpeg document for more information)
void read_png_info()			:read info from pngfile to [read_ptr,read_info_ptr](defination according to libpng document)
void write_bmp_data()			:wirte imagedata from read_ptr to buffer(pSrc),write bmpfile data to pDst according to colortype,should used after read_png_info()

5.colortype:
(according to png file format,see png format for more)
2:pSrc is in RGB_24,fill pDst(A level:0xff)
3:pSrc is in palette,fill pDst by palette
6:pSrc is in ARGB_32,fill pDst as pSrc
other colortype do not support

6.static vars:
png_FILE_p fpin;  										:libpng-pointer of input png file
FILE *fpout;   											:pointer of output bmp file
png_structp read_ptr;  									:libpng-stream,with png file data in it,used by libpng
png_infop read_info_ptr, end_info_ptr;					:libpng-stream,with png_file_info/png_file_end in it,used by libpng
png_structp write_ptr;  								:libpng-stream,for write png file data,used by libpng(useless)
png_infop write_info_ptr,write_end_info_ptr;			:libpng-stream,for write png_file_info/png_file_end,used by libpng(useless)
png_bytep row_buf;										:unused buffer;											:
int num_pass, pass;  									:unused int,for decode interlanced png data 
png_uint_32 width, height;								:image's width/height(px)
int bit_depth, color_type;								:image's bit_depth/color_type(see colortype in No.5)
int interlace_type, compression_type, filter_type;		:image's interlace_type/compression_type/filter_type,see png format for more
int isRGB;												:is png a rgb file
png_textp text_ptr;										:libpng-stream,with png's text data in it,used by libpng
int num_text;											:num of png's text data
png_timep mod_time;										:libpng-stream,with png's time data in it,used by libpng
png_colorp palette;										:libpng-colormap,used by libpng  
int num_palette;										:num of png file's palette

7.Error Code
PASS							:pass
ERROR_PNGREAD					:cannot open or read from png file
ERROR_BMPWRITE					:cannot open or write into bmp file
ERROR_COLORTYPE					:unknown colortype occurred,may be png file damage

see libpng document in http://www.libpng.org/pub/png/libpng-manual.txt
