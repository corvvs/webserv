#!/usr/bin/ruby

ENV.each{ |k, v|
  $stderr.puts [k, v] * "="
}

content_length = (ENV["CONTENT_LENGTH"] || 0).to_i
$stderr.puts "[[content_length: #{content_length}]]"
data = $stdin.read(content_length)
$stderr.puts data

# [setting CGI headers]
cgi_header = {}
# cgi_header["CONTENT-LENGTH"] = data.length
cgi_header["content-type"] = "application/json;hello=world"
# cgi_header["location"] = "/test?this%3Dis_local"
# cgi_header["location"] = "https://triple-underscore.github.io:8888/rfc-others/RFC3986-ja.html?hello=world#%E3%82%84%E3%81%BE%E3%81%82%E3%82%89%E3%81%97"
# cgi_header["location"] = "https://triple-underscore.github.io:8888/rfc-others/RFC3986-ja.html#%E3%82%84%E3%81%BE%E3%81%82%E3%82%89%E3%81%97"
# cgi_header["location"] = "//rfc-others/RFC3986-ja.html?hello=world"

# [print CGI headers]
cgi_header.each{ |k, v|
  $stdout.puts "#{k}: #{v}"
}

# [print NL]
$stdout.puts

# [print CGI body]
# exit(255)
$stdout.write(data)
# data.chars.each_with_index{ |c, i|
#   $stderr.puts "sending: #{i}"
#   $stdout.write(c)
#   $stdout.flush
#   # sleep(0.1)
# }


