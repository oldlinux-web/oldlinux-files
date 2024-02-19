/* boot.h */

/* Redefine root and root image devices as variables.
 * This keeps the diffs small but may cause future confusion.
 */
#define BOOT_DEV   (boot_parameters.bp_ramimagedev)
#define ROOT_DEV   (boot_parameters.bp_rootdev)

/* Device numbers of RAM, floppy and hard disk devices.
 * h/com.h defines RAM_DEV but only as the minor number.
 */
#define DEV_FD0   0x200
#define DEV_HD0   0x300
#define DEV_RAM   0x100

/* Default device numbers for root and root image.
 * Root image is only used when root is /dev/ram.
 */
#define DRAMIMAGEDEV (DEV_FD0 + 0)
#define DROOTDEV  (DEV_RAM + 0)

/* Default RAM disk size.
 * Not used if root is /dev/ram when size is from root image.
 */
#define DRAMSIZE   0

/* Default scan code to fake a PC keyboard. */
#define DSCANCODE  13

/* Default processor type for no restriction (88 would force 386 to 88). */
#define DPROCESSOR 0xFFFF

/* Structure to hold boot parameters. */
struct bparam_s
{
  dev_t bp_rootdev;
  dev_t bp_ramimagedev;
  unsigned short bp_ramsize;
  unsigned short bp_scancode;		/* still put into BX for kernel */
  unsigned short bp_processor;
};

extern struct bparam_s boot_parameters;
