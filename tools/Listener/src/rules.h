#ifndef LISTENER_RULES_H
#define LISTENER_RULES_H 1

char *get_token(char *cmd, int *skip_bytes, char *pathname, struct thread_info *info);
int expect_rule_start(FILE *fp);
int expect_rule_end(FILE *fp);
char *get_rule_for(char *entry, FILE *fp);
int parse_masks(char *masks);
struct directory_info *assign_rules(char *config_file, int *retval);

#endif /* LISTENER_RULES_H */
