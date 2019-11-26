#include <stdio.h>

int
main()
{
    long tot,avl;

    getmemuse(&tot,&avl);
    printf("getmemuse : tot = %u, avl = %u\n",tot,avl);

	getmemuses(&tot,&avl);
    printf("getmemuse2 : tot = %u, avl = %u\n",tot,avl);

}

