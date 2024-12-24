#ifndef DBUS_CLIENT_H_
#define DBUS_CLIENT_H_

int connect_dbus_client (void);
void close_dbus_client (void);

int trigger_update_database (void);
int trigger_cancel_rule (void);
int trigger_schedule (void);
int trigger_custom_schedule (void);

#endif /* DBUS_CLIENT_H */
