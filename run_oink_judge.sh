#!/usr/bin/bash

source venv/bin/activate
nohup python run_web.py >> logs/run_web.log 2>&1 &
nohup ./build/modules/auth_service/auth_service_server configs/auth/config.json configs/auth/credentials.json >> logs/auth_server.log 2>&1 &
nohup ./build/modules/content_service/oink_judge_content_service_server configs/content_service/config.json configs/content_service/credentials.json >> logs/data_sender_server.log 2>&1 &
nohup ./build/modules/dispatcher/oink_judge_dispacther configs/dispatcher/config.json configs/dispatcher/credentials.json >> logs/dispatcher_server.log 2>&1 &
nohup ./build/modules/test_node/oink_judge_test_node configs/test_node/config.json configs/test_node/credentials.json >> logs/test_node.log 2>&1 &
