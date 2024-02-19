#include <stdio.h>
#include <string.h>
#include <ttyent.h>

main()
{
	struct ttyent *ent;
	while (ent = getttyent ()) {
		printf("name: %s, getty: %s, type: %s, comment: %s st: %d\n",
			ent->ty_name, ent->ty_getty, ent->ty_type,
			ent->ty_comment, ent->ty_status);
	}
}
