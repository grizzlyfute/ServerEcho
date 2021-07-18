# install and deploy server echo an Debian GNU/LINUX

PROG="Server echo"
PROG_MSG="(prod)"
PROG_DIR="/opt/ServerEcho"
LOGFILE="/var/log/ServerEcho/serverEcho.log"
PROG_USER="TcpEchoUser"
PIDFILE="/var/run/ServerEcho.pid"
PROC_NAME="ServerEcho"

INIT_SCRIPT="ServerEchoInit"
LOGROTATE_CONF="logrotate"
PORTFILE="port"

if [ `id -u` -ne 0 ]
then
  printf "Error: must be root to run this script\n" >&2
  exit 1
fi

if ! test -e "$INIT_SCRIPT"
then
  printf "$INIT_SCRIPT doesn't exist\n" >&2
  exit 2
fi

if ! test -e "$LOGROTATE_CONF"
then
  printf "$LOGROTATE_CONF doesn't exist\n" >&2
  exit 2
fi

if ! test -e "$PORTFILE"
then
  printf "$PORTFILE doesn't exist\n" >&2
  exit 2
fi

if ! test -e "$PROC_NAME"
then
  printf "$PROC_NAME doesn't exist\n" >&2
  exit 2
fi

#create dir and copy file
DIRLOG=`dirname $LOGFILE`
if ! test -d "$DIRLOG"
then
  mkdir -p "$DIRLOG"
fi

if ! test -d "$PROG_DIR"
then
  mkdir -p "$PROG_DIR"
fi

cp -p "$PROC_NAME" "$PROG_DIR"
cp -p "$PORTFILE" "$PROG_DIR"
cp -p "$INIT_SCRIPT" /etc/init.d/`basename "$INIT_SCRIPT"`

#create user as it should be
if [ -n `grep $PROG_USER /etc/passwd` ]
then
  #no password as default
  useradd -d "$PROG_DIR" -c 'Server echo user' -s /bin/false $PROG_USER
fi

#set permission
chown root:$PROG_USER "$DIRLOG"
chown $PROG_USER:$PROG_USER "$PROG_DIR"
chown $PROG_USER:$PROG_USER "$PROG_DIR"/`basename "$PROC_NAME"`
chown $PROG_USER:$PROG_USER "$PROG_DIR"/`basename "$PORTFILE"`
chown root:root /etc/init.d/`basename "$INIT_SCRIPT"`

chmod 750 "$DIRLOG"
chmod 750 "$PROG_DIR"
chmod 750 "$PROG_DIR"/`basename "$PROC_NAME"`
chmod 640 "$PROG_DIR"/`basename "$PORTFILE"`
chmod 755 /etc/init.d/`basename "$INIT_SCRIPT"`

#logrotate
if test -d "/etc/logrotate.d"
then
  cp -p "$LOGROTATE_CONF" /etc/logrotate.d/"$PROC_NAME"
  /etc/init.d/rsyslog restart
else
  printf "No log rotate dir found\n" >&2
fi

#restarting service
update-rc.d `basename "$INIT_SCRIPT"` defaults
/etc/init.d/`basename "$INIT_SCRIPT"` start

