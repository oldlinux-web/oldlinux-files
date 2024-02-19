/********************************************************/
/* Rootset.c - modify Linux bootdisk root device entry  */
/*           - quick Turbo C hack by Iain_Reid@ed.ac.uk */
/********************************************************/


#include <stdio.h
#include <conio.h>
#include <process.h>
#include <dos.h>

void main(int ac, char **av)
{
        char buf[512];


        if (ac != 3) {
                fprintf (stderr, "%s: update Linux bootimage root device details.\n",
                        av[0]);
                fprintf (stderr, "Usage: %s <Major> <Minor>\n", av[0]);
                exit (-1);
        }

        /**************************************************************/
        /* Don't remove this keypress bit 'cos it gives you a chance  */
        /* to run this program from your dos boot floppy, remove that */
        /* disk, put your linux boot disk into the same drive, patch  */
        /* in the new root device details and save them. No HD or     */
        /* rawrite required! (v. handy if DOS is dead)                            */
        /**************************************************************/

        printf ("Insert Linux boot disk into drive A and press any key\n");
        getch();

        if (absread (0, 1, 0, &buf) != 0) {
                 perror ("Disk reading problem");
                 exit (-1);
        }

        printf ("Current rootdevice: Major %d Minor: %d\n", buf[509], buf[508]);

        buf[508] = atoi (av[2]);
        buf[509] = atoi (av[1]);

        if (abswrite (0, 1, 0, &buf) != 0)
        {
                 perror ("Disk writing problem");
                 exit (-1);
        }
        printf ("New rootdevice: Major %d Minor: %d\n", buf[509], buf[508]);

        exit(0);
}
