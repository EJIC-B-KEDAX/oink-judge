#!/usr/bin/bash

pkill -f run_web.py
pkill -f auth_service
pkill -f content_service_server
pkill -f dispatcher
pkill -f test_node
