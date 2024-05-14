#pragma once

void test_request_cb(struct evhttp_request *req, void *arg);

void user_request_cb(struct evhttp_request *req, void *arg);

void station_request_cb(struct evhttp_request *req, void *arg);

void client_request_cb(struct evhttp_request *req, void *arg);

void unset_request_cb(struct evhttp_request *req, void *arg);

void dump_request_cb(struct evhttp_request *req, void *arg);