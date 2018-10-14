#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>

//this is arbitray and unsafe until I reserve this memory somehow
#define MEMLOC 0xa0000004 
//#define SIZE 1000000
#define SIZE 1050

#define ARM_2_PRU 0x01
#define PRU_2_ARM 0x02
#define STATUS 0x01 // first memloc is that status/communication register
/*
   BEAGLEBONE MEMORY STRUCTURE
   From What I can tell, the beaglebone's memory structure is 32 bit words located at 
   addresses 0x04 bits apart
 */

int main()
{
	//open /dev/mem which gives us access to physical memory
	int fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd == -1) 
	{
		printf("failed to open\n");
		return 0;
	}

	//mmap our physical mem location to a virtual address
	//should this be volatile and unsigned?
	int *ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, MEMLOC);
	if (ptr == MAP_FAILED)
	{
		close(fd);
		perror("Error mmapping the file");
		exit(EXIT_FAILURE);
	}

	//zero status flags
	*ptr &= !ARM_2_PRU;
	*ptr &= !PRU_2_ARM;

	//set starting location
	int *addr = ptr + STATUS;

	//zero the space
	for(addr ; addr < (ptr + STATUS + SIZE) ; addr++)
		*addr = 0x00;

	//send start flag to PRU
	*ptr |= ARM_2_PRU;

	//wait for response
	while((*ptr & PRU_2_ARM) < 1); //TODO: need a timeout here 

	printf("PRU Response Received\n");

	//reset starting location
	addr = ptr + STATUS;

	//read back data 
	int k = 0;
	for(addr ; addr < (ptr + STATUS + SIZE) ; addr++)
	{
		printf("%x(%i) = %x\n", addr, k, *addr);
		k++;
	}

	printf("Deallocating Memory\n");

	//unmap memory
	munmap(ptr, SIZE);
}


//Sorry, Oliver is a old code horder, deal with it...
/*
   for(int i = 0 ; i < 5 ; ++i)
   {
   if((*ptr & PRU_2_ARM) > 0)
   {
   printf("recieved response from PRU\n");
 *ptr &= !PRU_2_ARM; //clear the reciece flag
// *ptr |= ARM_2_PRU; //set the send flag again
}else{
printf("waiting for respose\n");
 *ptr |= ARM_2_PRU;
 }
 sleep(1);
 }

 */


/*
   while(1)
   {
 *(ptr + STATUS) = 1;
 printf("ptr = %x\n", *(ptr+1));
 sleep(2);
 *(ptr + STATUS) = 0;
 printf("ptr = %x\n", *(ptr+1));
 sleep(2);
 }
 printf("ptr addr: %x\n", ptr+1);
 ptr[0] = 0x0;
 while(1)
 {
 printf("orig ptr = %x\n", *ptr);

 ptr[0] = 0x41;
 printf("new ptr = %x\n", *ptr);
 ptr[0] = 0x0;

 sleep(2);

 }*/
