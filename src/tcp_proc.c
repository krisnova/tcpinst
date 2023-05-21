/******************************************************************************\
 *                                                                            *
 *                    ███╗   ██╗ ██████╗ ██╗   ██╗ █████╗                     *
 *                    ████╗  ██║██╔═══██╗██║   ██║██╔══██╗                    *
 *                    ██╔██╗ ██║██║   ██║██║   ██║███████║                    *
 *                    ██║╚██╗██║██║   ██║╚██╗ ██╔╝██╔══██║                    *
 *                    ██║ ╚████║╚██████╔╝ ╚████╔╝ ██║  ██║                    *
 *                    ╚═╝  ╚═══╝ ╚═════╝   ╚═══╝  ╚═╝  ╚═╝                    *
 *              Written By: Kris Nóva    <admin@krisnova.net>                 *
 *                                                                            *
\******************************************************************************/

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tcpjack.h"

struct ProcEntry proc_entry_from_ino(char *ino) {
  struct ProcEntry pentry = {};
  struct dirent *procdentry;  // Procfs
  char needle[64] = "";
  snprintf(needle, 64, "socket:[%s]", ino);
  DIR *procdp = opendir("/proc");
  if (procdp == NULL) return pentry;
  while ((procdentry = readdir(procdp)) != NULL) {
    struct dirent *procsubdentry;  // Procfs Subdir
    char proc_dir[64];
    snprintf(proc_dir, 64, "/proc/%s/fd", procdentry->d_name);
    DIR *procsubdp = opendir(proc_dir);
    if (procsubdp == NULL) continue;
    while ((procsubdentry = readdir(procsubdp)) != NULL) {
      if (procsubdentry->d_type == DT_LNK) {
        // Read FD link
        char proc_fd_path[64];
        char fd_content[64] = "";
        snprintf(proc_fd_path, 64, "/proc/%s/fd/%s", procdentry->d_name,
                 procsubdentry->d_name);
        readlink(proc_fd_path, fd_content, 64);
        if (strcmp(fd_content, needle) == 0) {
          // Found the process
          pid_t pid = atoi(procdentry->d_name);
          char *comm = malloc(1024);
          char proc_comm_path[64];
          snprintf(proc_comm_path, 64, "/proc/%s/comm", procdentry->d_name);
          FILE *comm_f = fopen(proc_comm_path, "r");
          if (comm_f == NULL) return pentry;
          while (!feof(comm_f)) {
            fread(comm, sizeof comm, 1, comm_f);
          }
          int tcp_fd = atoi(procdentry->d_name);
          struct ProcEntry found_pentry = {
              .pid = pid,
              .comm = comm,
              .tcp_fd = tcp_fd
          };
          return found_pentry;
        }
      }
    }
  }
  return pentry;
}

void print_proc_entry(struct ProcEntry pentry) {
  printf("%d %s", pentry.pid, pentry.comm);
  free(pentry.comm);
}