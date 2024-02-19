#include "global.h"
#include "iface.h"

struct interface *
if_lookup(name)
char *name;
{
	register struct interface *iface;

	for(iface = ifaces; iface != NULLIF; iface = iface->next)
		if(strcmp(iface->name,name) == 0)
			break;
	return iface;
}
/* Divert output packets from one interface to another. Useful for ARP
 * and digipeat frames coming in from receive-only interfaces
 */
doforward(argc,argv)
int argc;
char *argv[];
{
	struct interface *iface,*iface1;
	

	if(argc < 2){
		for(iface = ifaces; iface != NULLIF; iface = iface->next){
			if(iface->forw != NULLIF){
				printf("%s -> %s\n",iface->name,iface->forw->name);
			}
		}
		return 0;
	}
	if((iface = if_lookup(argv[1])) == NULLIF){
		printf("Interface %s unknown\n",argv[1]);
		return 1;
	}
	if(argc < 3){
		if(iface->forw == NULLIF)
			printf("%s not forwarded\n",iface->name);
		else
			printf("%s -> %s\n",iface->name,iface->forw->name);
		return 0;
	}
	if((iface1 = if_lookup(argv[2])) == NULLIF){
		printf("Interface %s unknown\n",argv[2]);
		return 1;
	}
	if(iface1 == iface){
		/* Forward to self means "turn forwarding off" */
		iface->forw = NULLIF;
	} else {
		if(iface1->output != iface->output)
			printf("Warning: Interfaces of different type\n");
		iface->forw = iface1;
	}
	return 0;
}
