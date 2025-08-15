#!/bin/sh

# copyright(c) msakamoto-sf

. ./common.sh

EDGE_LEFT=10
EDGE_RIGHT=69
EDGE_TOP=5
EDGE_BOTTOM=20
MARK_HV_HORIZON=1
MARK_HV_VERTICAL=2
MARK_DIR_INCR=incr
MARK_DIR_DECR=decr
MARK_MIN_HORZ=`expr $EDGE_LEFT + 1`
MARK_MAX_HORZ=`expr $EDGE_RIGHT - 1`
MARK_MIN_VERT=`expr $EDGE_TOP + 1`
MARK_MAX_VERT=`expr $EDGE_BOTTOM - 1`
MARK_NUMS=10

# initialize mark array
# - decide horizontal/vertical at random
# - decide direction incremental(to right or bottom)/
#     decrement(to left or upper) at random
# - decide initial x/y coordinates at random
prepare_marks()
{
    i=0
    while [ $i -lt $MARK_NUMS ]; do
        rand buf 1 2
        case $buf in
        1)
            eval mark_hv_$i=\$MARK_HV_HORIZON
            ;;
        *)
            eval mark_hv_$i=\$MARK_HV_VERTICAL
            ;;
        esac
        rand buf 1 2
        case $buf in
        1)
            eval mark_dir_$i=\$MARK_DIR_INCR
            ;;
        *)
            eval mark_dir_$i=\$MARK_DIR_DECR
            ;;
        esac
        rand buf $MARK_MIN_HORZ $MARK_MAX_HORZ
        eval mark_x_$i=\$buf
        rand buf $MARK_MIN_VERT $MARK_MAX_VERT
        eval mark_y_$i=\$buf
        i=`expr $i + 1`
    done
}

# draw edge border like:
#  +----------+
#  |          |
#  +----------+
draw_border()
{
    cols=`expr $EDGE_RIGHT - $EDGE_LEFT - 2`
    i=0
    b1=+
    while [ $i -le $cols ]; do
        b1="-$b1"
        i=`expr $i + 1`
    done
    b1="+$b1"

    cursor $EDGE_LEFT $EDGE_TOP
    echo $b1

    rows=`expr $EDGE_BOTTOM - $EDGE_TOP - 2`
    i=0
    j=`expr $EDGE_TOP + 1`
    while [ $i -le $rows ]; do
        cursor $EDGE_LEFT $j
        ECHO '|'
        cursor $EDGE_RIGHT $j
        ECHO '|'
        i=`expr $i + 1`
        j=`expr $j + 1`
    done

    cursor $EDGE_LEFT $EDGE_BOTTOM
    echo $b1
}

# put_mark(old_x, old_y, new_x, new_y)
put_mark()
{
    cursor $1 $2
    ECHO ' \c'
    cursor $3 $4
    ECHO 'o\c'
}

# switch horizontal/vertical
switch_hv()
{
    mark_index=0
    while [ $mark_index -le $MARK_NUMS ]; do
        eval hv=\$mark_hv_$mark_index
        if [ "$hv" = "$MARK_HV_VERTICAL" ]; then
            hv=$MARK_HV_HORIZON
        else
            hv=$MARK_HV_VERTICAL
        fi
        eval mark_hv_$mark_index=\$hv
        mark_index=`expr $mark_index + 1`
    done
    cursor 0 0
}

# move_marks($index)
move_marks()
{
    i=$1
    eval hv=\$mark_hv_$i
    eval dir=\$mark_dir_$i
    eval x=\$mark_x_$i
    eval y=\$mark_y_$i
    if [ "$hv" = "$MARK_HV_VERTICAL" ]; then
        if [ "$dir" = "$MARK_DIR_DECR" ]; then
            y=`expr $y - 1`
            if [ $y -le $EDGE_TOP ]; then
                dir=$MARK_DIR_INCR
                y=`expr $y + 1`
            fi
        else
            y=`expr $y + 1`
            if [ $y -ge $EDGE_BOTTOM ]; then
                dir=$MARK_DIR_DECR
                y=`expr $y - 1`
            fi
        fi
    else
        if [ "$dir" = "$MARK_DIR_INCR" ]; then
            x=`expr $x + 1`
            if [ $x -ge $EDGE_RIGHT ]; then
                dir=$MARK_DIR_DECR
                x=`expr $x - 1`
            fi
        else
            x=`expr $x - 1`
            if [ $x -le $EDGE_LEFT ]; then
                dir=$MARK_DIR_INCR
                x=`expr $x + 1`
            fi
        fi
    fi
    eval mark_x_$i=\$x
    eval mark_y_$i=\$y
    eval mark_dir_$i=\$dir
}

# move and draw for each marks
update_marks()
{
    mark_index=0
    while [ $mark_index -le $MARK_NUMS ]; do
        eval old_x=\$mark_x_$mark_index
        eval old_y=\$mark_y_$mark_index
        move_marks $mark_index
        eval new_x=\$mark_x_$mark_index
        eval new_y=\$mark_y_$mark_index
        put_mark $old_x $old_y $new_x $new_y
        mark_index=`expr $mark_index + 1`
    done
    cursor 1 1
}

trap '' $SIGINT $SIGQUIT $SIGTSTP
trap : $SIGTERM

init_tty

draw_border
prepare_marks
cursor 1 1
echo " 'q' to quit, 'SPACE' switch horizontal/vertical..."
cursor 1 1

# invoke back ground jobs which output "~"
# after timeout second passed, 
# and output is redirected to second input-waiting-while-loop
(
    while :
    do
        sleep 1
        ECHO '~\c'
    done &
    /bin/cat
) | while :
do
    key=`getchar`
    case "$key" in
    '~')
        update_marks
        ;;
    q)
        # send SIGTERM to my process group
        kill -TERM 0
        break
        ;;
    ' ')
        switch_hv
        ;;
    *)
        ;;
    esac
done

quit_tty
