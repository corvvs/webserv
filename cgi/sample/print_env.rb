require "cgi"

cgi = CGI.new
cgi.out{ "[CGI ENVIRON]\n" }

# "GATEWAY_INTERFACE"
cgi.print "GATEWAY_INTERFACE\n"
cgi.print cgi.gateway_interface
cgi.print "\n\n"

# "REQUEST_METHOD"
cgi.print "REQUEST_METHOD\n"
cgi.print cgi.request_method
cgi.print "\n\n"


# "SERVER_PROTOCOL"
cgi.print "SERVER_PROTOCOL\n"
cgi.print cgi.server_protocol
cgi.print "\n\n"


# "CONTENT_TYPE"
cgi.print "CONTENT_TYPE\n"
cgi.print cgi.content_type
cgi.print "\n\n"


# "SERVER_PORT"
cgi.print "SERVER_PORT\n"
cgi.print cgi.server_port
cgi.print "\n\n"


# "SERVER_NAME"
cgi.print "SERVER_NAME\n"
cgi.print cgi.server_name
cgi.print "\n\n"

# "CONTENT_LENGTH"
cgi.print "CONTENT_LENGTH\n"
cgi.print cgi.content_length
cgi.print "\n\n"

# "PATH_INFO"
cgi.print "PATH_INFO\n"
cgi.print cgi.path_info
cgi.print "\n\n"


# "SCRIPT_NAME"
cgi.print "SCRIPT_NAME\n"
cgi.print cgi.script_name
cgi.print "\n\n"


# "QUERY_STRING"
cgi.print "QUERY_STRING\n"
cgi.print cgi.query_string
cgi.print "\n\n"


# "REQUEST_URI"
cgi.print "REQUEST_URI\n"
cgi.print ENV['REQUEST_URI']
cgi.print "\n\n"

