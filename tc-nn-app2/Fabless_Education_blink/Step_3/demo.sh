#!/bin/bash

cleanup() {
    echo ""
    echo "exit sig"
    echo "exit_python"


    [ -n "$PY_PID" ] && sudo kill -SIGINT "$PY_PID" 2>/dev/null
    [ -n "$DASH_PID" ] && kill "$DASH_PID" 2>/dev/null
    [ -n "$FAB_PID" ] && kill "$FAB_PID" 2>/dev/null

    sudo pkill -f Data_parsing_QT.py
    sudo pkill -f dashboard
    sudo pkill -f fabless

    trap - SIGINT SIGTERM 
    kill -9 $$ 
}

trap cleanup SIGINT SIGTERM

export QT_QPA_PLATFORM=wayland

echo "topst" | sudo -S ifconfig eth0 192.168.0.250 up

echo "topst" | sudo -S python3 Data_parsing_QT.py &
PY_PID=$!
sleep 1

./dashboard/dashboard &
DASH_PID=$!
sleep 1

./fabless/fabless &
FAB_PID=$!

while true; do
    sleep 1
done
