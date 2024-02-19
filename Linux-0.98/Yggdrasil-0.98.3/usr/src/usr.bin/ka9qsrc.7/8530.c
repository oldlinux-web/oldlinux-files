/* Routines generic to all interfaces with 8530s */
#include "global.h"
#include "8530.h"

/* Write 8530 register */
void
write_scc(ctl,reg,val)
register int16 ctl;
int16 reg,val;
{
	char i_state;

	i_state = disable();
	/* Select register; note that point high is also written */
	if(reg != R0)
		outportb(ctl,(char)(NULLCODE|reg));
	outportb(ctl,(char)val);
	restore(i_state);
}
/* Read 8530 register */
char
read_scc(ctl,reg)
register int16 ctl;
char reg;
{
	char c,i_state;

	i_state = disable();
	/* Select register; note that point high is also written */
	if(reg != R0)
		outportb(ctl,(char)NULLCODE|reg);
	c = inportb(ctl);
	restore(i_state);
	return c;
}

