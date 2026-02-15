#!/usr/bin/bash

pkill -f run_web.py
pkill -f oink_judge_auth_service
pkill -f oink_judge_content_service_server
pkill -f oink_judge_dispatcher
pkill -f oink_judge_test_node
