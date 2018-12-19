#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/signalfd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <glib.h>
#include <dbus/dbus.h>

#include "tbot.h"

DBusConnection *connection = NULL;

void set_dbus_connection(DBusConnection *conn)
{
	connection = conn;
}

DBusConnection *tbotd_get_dbus_connection(void)
{
	return connection;
}

int connect_bus(void)
{
	DBusConnection *conn;
	DBusError err;
	int result;

	dbus_error_init(&err);

	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if (dbus_error_is_set(&err) == TRUE) {
		printf("%s\n", err.message);
		return -EIO;
	}

	result = dbus_bus_request_name(conn, TBOT_NAME, DBUS_NAME_FLAG_DO_NOT_QUEUE, &err);

	if (dbus_error_is_set(&err) == TRUE) {
		printf("%s\n", err.message);
		dbus_connection_unref(conn);
		return -EALREADY;
	}

	if (result != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		dbus_set_error(&err, TBOT_NAME, "Name already in use");
		dbus_connection_unref(conn);
		return -EALREADY;
	}

	set_dbus_connection(conn);

	return 0;
}



void disconnect_dbus(void)
{
	DBusConnection *conn = tbotd_get_dbus_connection();

	if (!conn || !dbus_connection_get_is_connected(conn))
		return;

	set_dbus_connection(NULL);

	dbus_connection_unref(conn);
}
