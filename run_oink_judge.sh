#!/usr/bin/bash

source venv/bin/activate
nohup python run_web.py >> logs/run_web.log 2>&1 &
nohup ./build/auth_server configs/auth/config.json configs/auth/credentials.json >> logs/auth_server.log 2>&1 &
nohup ./build/data_sender_server configs/data_sender/config.json configs/data_sender/credentials.json >> logs/data_sender_server.log 2>&1 &
nohup ./build/dispatcher_server configs/dispatcher/config.json configs/dispatcher/credentials.json >> logs/dispatcher_server.log 2>&1 &
nohup ./build/test_node configs/test_node/config.json configs/test_node/credentials.json >> logs/test_node.log 2>&1 &