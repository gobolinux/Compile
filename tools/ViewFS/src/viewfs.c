
#define u_int u_int32_t
#define u_long u_int64_t

#include <db.h>
#include <stdio.h>
#define FUSE_USE_VERSION 25
#include <fuse/fuse_lowlevel.h>
#include <signal.h>
#include <unistd.h>

#include "events.h"
#include "cmdline.h"
#include "tree.h"
#include "watchdir.h"

static struct fuse_session *session;

tree_t* tree;

static void catch_signal(int sig) {
   fuse_session_exit(session);
}

int main(int argc, char** argv) {
   int err = 0;
   char* watch_dir;
   char* mountpoint;
   int foreground;
   err = parse_cmdline(argc, argv, &watch_dir, &mountpoint, &foreground);
   if (err) goto quit;
   int fd = fuse_mount(mountpoint, NULL);
   if (fd == -1) goto quit;
   session = viewfs_session_new();
   struct fuse_chan* chan = fuse_kern_chan_new(fd);
   fuse_session_add_chan(session, chan);
   signal(SIGHUP, catch_signal);
   signal(SIGTERM, catch_signal);
   signal(SIGINT, catch_signal);
   signal(SIGPIPE, SIG_IGN);
   tree = tree_open();
   if (!tree) goto quit_unmount;
   err = scan_watch_dir(tree, watch_dir);
   if (err) goto quit_unmount;
   fuse_session_loop(session);
   fuse_session_destroy(session);
   close(fd);
   tree_close(tree);
quit_unmount:
   fuse_unmount(mountpoint);
   free(mountpoint);
   free(watch_dir);
quit:
   return err;
}
