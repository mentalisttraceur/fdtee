/*
 * vee 3.0.0
 * Copyright (C) Alexander Kozhevnikov <mentalisttraceur@gmail.com> 2015-06-07;
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
#include <stdbool.h>

#include <unistd.h>
#include <signal.h>

char const * const helpText[] =
{
 "Usage: vee [OPTION] [FD]...\n\n",
 "Copy stdin to each FD (file descriptor)*.\n\n",
 "  -h, --help              Print this help text and exit.\n",
 "  -i, --ignore-interrupts Ignore interrupt signals (SIGINT)\n\n",
 " * File descriptors are expected to be positive integers.\n"
};

int strToUInt(char const * str)
{
 int c, retval = 0;
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

void printUsageAndExit(int const exitcode, FILE * const ostream)
{
 size_t len = sizeof(helpText) / sizeof(char *);
 for(size_t i = 0; i < len; i += 1)
 {
  fputs(helpText[i], ostream);
 }
 exit(exitcode);
}

void handleOption(char const * const str)
{
 if(!strcmp(str, "-i") || !strcmp(str, "--ignore-interrupts"))
 {
  signal(SIGINT, SIG_IGN);
  return;
 }
 else
 if(!strcmp(str, "-h") || !strcmp(str, "--help"))
 {
  printUsageAndExit(EXIT_SUCCESS, stdout);
 }
 fputs("vee: Unrecognized option: ", stderr);
 fputs(str, stderr);
 fputc('\n', stderr);
 printUsageAndExit(EXIT_FAILURE, stderr);
}

int main(int argc, char * * argv)
{
 char c;
 
 /* If there are arguments, the first argument is the program name: skip it. */
 if(argc)
 {
  argc -= 1;
  argv += 1;
 }
 
 /* Make array to hold the file descriptors to write to. */
 int * const fds = malloc(argc * sizeof(int));
 size_t fdcount = 0;
 for(size_t i = 0; i < argc; i += 1)
 {
  char * const arg = argv[i];
  fds[fdcount] = strToUInt(arg);
  if(fds[fdcount] >= 0)
  {
   fdcount += 1;
   continue;
  }
  handleOption(arg);
 }
 
 signal(SIGPIPE, SIG_IGN);
 
 ssize_t readcount;
 while(readcount = read(0, &c, 1))
 {
  for(size_t i = 0; i < fdcount; i += 1)
  {
   write(fds[i], &c, readcount);
  }
 }
 return EXIT_SUCCESS;
}
