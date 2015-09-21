/*
 * vee 3.5.1
 * Copyright (C) Alexander Kozhevnikov <mentalisttraceur@gmail.com> 2015-09-21;
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

#define _POSIX_C_SOURCE 1

#include <limits.h>

#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <signal.h>

#define UCHAR_HALF_MAX (UCHAR_MAX >> 1)
/*\
The maximum value of the bottom "half" of the value range of an unsigned char.
Also the value that sets all but the most significant bit of an unsigned char.
\*/

#define UCHAR_TOP_BIT (UCHAR_HALF_MAX + 1)
/*\
The most significant bit of an unsigned char.
\*/

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
char const fdOverflowedInt[] = "vee: FD value greater than maximum possible: ";

#define bytepack_m(ptr, val) \
if(!val) \
{ \
 ptr[0] = UCHAR_TOP_BIT; \
 ptr[1] = 0; \
} \
else \
{ \
 do \
 { \
  unsigned char bytepack_m_tempUChar = val; \
  val >>= CHAR_BIT - 1; \
  if(val) \
  { \
   bytepack_m_tempUChar |= UCHAR_TOP_BIT; \
  } \
  *ptr = bytepack_m_tempUChar; \
  ptr += 1; \
 } \
 while(val); \
}

#define byteback_m(ptr, val) \
for \
( \
 unsigned char byteback_m_shift = 0, \
  byteback_m_continueBit = UCHAR_TOP_BIT; \
 byteback_m_continueBit; \
 byteback_m_shift += CHAR_BIT - 1, ptr += 1 \
) \
{ \
 unsigned char byteback_m_tempUChar = *ptr; \
 byteback_m_continueBit &= byteback_m_tempUChar; \
 byteback_m_tempUChar &= UCHAR_HALF_MAX; \
 val += byteback_m_tempUChar << byteback_m_shift; \
}

#define strToFD_m(str, val) \
val = 0; \
for(int c = *str; c; str += 1, c = *str) \
{ \
 if(c < '0' || c > '9') \
 { \
  val = -1; \
  break; \
 } \
 if(val <= INT_MAX / 10) \
 { \
  val *= 10; \
  c -= '0'; \
  if(val <= INT_MAX - c) \
  { \
   val += c; \
   continue; \
  } \
 } \
 write(2, fdOverflowedInt, sizeof(fdOverflowedInt) - 1); \
 write(2, argPtr2, strlen(argPtr2)); \
 return EXIT_FAILURE; \
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
 return EXIT_SUCCESS; \
} \
else \
{ \
 write(2, unrecognizedOption, sizeof(unrecognizedOption) - 1); \
 write(2, str, strlen(str)); \
 write(2, helpText, sizeof(helpText) - 1); \
 return EXIT_FAILURE; \
}

int main(int argc, char * * argv)
{
 int fd;
 /* If there are arguments, the first argument is the program name: skip it. */
 for(size_t i = 1; i < argc; i += 1)
 {
  char * restrict argPtr1 = argv[i];
  char * restrict argPtr2 = argPtr1;
  strToFD_m(argPtr1, fd)
  if(fd >= 0 && argPtr1 > argPtr2)
  {
   bytepack_m(argPtr2, fd);
   continue;
  }
  handleOption_m(argPtr2)
  *argPtr2 = 0;
 }
 
 #ifdef SIGPIPE
  signal(SIGPIPE, SIG_IGN);
 #endif
 
 char buffer[PIPE_BUF];
 ssize_t readcount;
 int exitstatus = EXIT_SUCCESS;
 while((readcount = read(0, &buffer, PIPE_BUF)) > 0)
 {
  for(size_t i = 1; i < argc; i += 1)
  {
   unsigned char * restrict arg = argv[i];
   if(!*arg)
   {
    continue;
   }
   fd = 0;
   byteback_m(arg, fd)
   if(write(fd, &buffer, readcount) != readcount)
   {
    exitstatus = EXIT_FAILURE;
   }
  }
 }
 return exitstatus;
}
