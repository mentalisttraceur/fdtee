/*
 * vee 3.3.2
 * Copyright (C) Alexander Kozhevnikov <mentalisttraceur@gmail.com> 2015-06-14;
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

static inline int strToUInt(char const * str)
{
 int retval = 0;
 for(int c = *str; c; str += 1, c = *str)
 {
  if(c < '0' || c > '9')
  {
   return -1;
  }
  c -= '0';
  retval *= 10;
  retval += c;
 }
 return retval;
}

static inline void handleOption(char const * const str)
{
 if(!strcmp(str, "-i") || !strcmp(str, "--ignore-interrupts"))
 {
  signal(SIGINT, SIG_IGN);
  return;
 }
 else
 if(!strcmp(str, "-h") || !strcmp(str, "--help"))
 {
  write(1, helpText + 1, sizeof(helpText) - 2);
  exit(EXIT_SUCCESS);
 }
 write(2, unrecognizedOption, sizeof(unrecognizedOption) - 1);
 write(2, str, strlen(str));
 write(2, helpText, sizeof(helpText) - 1);
 exit(EXIT_FAILURE);
}

int main(int argc, char * * argv)
{
 /* Make array to hold the file descriptors to write to. */
 int * const fds = malloc(argc * sizeof(int));
 size_t fdcount = 0;
 /* If there are arguments, the first argument is the program name: skip it. */
 for(size_t i = 1; i < argc; i += 1)
 {
  char const * const arg = argv[i];
  fds[fdcount] = strToUInt(arg);
  if(fds[fdcount] >= 0)
  {
   fdcount += 1;
   continue;
  }
  handleOption(arg);
 }
 
 signal(SIGPIPE, SIG_IGN);
 
 char buffer[BUFSIZ];
 ssize_t readcount;
 int exitcode = EXIT_SUCCESS;
 while((readcount = read(0, &buffer, BUFSIZ)) > 0)
 {
  for(size_t i = 0; i < fdcount; i += 1)
  {
   if(write(fds[i], &buffer, readcount) != readcount)
   {
    exitcode = EXIT_FAILURE;
   }
  }
 }
 return exitcode;
}
