#include "socket/SocketConnected.hpp"
#include "socket/SocketListening.hpp"
#include "string"
#include <iostream>

int main(void) {
  SocketListening *l = SocketListening::bind(SD_IP4, ST_TCP, 8080);
  l->listen(1024);
  SocketConnected *c = NULL;
  while (c == NULL) {
    c = l->accept();
  }
  std::string request(100, 0);
  std::string response("HTTP/1.1 200 OK\n"
                       "Age: 458104\n"
                       "Cache-Control: max-age=604800\n"
                       "Content-Type: text/html; charset=UTF-8\n"
                       "Date: Fri, 24 Jun 2022 10:14:06 GMT\n"
                       "Etag: \"3147526947+ident\"\n"
                       "Expires: Fri, 01 Jul 2022 10:14:06 GMT\n"
                       "Last-Modified: Thu, 17 Oct 2019 07:18:26 GMT\n"
                       "Server: ECS (sab/56CF)\n"
                       "Vary: Accept-Encoding\n"
                       "X-Cache: HIT\n"
                       "Content-Length: 1256\n"
                       "\n"
                       "<!doctype html>\n"
                       "<html>\n"
                       "<head>\n"
                       "    <title>Example Domain</title>\n"
                       "\n"
                       "    <meta charset=\"utf-8\" />\n"
                       "    <meta http-equiv=\"Content-type\" content=\"text/html; charset=utf-8\" />\n"
                       "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />\n"
                       "    <style type=\"text/css\">\n"
                       "    body {\n"
                       "        background-color: #f0f0f2;\n"
                       "        margin: 0;\n"
                       "        padding: 0;\n"
                       "        font-family: -apple-system, system-ui, BlinkMacSystemFont, \"Segoe UI\", \"Open Sans\", \"Helvetica Neue\", Helvetica, Arial, sans-serif;\n"
                       "        \n"
                       "    }\n"
                       "    div {\n"
                       "        width: 600px;\n"
                       "        margin: 5em auto;\n"
                       "        padding: 2em;\n"
                       "        background-color: #fdfdff;\n"
                       "        border-radius: 0.5em;\n"
                       "        box-shadow: 2px 3px 7px 2px rgba(0,0,0,0.02);\n"
                       "    }\n"
                       "    a:link, a:visited {\n"
                       "        color: #38488f;\n"
                       "        text-decoration: none;\n"
                       "    }\n"
                       "    @media (max-width: 700px) {\n"
                       "        div {\n"
                       "            margin: 0 auto;\n"
                       "            width: auto;\n"
                       "        }\n"
                       "    }\n"
                       "    </style>    \n"
                       "</head>\n"
                       "\n"
                       "<body>\n"
                       "<div>\n"
                       "    <h1>Example Domain</h1>\n"
                       "    <p>This domain is for use in illustrative examples in documents. You may use this\n"
                       "    domain in literature without prior coordination or asking for permission.</p>\n"
                       "    <p><a href=\"https://www.iana.org/domains/example\">More information...</a></p>\n"
                       "</div>\n"
                       "</body>\n"
                       "</html>");
  ssize_t n = c->receive((void *)request.data(), request.size(), 0);
  c->send((void *)response.data(), response.size(), 0);
  std::cout << n << std::endl;
  std::cout << request << std::endl;
  delete l;
  delete c;
}
