#ifndef PRIVILEGES_H_
#define PRIVILEGES_H_

int init_privileges (void);
int drop_privileges (void);
int drop_privileges_permanently (void);
int raise_privileges (void);
int check_user (void);

#endif /* PRIVILEGES_H_ */
