/*****************************************************************************\
 * Copyright 2015 Alexander Kozhevnikov <mentalisttraceur@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
\*****************************************************************************/

#define _POSIX_C_SOURCE 1

#include <limits.h> /* PIPE_BUF, UCHAR_MAX, INT_MAX, CHAR_BIT */

#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE, size_t */
#include <string.h> /* strlen, strcmp */

#include <unistd.h> /* read, write, ssize_t */
#include <signal.h> /* signal, SIG_IGN, SIGPIPE, SIGINT */

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
 "Usage: fdtee [OPTION] [FD]...\n"
 "\n"
 "Copy stdin to each FD (file descriptor)*.\n"
 "\n"
 "  -h, --help              Print this help text and exit.\n"
 "  -i, --ignore-interrupts Ignore interrupt signals (SIGINT).\n"
 "\n"
 " * File descriptors are expected to be positive integers.\n"
;

char const unrecognizedOption[] = "fdtee: Unrecognized option: ";
char const fdOverflowedInt[] = "fdtee: FD value greater than maximum possible: ";

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
