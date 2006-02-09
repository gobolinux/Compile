
#define u_int u_int32_t
#define u_long u_int64_t

#include <db.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "tree.h"

#define TREE_ROOT_ID 1

//#define DEBUG_TREE(...) fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n")
#define DEBUG_TREE(...)

/*

typedef struct {
   DB* dbp;
   uint32_t next_inode;
} tree_t; 

*/

#define TREE_FILE "/tmp/viewfs.db"

tree_t* tree_open() {
   tree_t* tree = malloc(sizeof(tree_t));
   tree->next_inode = TREE_ROOT_ID;
   u_int32_t flags;
   int err;
   err = db_create(&(tree->dbp), NULL, 0);
   if (err) return NULL;
   flags = DB_CREATE | DB_TRUNCATE; /* for now, don't persist data */
   err = tree->dbp->open(tree->dbp, NULL, TREE_FILE, NULL, DB_BTREE, flags, 0);
   if (err) return NULL;
   int id = tree_new_entry(tree, 0, "/");
   assert(id == TREE_ROOT_ID);
   return tree;
}

void tree_close(tree_t* tree) {
   // FIXME: shouldn't TREE_FILE be erased at this point?
   tree->dbp->remove(tree->dbp, TREE_FILE, NULL, 0);
   tree->dbp->close(tree->dbp, 0);
   free(tree);
}

static void make_key(DBT* key, int* inode) {
   memset(key, 0, sizeof(DBT));
   key->data = inode;
   key->size = sizeof(int);
   key->flags = DB_DBT_USERMEM;
}

/**
 * Obtain data for an inode.
 * @return 0 if no errors occurred; the DB error code otherwise.
 */
static int get_inode(tree_t* tree, int inode, DBT* key, DBT* data) {
   DEBUG_TREE("GET_INODE: inode %d", inode);
   char* dir_name;
   make_key(key, &inode);
   memset(data, 0, sizeof(DBT));
   int err = tree->dbp->get(tree->dbp, NULL, key, data, 0);
   if (err)
      fprintf(stderr, "DB Error: %s\n", db_strerror(err));
   return err;
}

/**
 * No checks are performed to verify if child was already in the dir node.
 * @return 0 if no errors occurred; the DB error code otherwise.
 */
static int add_child(tree_t* tree, int parent, int child) {
   DEBUG_TREE("ADD_CHILD: parent %d; child %d", parent, child);
   DBT key, data;
   int err = get_inode(tree, parent, &key, &data);
   if (err)
      return err;
   make_key(&key, &parent);
   char* buffer = malloc(data.size + sizeof(int));
   memcpy(buffer, data.data, data.size);
   memcpy(buffer + data.size, &child, sizeof(int));
   data.data = buffer;
   data.size += sizeof(int);
   err = tree->dbp->put(tree->dbp, NULL, &key, &data, 0);
   free(buffer);
   err = get_inode(tree, parent, &key, &data);
   if (err)
      fprintf(stderr, "DB Error: %s\n", db_strerror(err));
   return err;
}

/**
 * @return the inode number, or 0 in case of error.
 */
uint32_t tree_new_entry(tree_t* tree, int parent, char* name) {
   DEBUG_TREE("TREE_NEW_ENTRY: parent %d; name %s", parent, name);
   DBT key, data;
   int err = 0;
   int name_size = strlen(name) + 1;
   memset(&data, 0, sizeof(DBT));
   int inode = tree->next_inode++;
   if (inode == 0)
      return 0;
   make_key(&key, &inode);
   data.size = sizeof(int) + name_size;
   void* marshall = malloc(data.size);
   if (!marshall) return 0;
   memcpy(marshall, &inode, sizeof(int));
   memcpy(marshall + sizeof(int), name, name_size);
   data.data = marshall;
   err = tree->dbp->put(tree->dbp, NULL, &key, &data, DB_NOOVERWRITE);
   free(marshall);
   if (!err && parent != 0)
      err = add_child(tree, parent, inode);
   if (err)
      return 0;
   return inode;
}

/**
 * Obtain information about a node. Information about the node's parent, its
 * list of children and/or the filename of an inode entry are given,
 * depending on whether pointers are passed to its parameters. 
 * Memory for the list and/or name is allocated by this function and ownership
 * passes to the caller.
 *
 * @param tree Tree handle
 * @param dir_inode Inode number of the directory.
 * @param out_parent  If passed a non-NULL value, the inode number of
 * this node's parent is set at the given location.
 * @param out_children If passed a non-NULL value, it returns at the given
 * location either a pointer to a copy of array with children
 * inodes, or NULL if there are no children.
 * @param out_nr_children If passed a non-NULL value,
 * the number of children is returned at the given location.
 * @param out_name If passed a non-NULL value, it returns at the given
 * location a pointer to a string containing the file name.
 * @return Returns 0 if no errors occurred, or the DB error code, if any.
 */
int tree_get(tree_t* tree, int inode, int* out_parent, int** out_children, int* out_nr_children, char** out_name) {
   DEBUG_TREE("TREE_GET: inode %d", inode);
   DBT key, data;
   int err = get_inode(tree, inode, &key, &data);
   if (err) return err;
   if (out_parent) {
      *out_parent = *((int*) data.data);
   }
   char* name = data.data + sizeof(int);
   int len_name = strlen(name);
   if (out_name) {
      *out_name = malloc(len_name + 1);
      strncpy(*out_name, name, len_name + 1);
   }
   int offset = sizeof(int) + len_name + 1;
   int delta = data.size - offset;
   if (out_children) {
      assert(offset <= data.size);
      if (offset == data.size) {
         *out_children = NULL;
      } else {
         *out_children = malloc(delta);
         memcpy(*out_children, data.data + offset, delta);
      }
   }
   if (out_nr_children)
      *out_nr_children = delta / sizeof(int);
   return 0;
}

/**
 * Compare given string with filename of a node.
 *
 * @param tree Tree handle.
 * @param inode Inode number of the entry.
 * @param name Name to be compared with node name.
 * @return true if names are equal, false otherwise.
 */
bool tree_name_cmp(tree_t* tree, int inode, const char* name) {
   DEBUG_TREE("TREE_NAME_CMP: inode %d; name %s", inode, name);
   DBT key, data;
   int err = get_inode(tree, inode, &key, &data);
   if (err)
      return false;
   char* inode_name = data.data + sizeof(int);
   return (strcmp(name, inode_name) == 0);
}

/**
 * Lookup the inode number of a child.
 *
 * @param tree Tree handle.
 * @param parent Inode number of the parent node.
 * @param out_inode Returns the inode number of the child, or 0 if
 * the child was not found or an error ocurred.
 * @return Returns zero if there were no errors or the DB error code.
 * Child not found is not considered an error.
 */
int tree_lookup(tree_t* tree, int parent, const char* name, int* out_inode) {
   assert(out_inode);
   int ret = 0;
   int* children;
   int nr_children;
   *out_inode = 0;
   int err = tree_get(tree, parent, NULL, &children, &nr_children, NULL);
   if (err)
      return err;
   int i = 0;
   for (i = 0; i < nr_children; i++) {
      if (tree_name_cmp(tree, children[i], name)) {
         *out_inode = children[i];
         break;
      }
   }
   if (children)
      free(children);
   return 0;
}
