/* software product name: foobarz-chhostid.c
 * suggested binary name: chhostid
 * license              : BSD
 * license text:
Copyright (c) 2012, foobarz
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define FOOBARZ_CHHOSTID_VERSION "1.0.5"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <endian.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sysexits.h>
/* note: if include stdlib then sethostid fails even with _BSD_SOURCE defined */

long swapbytes(long abcd) {
  long badc;
  unsigned char addr_a;
  unsigned char addr_b;
  unsigned char addr_c;
  unsigned char addr_d;
  addr_a = (unsigned char) (abcd >> 24);
  addr_b = (unsigned char) (abcd >> 16);
  addr_c = (unsigned char) (abcd >>  8);
  addr_d = (unsigned char) (abcd);
  return badc = ( (addr_b << 24) | (addr_a << 16) | (addr_d << 8) | addr_c );
}

int main(int argc, char* argv[])
{
 long newhostid;
 struct in_addr addr; 
 char* invalidchar;

 /*********** check command line */
 if(!( ((argc==3) && ( (strcmp(argv[1],"-h")==0) ||
                       (strcmp(argv[1],"-i")==0) ||
                       (strcmp(argv[1],"-l")==0) )) ||
       ((argc==2) && ( (strcmp(argv[1],"-s")==0) ))
     )
   ) {
                 /*01234567890123456789012345678901234567890123456789012345678901234567890123456789*/
  fprintf(stderr, "foobarz-chhostid, version %s. License: BSD.\n", FOOBARZ_CHHOSTID_VERSION);
  fprintf(stderr, "foobarz-chhostid, (c) 2012, foobarz; all rights reserved.\n");
  fprintf(stderr, "descr: %s change system hostid (in /etc/hostid).\n", argv[0]);
  fprintf(stderr, "usage: %s -h <new hostid>\n"
                  "        Change system hostid to <new hostid>;\n"
                  "        <new hostid> must be 32bit 8-digit hex number xxxxxxxx, each x in 0-f|F.\n"
                  "        The ip corresponding to <new hostid> is also listed for reference only.\n", argv[0]);
  fprintf(stderr, "usage: %s -i <dotted ip address>\n"
                  "        Change system hostid to hostid corresponding to <dotted ip address>.\n", argv[0]);
  fprintf(stderr, "usage: %s -l <dotted ip address>\n"
                  "        List hostid corresponding to <dotted ip address>;\n"
                  "        no change to system hostid.\n", argv[0]);
  fprintf(stderr, "usage: %s -s\n"
                  "        List current system hostid and corresponding ip address;\n"
                  "        no change to system hostid.\n", argv[0]);
  return EX_USAGE;
 }

 /*********** -h <hostid> */
 if( strcmp(argv[1],"-h") == 0 ) {
  if(strlen(argv[2]) != 8) { fprintf(stderr, "new hostid must be 8-digit hex number xxxxxxxx, each x in 0-f|F\n"); return EX_USAGE; }

  newhostid = strtol(argv[2],&invalidchar,16);
  if( newhostid==LONG_MIN ) { perror("strtol (underflow)"); return EX_USAGE; }
  if( newhostid==LONG_MAX ) { perror("strtol (overflow)"); return EX_USAGE; }
  if(*invalidchar != '\0') { fprintf(stderr, "invalid hex number %c; must be in 0-f|F\n", *invalidchar); return EX_USAGE; }

  if( sethostid(newhostid) != 0 ) {
   perror("sethostid");
   if( errno == EPERM ) return EX_NOPERM;
   return EX_UNAVAILABLE;
  }
  printf("System hostid changed.\n");

  /* hostid is a byte-swapped ip like b.a.d.c */
  /* get ip by swapping to correct order a.b.c.d and save into in_addr struct */
  addr.s_addr = htonl(swapbytes(newhostid));
 }
 
 /*********** -i or -l, then find hostid for <dotted ip address> */
 if( (strcmp(argv[1],"-i") == 0) || (strcmp(argv[1],"-l") == 0) ) { 
  if( inet_aton(argv[2], &addr) == 0 ) { fprintf(stderr, "inet_aton: invalid dotted ip address: %s\n", argv[2]); return EX_USAGE; }

  /* hostid is a byte-swapped ip like b.a.d.c */
  /* get hostid by swapping bytes of ip a.b.c.d */
  newhostid = swapbytes(ntohl(addr.s_addr));
 }

 /*********** -i <dotted ip address> */
 if( strcmp(argv[1],"-i") == 0 ) {
  if( sethostid(newhostid) != 0 ) {
   perror("sethostid");
   if( errno == EPERM ) return EX_NOPERM;
   return EX_UNAVAILABLE;
  }
  printf("System hostid changed.\n");
 }

 /*********** -l <dotted ip address> */
 if(strcmp(argv[1],"-l") == 0 ) {
   printf("Listing hostid for ip. (no changes)\n");
 }

 /*********** -s */
 if(strcmp(argv[1],"-s") == 0 ) {   
   /* see if /etc/hostid exists and print info */
   if( access("/etc/hostid", F_OK) != 0 ) {
     printf("Notice: /etc/hostid does not exist.\n");
     perror("Reason");
     printf("System hostid derived from ip of hostname found in hosts file or dns.\n");
   } else {
     printf("Notice: /etc/hostid exists.\n"
            "System hostid is obtained from hostid file.\n");
   }
   
  errno=0;
  newhostid = gethostid();
  if(errno!=0) { perror("gethostid"); return EX_UNAVAILABLE; }

  /* hostid is a byte-swapped ip like b.a.d.c */
  /* get ip by swapping to correct order a.b.c.d and save into in_addr struct */
  addr.s_addr = htonl(swapbytes(newhostid));

  printf("Listing current system hostid. (no changes)\n");
 }

 /*********** print info and return */
 printf("ip dot: %s\n", inet_ntoa(addr) ); 
 printf("ip hex: %.8x\n", ntohl(addr.s_addr) );  
 printf("hostid: %.8x\n", newhostid );
 return EX_OK; 
}
