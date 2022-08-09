FROM httpd

RUN apt-get update -y && \
    apt-get install -y python3 && \
    apt-get install -y php && \
    find /usr/local/apache2/cgi-bin/ -type f| xargs -I{} chmod 777 {}
