#	stty dec crt
#fi

HOSTNAME=`hostname`
#PS1='$HOSTNAME[!]$PWD> '
PS2='rest?> '
export HOSTNAME PS1 PS2

CDPATH=:$HOME
ENV=$HOME/.bashrc
FCEDIT=/usr/bin/vi
#export HISTFILE=$HOME/.bash_history
#export HISTSIZE=25
#export HZ=100
MAIL=/usr/spool/mail/$USER
MBOX=$HOME/mail/mbox
PATH=$HOME/bin:/bin:/usr/bin:/etc:/usr/etc
VISUAL=/usr/bin/vi
#export TERMCAP=$HOME/etc/termcap

umask 022
trap clear 0
export CDPATH ENV FCEDIT
export MBOX SHELL VISUAL
export TERM MAIL PATH 

if [ -f ~/.bashrc ]; then source ~/.bashrc; fi
