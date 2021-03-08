
#include <web_server/ContextParse.h>

bool ContextParse::parseRequest(Buffer* buf, Timestamp receive_time) {
    bool ok = true;
    bool has_more = true;

    while (has_more) {
        if (state_ == ExceptRequestLine) {
            const char* crlf = buf->findCRLF();
            if (crlf) {
                ok = processRequestLine(buf->peek(), crlf);
                if (ok) {
                    request_.setReceiveTime(receive_time);
                    buf->retrieveUntil(crlf + 2);
                    state_ = ExceptHeader;
                }
                else {
                    has_more = false;
                }
            }
            else {
                has_more = false;
            }
        }
        else if (state_ == ExceptHeader) {
            const char* crlf = buf->findCRLF();
            if (crlf) {
                const char* colon = std::find(buf->peek(), crlf, ':');
                if (colon != crlf) {
                    request_.addHeader(buf->peek(), colon, crlf);
                }
                else {
                    state_ = GotAll;
                    has_more = false;
                }
                buf->retrieveUntil(crlf + 2);
            }
            else {
                has_more = false;
            }
        }
        else if (state_ == ExceptBody) {
            has_more = false;
            state_ = GotAll;
        }
    }

    return ok;
}

bool ContextParse::processRequestLine(const char* begin, const char* end) {
    bool succeed = false;
    const char* start = begin;
    const char* space = std::find(start, end, ' ');

    if (space != end && request_.setMethod(start, space)) {
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space != end) {
            const char* question = std::find(start, space, '?');
            if (question != space) {
                request_.setPath(start, question);
                request_.setQuery(question, space);
            }
            else {
                request_.setPath(start, space);
            }
            start = space + 1;
            succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
            if (succeed) {
                if (*(end - 1) == '1') {
                    request_.setVersion(Request::Http11);
                }
                else if (*(end - 1) == '0') {
                    request_.setVersion(Request::Http10);
                }
                else {
                    succeed = false;
                }
            }
        }
    }

    return succeed;
}