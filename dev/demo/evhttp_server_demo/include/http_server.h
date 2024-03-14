#pragma once

void test_request_cb(struct evhttp_request *req, void *arg);

void user_update_request_cb(struct evhttp_request *req, void *arg);

void mount_update_request_cb(struct evhttp_request *req, void *arg);

void info_update_request_cb(struct evhttp_request *req, void *arg);

void unset_request_cb(struct evhttp_request *req, void *arg);