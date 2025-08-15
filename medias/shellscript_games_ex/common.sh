#!/bin/sh

#
# 「ゲームで極めるシェルスクリプトスーパーテクニック」
# 山森 丈範, 技術評論社
# より、共通関数を抜粋、収録
# 本スクリプトの著作権は上記著者にあります。
#

# common commands, optimizations

# generic "echo" for control codes
case "`echo -e`" in
	-e)
		ECHO() { echo "$@"; }
		;;
	*)
		ECHO() { echo -e "$@"; }
		;;
esac
case "`ECHO '\r'`" in
	'\r')
		case "`(print X) 2> /dev/null`" in
			X)
				ECHO() { print "$@"; }
				;;
			*)
				PATH=/usr/5bin:$PATH
				export PATH
				;;
		esac
		;;
esac

# generic "expr" command (optimized for built-in expr feature)
case "$((1))" in
	1)
		expr()
		{
			echo "$(($*))"
		}
		;;
esac 2>/dev/null

# (no arg)
cls()
{
	ECHO '\033[H\033[2J\c'
}

# cursor(column, row) NOTE : start with 1, not 0
cursor()
{
	ECHO '\033['$2';'$1'H\c'
}

beep()
{
	ECHO '\07\c'
}

new_screen()
{
	# save cursor position
	ECHO '\033\067\c'
	# open new screen
	ECHO '\033[?47h\c'
}

exit_screen()
{
	# close screen
	ECHO '\033[?47l\c'
	# restore cursor position
	ECHO '\033\070\c'
}

init_tty()
{
	stty -icanon -echo min 1 -ixon
	new_screen
	cls
}

quit_tty()
{
	cls
	exit_screen
	stty icanon echo eof '^d' ixon
}

getchar()
{
	dd bs=1 count=1 2> /dev/null
}

#trap
if (trap '' INT) 2> /dev/null
then
	SIGINT=INT SIGQUIT=QUIT SIGTERM=TERM SIGTSTP=TSTP
else
	SIGINT=2 SIGQUIT=3 SIGTERM=15 SIGTSTP=18
fi

# short size (2byte) random
if [ "$RANDOM" != "$RANDOM" ]
then
	random_short()
	{
		eval $1=$RANDOM
	}
elif [ -c /dev/urandom ]
then
	random_short()
	{
		rand_s=`(
			set \'dd if=/dev/urandom bs=2 count=1 2>/dev/null | od -d\'
			echo "$2"
		)`
		eval $1=$rand_s
	}
else
	seed=$$
	random_short()
	{
		seed=`expr $seed \* 256 % 32768 + \
		\( $seed / 16384 + $seed / 8192 \) % 2 \* 128 + \
		\( $seed /  8192 + $seed / 4096 \) % 2 \*  64 + \
		\( $seed /  4096 + $seed / 2048 \) % 2 \*  32 + \
		\( $seed /  2048 + $seed / 1024 \) % 2 \*  16 + \
		\( $seed /  1024 + $seed /  512 \) % 2 \*   8 + \
		\( $seed /   512 + $seed /  256 \) % 2 \*   4 + \
		\( $seed /   256 + $seed /  128 \) % 2 \*   2 + \
		\( $seed /   128 + $seed /   64 \) % 2`
		eval $1=$seed
	}
fi

# rand( _name_ min max )
rand()
{
	random_short rand_val
	rand_val=`expr $rand_val % 256 \* \( "$3" - "$2" + 1 \) / 256 + "$2"`
	eval $1=$rand_val
}

# partial_scroll_lock($row_from $row_to)
partial_scroll_lock()
{
    ECHO '\033['$1';'$2'r\c'
}

partial_scroll_unlock()
{
    ECHO '\033[r\c'
}

partial_scroll_up()
{
    ECHO '\033M\c'
}

