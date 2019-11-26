#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>

int
main()
{
#if 0
	uint64_t l = (uint64_t)450049006000100L;
#endif
	uint64_t l = 490060001L;
	uint64_t ret;

	printf("sizeof(int) 		%d \n",sizeof(int));
	printf("sizeof(long) 		%d \n",sizeof(long));
	printf("sizeof(uint32_t) 	%d \n",sizeof(uint32_t));
	printf("sizeof(uint64_t) 	%d \n",sizeof(uint64_t));

	printf("%lu \n",l);
	ret = long2bcd(l);
	printf("%lu \n",ret);
}

