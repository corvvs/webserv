FROM ubuntu:devel

RUN yes | unminimize
RUN apt-get update
RUN apt-get install -y valgrind man-db
RUN apt-get install -y nginx

RUN mkdir -p /home/dev
RUN echo hello > hello

CMD nginx -g "daemon off;"
