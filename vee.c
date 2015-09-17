/*
 * vee 3.4.5
 * Copyright (C) Alexander Kozhevnikov <mentalisttraceur@gmail.com> 2015-09-17;
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public Licence as published by
 * the Free Software Foundation, either version 3 of the licence or,
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for details.
 * 
 * You should've received a copy of the GNU General Public License
 * with this program. If not, see <http://www.gnu.org/licences/>,
 * or write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330 Boston MA 02111-1307 USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <signal.h>

char const helpText[] =
 "\n"
 "Usage: vee [OPTION] [FD]...\n"
 "\n"
 "Copy stdin to each FD (file descriptor)*.\n"
 "\n"
 "  -h, --help              Print this help text and exit.\n"
 "  -i, --ignore-interrupts Ignore interrupt signals (SIGINT).\n"
 "\n"
 " * File descriptors are expected to be positive integers.\n";

char const unrecognizedOption[] = "vee: Unrecognized option: ";
char const mallocFailed[] = "vee: Unable to allocate memory for FD list.\n";

#define strToUInt_m(str, val) \
val = 0; \
for(int c = *str; c; str += 1, c = *str) \
{ \
 if(c < '0' || c > '9') \
 { \
  val = -1; \
  break; \
 } \
 c -= '0'; \
 val *= 10; \
 val += c; \
}

#define handleOption_m(str) \
if(!strcmp(str, "-i") || !strcmp(str, "--ignore-interrupts")) \
{ \
 signal(SIGINT, SIG_IGN); \
} \
else \
if(!strcmp(str, "-h") || !strcmp(str, "--help")) \
{ \
 write(1, helpText + 1, sizeof(helpText) - 2); \
 goto cleanup; \
} \
else \
{ \
 write(2, unrecognizedOption, sizeof(unrecognizedOption) - 1); \
 write(2, str, strlen(str)); \
 write(2, helpText, sizeof(helpText) - 1); \
 exitstatus = EXIT_FAILURE; \
 goto cleanup; \
}

int main(int argc, char * * argv)
{
 int exitstatus = EXIT_SUCCESS;
 /* Make array to hold the file descriptors to write to. */
 int * const restrict fds = malloc(argc * sizeof(int));
 if(!fds)
 {
  write(2, mallocFailed, sizeof(mallocFailed) - 1);
  return EXIT_FAILURE;
 }
 size_t fdcount = 0;
 /* If there are arguments, the first argument is the program name: skip it. */
 for(size_t i = 1; i < argc; i += 1)
 {
  char const * restrict argPtr1 = argv[i];
  char const * restrict argPtr2 = argPtr1;
  int fd;
  strToUInt_m(argPtr1, fd)
  if(fd >= 0 && argPtr1 > argPtr2)
  {
   fds[fdcount] = fd;
   fdcount += 1;
   continue;
  }
  handleOption_m(argPtr2)
 }
 
 #ifdef SIGPIPE
  signal(SIGPIPE, SIG_IGN);
 #endif
 
 char buffer[BUFSIZ];
 ssize_t readcount;
 while((readcount = read(0, &buffer, BUFSIZ)) > 0)
 {
  for(size_t i = 0; i < fdcount; i += 1)
  {
   if(write(fds[i], &buffer, readcount) != readcount)
   {
    exitstatus = EXIT_FAILURE;
   }
  }
 }
cleanup:
 free(fds);
 return exitstatus;
}
