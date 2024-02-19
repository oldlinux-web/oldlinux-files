#include <vga.h>

static char line[1024];

static void testmode(int mode)
{
   int xmax, ymax, colors, i, x, y;

   vga_setmode(mode);

   vga_screenoff();

   xmax   = vga_getxdim()-1;
   ymax   = vga_getydim()-1;

   vga_drawline(   0,    0, xmax,    0);
   vga_drawline(xmax,    0, xmax, ymax);
   vga_drawline(xmax, ymax,    0, ymax);
   vga_drawline(   0, ymax,    0,    0);
   
   for(i = 0; i <= 15; i++) {
       vga_setcolor(i);
       vga_drawline(10+i*5, 10, 100+i*5, 90);
   }
   for(i = 0; i <= 15; i++) {
       vga_setcolor(i);
       vga_drawline(100+i*5, 10, 10+i*5, 90);
   }

   if(vga_getcolors() == 256) {
       for(i = 0; i < 64; i++)
           vga_setpalette(i+128, i, i, i);
       for(i = 0; i <= xmax; i++)
       	   line[i] = 128 + i%64;
   } else        
       for(i = 0; i <= xmax; i++)
       	   line[i] = i%16;

   for(i = 100; i < 190; i++)
       vga_drawscanline(i, line);

   vga_screenon(); 

   printf("Test%d\n", mode);

   vga_getch();
}


main()
{
    testmode(G320x200x16);
    testmode(G640x200x16);
    testmode(G640x350x16);
    testmode(G640x480x16);
    testmode(G320x200x256);
    testmode(G320x240x256);
    testmode(G320x400x256);
    testmode(G360x480x256);

    vga_setmode(TEXT);
}
