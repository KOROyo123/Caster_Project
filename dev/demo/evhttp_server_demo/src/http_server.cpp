
#include "http_server.h"

#include "stdio.h"

#include "event2/http.h"
#include "event2/util.h"
#include "event.h"

/*
POST ：表示向指定资源提交数据，数据包含在请求头中。有可能导致新的资源建立或原有资源修改。 POST 请求是 HTTP 请求中使用最多的一种请求方式。
GET ：表示请求指定的页面信息，并返回实体内容。
HEAD ：类似于 GET，只不过返回的响应体中没有具体内容，只有报文头，用于获取报文头
PUT ：从客户端向服务器传送的数据取代指定的内容，即向指定的位置上传最新的内容。
PATCH ：对 PUT 方法的补充，用来对已知资源进行局部更新。
OPTIONS ：返回服务器针对特殊资源所支持的 HTML 请求方式 或 允许客户端查看服务器的性能。
DELETE ：请求服务器删除 Request-URL 所标识的资源。
CONNECT ：HTTP 1.1 中预留给能够将连接改为管道方式的代理服务器。
TRACE ：回显服务器收到的请求，主要用于测试和诊断。

https://blog.csdn.net/weixin_44135121/article/details/99670225
*/

// 绑定到/test路径下的回调
void test_request_cb(struct evhttp_request *req, void *arg)
{
    const char *cmdtype;
    struct evkeyvalq *headers;
    struct evkeyval *header;
    struct evbuffer *buf;

    // 根据请求类型分别处理
    switch (evhttp_request_get_command(req))
    {
    case EVHTTP_REQ_GET:
        cmdtype = "GET";
        break;
    case EVHTTP_REQ_POST:
        cmdtype = "POST";
        break;
    case EVHTTP_REQ_HEAD:
        cmdtype = "HEAD";
        break;
    case EVHTTP_REQ_PUT:
        cmdtype = "PUT";
        break;
    case EVHTTP_REQ_DELETE:
        cmdtype = "DELETE";
        break;
    case EVHTTP_REQ_OPTIONS:
        cmdtype = "OPTIONS";
        break;
    case EVHTTP_REQ_TRACE:
        cmdtype = "TRACE";
        break;
    case EVHTTP_REQ_CONNECT:
        cmdtype = "CONNECT";
        break;
    case EVHTTP_REQ_PATCH:
        cmdtype = "PATCH";
        break;
    default:
        cmdtype = "unknown";
        break;
    }

    printf("Received a %s request for %s\nHeaders:\n",
           cmdtype, evhttp_request_get_uri(req));

    // 获取header
    headers = evhttp_request_get_input_headers(req);
    // 依次读取header
    for (header = headers->tqh_first; header; header = header->next.tqe_next)
    {
        // header->key, header->value
        printf("  %s: %s\n", header->key, header->value);
    }

    // 读取正文
    buf = evhttp_request_get_input_buffer(req);
    puts("Input data: <<<");
    while (evbuffer_get_length(buf))
    {
        int n;
        char cbuf[128];
        n = evbuffer_remove(buf, cbuf, sizeof(cbuf));
        if (n > 0)
            (void)fwrite(cbuf, 1, n, stdout);
    }
    puts(">>>");

    // 回复请求
    evhttp_send_reply(req, 200, "OK", NULL);
}


/* Callback used for the /dump URI, and for every non-GET request:
 * dumps all information to stdout and gives back a trivial 200 ok */
void
dump_request_cb(struct evhttp_request *req, void *arg)
{
	const char *cmdtype;
	struct evkeyvalq *headers;
	struct evkeyval *header;
	struct evbuffer *buf;

	switch (evhttp_request_get_command(req)) {
	case EVHTTP_REQ_GET: cmdtype = "GET"; break;
	case EVHTTP_REQ_POST: cmdtype = "POST"; break;
	case EVHTTP_REQ_HEAD: cmdtype = "HEAD"; break;
	case EVHTTP_REQ_PUT: cmdtype = "PUT"; break;
	case EVHTTP_REQ_DELETE: cmdtype = "DELETE"; break;
	case EVHTTP_REQ_OPTIONS: cmdtype = "OPTIONS"; break;
	case EVHTTP_REQ_TRACE: cmdtype = "TRACE"; break;
	case EVHTTP_REQ_CONNECT: cmdtype = "CONNECT"; break;
	case EVHTTP_REQ_PATCH: cmdtype = "PATCH"; break;
	default: cmdtype = "unknown"; break;
	}

	printf("Received a %s request for %s\nHeaders:\n",
	    cmdtype, evhttp_request_get_uri(req));

	headers = evhttp_request_get_input_headers(req);
	for (header = headers->tqh_first; header;
	    header = header->next.tqe_next) {
		printf("  %s: %s\n", header->key, header->value);
	}

	buf = evhttp_request_get_input_buffer(req);
	puts("Input data: <<<");
	while (evbuffer_get_length(buf)) {
		int n;
		char cbuf[128];
		n = evbuffer_remove(buf, cbuf, sizeof(cbuf));
		if (n > 0)
			(void) fwrite(cbuf, 1, n, stdout);
	}
	puts(">>>");

	evhttp_send_reply(req, 200, "OK", NULL);
}


void user_request_cb(struct evhttp_request *req, void *arg)
{
    evhttp_send_reply(req, 200, "OK", NULL);
}

void station_request_cb(struct evhttp_request *req, void *arg)
{
    evhttp_send_reply(req, 200, "OK", NULL);
}

void client_request_cb(struct evhttp_request *req, void *arg)
{
    evhttp_send_reply(req, 200, "OK", NULL);

}

void unset_request_cb(struct evhttp_request *req, void *arg)
{
    evhttp_send_error(req, 404, "Document was not found");
}
