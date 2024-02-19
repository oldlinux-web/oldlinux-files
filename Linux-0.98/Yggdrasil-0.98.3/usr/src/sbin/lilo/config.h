/* config.h  -  Configurable parameters */

/* Written 1992 by Werner Almesberger */


#ifndef _CONFIG_H_
#define _CONFIG_H_

#define TMP_DEV     "/tmp/dev.%d" /* temporary devices are created here */
#define MAX_TMP_DEV 50 /* highest temp. device number */

#define LILO_DIR    "/etc/lilo" /* base directory for LILO files */
#define DFL_DISKTAB LILO_DIR "/disktab" /* LILO's disk parameter table */
#define MAP_FILE    LILO_DIR "/map" /* default map file */
#define MAP_TMP_APP "~" /* temporary file appendix */
#define DFL_CHAIN   LILO_DIR "/chain.b" /* default chain loader */

#define MAX_LINE    1024 /* maximum disk parameter table line length */

#endif
