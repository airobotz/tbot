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

#include "tbot.h"

#define SHUTDOWN_GRACE_SECONDS 10

static gboolean option_version = FALSE;

static GOptionEntry options[] = {
	{ "version", 'v', 0, G_OPTION_ARG_NONE, &option_version,
				"Show version information and exit" },
	{ NULL },
};

static GMainLoop *event_loop;

void tbot_exit(void)
{
	g_main_loop_quit(event_loop);
}

static gboolean quit_eventloop(gpointer user_data)
{
	tbot_exit();
	return FALSE;
}

static gboolean signal_handler(GIOChannel *channel, GIOCondition cond,
							gpointer user_data)
{
	static bool terminated = false;
	struct signalfd_siginfo si;
	ssize_t result;
	int fd;

	if (cond & (G_IO_NVAL | G_IO_ERR | G_IO_HUP))
		return FALSE;

	fd = g_io_channel_unix_get_fd(channel);

	result = read(fd, &si, sizeof(si));
	if (result != sizeof(si))
		return FALSE;

	switch (si.ssi_signo) {
	case SIGINT:
	case SIGTERM:
		if (!terminated) {
			g_timeout_add_seconds(SHUTDOWN_GRACE_SECONDS,
							quit_eventloop, NULL);

		}

		terminated = true;
		break;
	case SIGUSR2:
		break;
	}

	return TRUE;
}

static guint setup_signalfd(void)
{
	GIOChannel *channel;
	guint source;
	sigset_t mask;
	int fd;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGUSR2);

	if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
		perror("Failed to set signal mask");
		return 0;
	}

	fd = signalfd(-1, &mask, 0);
	if (fd < 0) {
		perror("Failed to create signal descriptor");
		return 0;
	}

	channel = g_io_channel_unix_new(fd);

	g_io_channel_set_close_on_unref(channel, TRUE);
	g_io_channel_set_encoding(channel, NULL, NULL);
	g_io_channel_set_buffered(channel, FALSE);

	source = g_io_add_watch(channel,
				G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				signal_handler, NULL);

	g_io_channel_unref(channel);

	return source;
}

void tbot_banner(void) 
{
	printf("/*** \n");
	printf("* \n");                               
 	printf("*     ##### #####   ####  ##### \n"); 
  	printf("*       #   #    # #    #   # \n");   
  	printf("*       #   #####  #    #   # \n");   
  	printf("*       #   #    # #    #   # \n");   
  	printf("*       #   #    # #    #   # \n");   
  	printf("*       #   #####   ####    # \n");   
  	printf("* \n");                               
  	printf("*/ \n");
}

int main(int argc, char *argv[])
{
	GOptionContext *context;
	GError *err = NULL;
	guint signal;

	tbot_banner();

	context = g_option_context_new(NULL);
	g_option_context_add_main_entries(context, options, NULL);

	if (g_option_context_parse(context, &argc, &argv, &err) == FALSE) {
		if (err != NULL) {
			g_printerr("%s\n", err->message);
			g_error_free(err);
		} else
			g_printerr("An unknown error occurred\n");
		exit(1);
	}

	g_option_context_free(context);

	if (option_version == TRUE) {
		printf("%s\n", TBOT_VERSION);
		exit(0);
	}

	event_loop = g_main_loop_new(NULL, FALSE);

	signal = setup_signalfd();

	if (connect_bus() < 0) {
		error("Unable to get on D-Bus");
		exit(1);
	}

	g_main_loop_run(event_loop);

	disconnect_dbus();

	printf("Quiting\n");

	g_source_remove(signal);

	g_main_loop_unref(event_loop);

	printf("Exit\n");

	return 0;
}