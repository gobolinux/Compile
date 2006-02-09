
#define _GNU_SOURCE
#include <stdio.h>
#define FUSE_USE_VERSION 25
#include <fuse/fuse_lowlevel.h>
#include <errno.h>
#include <sys/statvfs.h>

#include "tree.h"
#include "events.h"

#ifndef DEBUG_EVENTS
#define DEBUG_EVENTS(...) fprintf(stderr, __VA_ARGS__); fprintf(stderr, " (%d)\n", event_count++)
#endif

/*

typedef struct {
   int nr_children;
   int parent;
   int* children;
} dirstream_t;

*/

extern tree_t* tree;

static int event_count = 0;

static void viewfs_init(void* userdata) {
   DEBUG_EVENTS("init");
}

static void viewfs_destroy(void* userdata) {
   DEBUG_EVENTS("destroy");
}

static void viewfs_lookup(fuse_req_t req, fuse_ino_t parent, const char *name) {
   DEBUG_EVENTS("lookup: ino %d; name %s", parent, name);
   struct fuse_entry_param param = {
      .generation = 1, // FIXME: check handling of this parameter
      .attr_timeout = 1.0,
      .entry_timeout = 1.0,
      .attr = {
         .st_mode = S_IFDIR | 0755,
         .st_nlink = 2
      }
   };
   int child;
   int err = tree_lookup(tree, parent, name, &child);
   if (child) {
      param.ino = child;
      param.attr.st_ino = child;
      fuse_reply_entry(req, &param);
   } else {
      if (err)
         fuse_reply_err(req, EIO);
      fuse_reply_err(req, ENOENT);
   }
}

static void viewfs_forget(fuse_req_t req, fuse_ino_t ino, unsigned long nlookup) {
   DEBUG_EVENTS("forget");
}

static void viewfs_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
   DEBUG_EVENTS("getattr: ino %d", ino);
   int err = tree_get(tree, ino, NULL, NULL, NULL, NULL);
   if (!err) {
      struct stat attrs = {
         .st_mode = S_IFDIR | 0755,
         .st_nlink = 2
      };
      fuse_reply_attr(req, &attrs, 1.0);
   } else {
      fuse_reply_err(req, ENOENT);
   }
}

static void viewfs_readlink(fuse_req_t req, fuse_ino_t ino) {
   DEBUG_EVENTS("readlink");
}

static void viewfs_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
   DEBUG_EVENTS("opendir");
   dirstream_t* dir = malloc(sizeof(dirstream_t));
   dir->children = NULL; 
   dir->nr_children = 0;
   dir->parent = 0;
   int err = tree_get(tree, ino, &dir->parent, &dir->children, &dir->nr_children, NULL);
   if (err) {
      free(dir);
      fuse_reply_err(req, ENOENT);
      return;
   }
   dir->nr_children += 2; // "." and ".."
   fi->fh = (int) dir;
   fuse_reply_open(req, fi);
}

static void viewfs_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                   struct fuse_file_info *fi) {
   DEBUG_EVENTS("readdir: ino %d, size %d, off %d", ino, size, off);
   dirstream_t* dir = (dirstream_t*) (ptrdiff_t) fi->fh;
   if (off > dir->nr_children) {
      fuse_reply_err(req, EINVAL);
      return;
   }
   if (off == dir->nr_children) {
      fuse_reply_buf(req, NULL, 0);
      return;
   }
   char* buf = malloc(size);
   char* buf_off = buf;
   int left = size;
   while (off < dir->nr_children) {
      char* name;
      int err;
      struct stat stbuf;
      stbuf.st_mode = S_IFDIR | 0755;
      stbuf.st_nlink = 2;
      if (off == 0) {
         name = strdup(".");
         stbuf.st_ino = ino;
      } else if (off == 1) {
         name = strdup("..");
         stbuf.st_ino = dir->parent;
      } else {
         err = tree_get(tree, dir->children[off-2], NULL, NULL, NULL, &name);
         if (err) {
            free(buf);
            fuse_reply_err(req, EINVAL);
            return;
         }
         stbuf.st_ino = dir->children[off-2];
      }
      int len_name = strlen(name);
      size_t ent_size = fuse_dirent_size(len_name);
      if (left >= ent_size) {
         buf_off = fuse_add_dirent(buf_off, name, &stbuf, ++off);
         left -= ent_size;
         free(name);
      } else {
         free(name);
         break;
      }
   }
   fuse_reply_buf(req, buf, size - left);
   free(buf);
}

static void viewfs_releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
   DEBUG_EVENTS("releasedir");
   dirstream_t* dir = (dirstream_t*) (ptrdiff_t) fi->fh;
   free(dir->children);
   free(dir);
   fuse_reply_err(req, 0);
}

static void viewfs_statfs(fuse_req_t req) {
   DEBUG_EVENTS("statfs");
   struct statvfs stbuf = {
      .f_bfree = 0
   };
   fuse_reply_statfs(req, &stbuf);
}

struct fuse_session* viewfs_session_new() {
   struct fuse_lowlevel_ops ops = {
      .init = viewfs_init,
      .destroy = viewfs_destroy,
      .lookup = viewfs_lookup,
      .forget = viewfs_forget,
      .getattr = viewfs_getattr,
      .setattr = NULL,
      .readlink = viewfs_readlink,
      .mknod = NULL,
      .mkdir = NULL,
      .unlink = NULL,
      .rmdir = NULL,
      .symlink = NULL,
      .rename = NULL,
      .link = NULL,
      .open = NULL,
      .read = NULL,
      .write = NULL,
      .flush = NULL,
      .release = NULL,
      .fsync = NULL,
      .opendir = viewfs_opendir,
      .readdir = viewfs_readdir,
      .releasedir = viewfs_releasedir,
      .fsyncdir = NULL,
      .statfs = viewfs_statfs,
      .setxattr = NULL,
      .getxattr = NULL,
      .listxattr = NULL,
      .removexattr = NULL
   };
   return fuse_lowlevel_new(NULL, &ops, sizeof(struct fuse_lowlevel_ops), NULL);
}
