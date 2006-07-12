/*
 * Listener - Listens for specific directories events and take actions 
 * based on rules specified by the user.
 *
 * Copyright (c) 2005,2006  Lucas C. Villa Real <lucasvr@gobolinux.org>
 * 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "listener.h"
#include "rules.h"

char *
get_token(char *cmd, int *skip_bytes, char *pathname, struct thread_info *info)
{
	int i=0, j=0, skip=0;
	char line[LINE_MAX], work_line[LINE_MAX];
	char *entry_ptr, *ptr;

	if (! cmd || ! strlen(cmd)) {
		*skip_bytes = 0;
		return NULL;
	}

	memset(line, 0, sizeof(line));
	memset(work_line, 0, sizeof(work_line));

	while (isblank(*cmd)) {
		cmd++;
		skip++;
	}
	while (*cmd && ! isblank(*cmd)) {
		line[i++] = *(cmd++);
		skip++;
	}
	*skip_bytes = skip;

	if ((entry_ptr = strstr(line, "$ENTRY_RELATIVE"))) {
		int wi = 0;
		for (ptr=line; ptr != entry_ptr; ptr++)
			work_line[wi++] = (*ptr)++;

		for (j=0; j<strlen(info->offending_name); ++j)
			work_line[wi++] = info->offending_name[j];

		/* skip '$ENTRY_RELATIVE' and copy the remaining data */
		for (ptr+=15; *ptr; ptr++)
			work_line[wi++] = (*ptr)++;

		return strdup(work_line);

	} else if ((entry_ptr = strstr(line, "$ENTRY"))) {
		int wi = 0;
		for (ptr=line; ptr != entry_ptr; ptr++)
			work_line[wi++] = (*ptr)++;

		for (j=0; j<strlen(pathname); ++j)
			work_line[wi++] = pathname[j];

		work_line[wi++] = '/';

		for (j=0; j<strlen(info->offending_name); ++j)
			work_line[wi++] = info->offending_name[j];

		/* skip '$ENTRY' and copy the remaining data */
		for (ptr+=6; *ptr; ptr++)
			work_line[wi++] = (*ptr)++;

		return strdup(work_line);
	}
	return strdup(line);
}

int
expect_rule_start(FILE *fp)
{
	char *token;
	char buf[LINE_MAX];

	while (! feof (fp)) {
		memset(buf, 0, sizeof(buf));
		fgets(buf, sizeof(buf), fp);
		if ((buf == NULL) || (buf[0] == '#') || ((strlen(buf)) == 0))
			continue;

		token = strtok(buf, " \t\n");
		if (token == NULL)
			continue;

		if (token[strlen(token)-1] == '\n')
			token[strlen(token)-1] = '\0';

		if (! strcmp (token, "{"))
			return 0;
		else
			break;
	}

	return -1;
}

int
expect_rule_end(FILE *fp)
{
	char *token;
	char buf[LINE_MAX];

	while (! feof (fp)) {
		memset(buf, 0, sizeof(buf));
		fgets(buf, sizeof(buf), fp);
		if ((buf == NULL) || (buf[0] == '#') || ((strlen(buf)) == 0))
			continue;

		token = strtok(buf, " \t\n");
		if (token == NULL)
			continue;

		if (token[strlen(token)-1] == '\n')
			token[strlen(token)-1] = '\0';

		if (! strcmp (token, "}"))
			return 0;
		else
			break;
	}

	return -1;
}

char *
get_rule_for(char *entry, FILE *fp)
{
	char *token = NULL;
	char buf[LINE_MAX];

	while (! feof (fp)) {
		memset(buf, 0, sizeof(buf));
		fgets(buf, sizeof(buf), fp);
		if ((buf == NULL) || (buf[0] == '#') || ((strlen(buf)) == 0))
			continue;
		else if ((buf[0] == '{') || (buf[0] == '}'))
			return NULL;
		else
			break;
	}

	/* check for ENTRY match */
	if (! strstr (buf, entry))
		return NULL;

	token = strtok(buf, "=");
	if (! token)
		return NULL;

	/* get the RULE associated with ENTRY */
	token = token + strlen(token) + 1;
	while (*token == '\t' || *token == ' ')
		token++;

	if (! token)
		return NULL;

	if (token[strlen(token)-1] == '\n')
		token[strlen(token)-1] = '\0';

	return strdup(token);
}

int
parse_masks(char *masks, int rule)
{
	int ret = EMPTY_MASK;

	if ((strstr(masks, "CLOSE_WRITE")))
		ret |= IN_CLOSE_WRITE;
	if ((strstr(masks, "MOVED_TO")))
		ret |= IN_MOVED_TO;
	if ((strstr(masks, "DELETE")))
		ret |= IN_DELETE;
	if ((strstr(masks, "CREATE")))
		ret |= IN_CREATE;
	if ((strstr(masks, "MODIFY")))
		ret |= IN_MODIFY;

	return ret;
}

static int num_subdirs = 1;

int
count_subdirs(const char *file, const struct stat *sb, int flag)
{
	if (flag == FTW_D) {
		/* is a subdirectory */
		dprintf("-> %s\n", file);
		num_subdirs++;
	}
	return 0;
}

extern int monitor_index;

struct directory_info **
assign_rules(char *config_file, int *retval)
{
	int i, n, subdirs, ret;
	FILE *fp;
	char *token, regex_rule[LINE_MAX];
	struct directory_info **dir_info;


	/* we didn't have success on the operation yet */
	*retval = -1;
	
	fp = fopen(config_file, "r");
	if (! fp) {
		fprintf(stderr, "fopen %s: %s\n", config_file, strerror(errno));
		return NULL;
	}

	/* read how many rules we have */
	n = subdirs = 0;
	while (! feof(fp)) {
		char buf[LINE_MAX];
		char pathname[LINE_MAX];

		memset(buf, 0, sizeof(buf));
		fgets(buf, sizeof(buf), fp);
		if ((buf == NULL) || (buf[0] == '#') || ((strlen(buf)) == 0))
			continue;
		else if (buf[0] == '{')
			n++;
		else if (strstr(buf, "TARGET")) {
			char *token = strtok(buf, " \t");
			token = strtok(NULL, " \t");
			token = strtok(NULL, " \t");
			if (! token) {
				fprintf(stderr, "Error: one or more TARGET entries don't have a value assigned to\n");
				return NULL;
			}
			token[strlen(token)-1] = '\0';
			sprintf(pathname, token);

		} else if (strstr(buf, "RECURSIVE") && strstr(buf, "YES")) {
			ftw(pathname, count_subdirs, 1024);
			subdirs += num_subdirs;
			num_subdirs = 0;
		}
	}

	/* there are no rules at all */
	if (n == 0) {
		*retval = 0;
		return NULL;
	}

	dprintf("total subdirs = %d, allocating %d entries\n", subdirs, n+1+subdirs);
	dir_info = (struct directory_info **) calloc(n+1+subdirs, sizeof(struct directory_info *));
	if (! dir_info) {
		perror("calloc");
		return NULL;
	}

	rewind(fp);
	
	/* register the pathname */
	for (monitor_index = 0; monitor_index < n+subdirs; /* do not increment */) {
		/* monitor_index is incremented inside monitor_directory() */
		i = monitor_index;

		/* expects to find the '{' character */
		if ((expect_rule_start(fp)) < 0) {
			fprintf(stderr, "Error: could not find the rule's start marker '{'\n");
			return NULL;
		}

		/* populates the dir_info struct */
		token = get_rule_for("TARGET", fp);
		if (! token) {
			fprintf(stderr, "Error on rule #%d: missing TARGET entry\n", i+1);
			return NULL;
		}

		dir_info[i] = (struct directory_info *) calloc(1, sizeof(struct directory_info));
		snprintf(dir_info[i]->pathname, sizeof(dir_info[i]->pathname), token);
		free(token);

		/* register the masks */
		token = get_rule_for("WATCHES", fp);
		if (! token) {
			fprintf(stderr, "Error on rule #%d: missing WATCHES entry\n", i+1);
			return NULL;
		}

		dir_info[i]->mask = parse_masks(token, i+1);
		if (dir_info[i]->mask == EMPTY_MASK) {
			fprintf(stderr, "Error on rule #%d: invalid WATCH %s\n", i+1, token);
			return NULL;
		}
		free(token);

		/* get the exec command */
		token = get_rule_for("SPAWN", fp);
		if (! token) {
			fprintf(stderr, "Error on rule #%d: missing SPAWN command\n", i+1);
			return NULL;
		}
		snprintf(dir_info[i]->exec_cmd, sizeof(dir_info[i]->exec_cmd), token);

		/* if there's $ENTRY on the SPAWN command, this rule expects it to exist */
		dir_info[i]->depends_on_entry = (strstr(token, "$ENTRY") == NULL ? 0 : 1);
		free(token);

		/* get the filters */
		token = get_rule_for("LOOKAT", fp);
		if (! token) {
			fprintf(stderr, "Error on rule #%d: missing LOOKAT entry\n", i+1);
			return NULL;
		}

		if (! strcasecmp(token, "DIRS"))
			dir_info[i]->filter = S_IFDIR;
		else if (! strcasecmp(token, "FILES"))
			dir_info[i]->filter = S_IFREG;
		else {
			fprintf(stderr, "Error on rule #%d: invalid LOOKAT option %s\n", i+1, token);
			free(token);
			return NULL;
		}
		free(token);

		/* get the regex rule */
		token = get_rule_for("ACCEPT_REGEX", fp);
		if (! token) {
			fprintf(stderr, "Error on rule #%d: missing ACCEPT_REGEX entry\n", i+1);
			return NULL;
		}

		snprintf(regex_rule, sizeof(regex_rule), "%s", token);
		free(token);

		ret = regcomp(&dir_info[i]->regex, regex_rule, REG_EXTENDED);
		if (ret != 0) {
			char err_msg[256];
			regerror(ret, &dir_info[i]->regex, err_msg, sizeof(err_msg) - 1);
			fprintf(stderr, "Regex error \"%s\": %s\n", regex_rule, err_msg);
			return NULL;
		}

		/* get the recursive flag */
		token = get_rule_for("RECURSIVE", fp);
		if (! token) {
			fprintf(stderr, "Error on rule #%d: missing RECURSIVE entry\n", i+1);
			return NULL;
		}

		if (! strcasecmp(token, "NO"))
			dir_info[i]->recursive = 0;
		else if (! strcasecmp(token, "YES"))
			dir_info[i]->recursive = 1;
		else {
			fprintf(stderr, "Error on rule #%d: invalid RECURSIVE option %s\n", i+1, token);
			free(token);
			return NULL;
		}
		free(token);

		/* expects to find the '}' character */
		if ((expect_rule_end(fp)) < 0) {
			fprintf(stderr, "Error: could not find the rule's end marker '}'\n");
			return NULL;
		}

		/* create the monitor rules */
		ret = monitor_directory(i, dir_info);
		if (ret < 0) {
			return NULL;
		}
	}

	*retval = 0;
	
	fclose(fp);
	return dir_info;
}

