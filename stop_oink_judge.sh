#!/usr/bin/bash

pkill -f run_web.py
pkill -f auth_server
pkill -f data_sender_server
pkill -f dispatcher_server
pkill -f test_node
