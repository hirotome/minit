#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <wait.h>

typedef struct {
    const char *name;
    const char *run_path;
} Service;

static const Service services[] = {
    {"udevd", "/etc/minit/udevd/run"},
    {"dbus",  "/etc/minit/dbus/run"},
    {"seatd", "/etc/minit/seatd/run"},
    {"dhcpcd", "/etc/minit/dhcpcd/run"},
    {NULL, NULL}
};

static void logmsg(const char *msg)
{
    fprintf(stderr, "[minit] %s\n", msg);
    int fd = open("/dev/kmsg", O_WRONLY | O_NOCTTY);
    if (fd >= 0) {
        dprintf(fd, "[minit] %s\n", msg);
        close(fd);
    }
}

static pid_t launch(const char *name, const char *path)
{
    pid_t pid = fork();
    if (pid == 0) {
        execl(path, path, (char *)NULL);
        _exit(127);
    }
    if (pid > 0) {
        char buf[128];
        snprintf(buf, sizeof(buf), "started %s", name);
        logmsg(buf);
    }
    return pid;
}

static void wait_for_dbus(void)
{
	for (int i = 0; i < 300; i++) {
		if (access("/run/dbus/system_bus_socket", F_OK) == 0) return;
		usleep(100000);
	}
}

int main(void)
{
    signal(SIGCHLD, SIG_IGN);
    signal(SIGINT,  SIG_IGN);
    signal(SIGHUP,  SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    mkdir("/dev", 0755);
    mkdir("/dev/pts", 0755);
    mkdir("/dev/shm", 1777);
    mkdir("/run", 0755);
    mkdir("/tmp", 1777);
    mkdir("/var/log", 0755);
    mkdir("/sys/fs/cgroup", 0755);
    mount("proc",     "/proc",     "proc",     0, NULL);
    mount("sysfs",    "/sys",      "sysfs",    0, NULL);
    mount("devtmpfs", "/dev",      "devtmpfs", 0, NULL);
    mount("devpts",   "/dev/pts",  "devpts",   0, "mode=0620,gid=5");
    mount("tmpfs",    "/dev/shm",  "tmpfs",    MS_NOSUID|MS_NODEV, "mode=1777");
    mount("tmpfs",    "/run",      "tmpfs",    MS_NOSUID|MS_NODEV|MS_NOEXEC, NULL);
    mount("tmpfs",    "/tmp",      "tmpfs",    MS_NOSUID|MS_NODEV, NULL);
    mount("tmpfs",    "/var/log",  "tmpfs",    0, NULL);
    mount(NULL, "/", NULL, MS_REMOUNT, NULL);
    mount("cgroup2", "/sys/fs/cgroup", "cgroup2", 0, NULL);

    sethostname("void", 5);
    system("echo 'void' > /etc/hostname");

    int fd = open("/dev/console", O_RDWR | O_NOCTTY);
    if (fd >= 0) {
        dup2(fd, 0); dup2(fd, 1); dup2(fd, 2);
        close(fd);
    }

    logmsg("=== MINIT ===");

    for (int i = 0; services[i].name; i++) {
        if (strcmp(services[i].name, "dbus") == 0) {
        	launch(services[i].name, services[i].run_path);
        	wait_for_dbus();
        	continue;
        }
        launch(services[i].name, services[i].run_path);
    }

    system("udevadm trigger --type=devices --action=add");
    system("udevadm trigger --subsystem-match=usb");
    system("udevadm trigger --subsystem-match=input");
    system("udevadm trigger --subsystem-match=drm");
    system("udevadm settle --timeout=10 || true");

//    if (fork() == 0) {
//        execl("/usr/sbin/wpa_supplicant", "wpa_supplicant",
//        	  "-B", "-i", "wlo1", "-c", "/etc/wpa_supplicant/wpa_supplicant.conf", NULL);
//        _exit(1);
//    }
    
    while (1) {
        pid_t pid = fork();
        if (pid == 0) {
            execl("/sbin/agetty", "agetty", "--noclear", "38400", "tty1", "linux", NULL);
            _exit(1);
        }
        if (pid > 0) {
            waitpid(pid, NULL, 0);
        }
    }
    while (1) pause();
    return 0;
}
