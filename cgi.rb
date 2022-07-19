#!/usr/bin/ruby

ENV.each{ |k, v|
  $stderr.puts [k, v] * "="
}

content_length = (ENV["CONTENT_LENGTH"] || 0).to_i

data = $stdin.read(content_length)

cgi_header = {}

cgi_header["CONTENT-LENGTH"] = data.length
cgi_header["content-type"] = "application/json;hello=world"
cgi_header["location"] = "/test?this%3Dis_local"
cgi_header["location"] = "https://triple-underscore.github.io:8888/rfc-others/RFC3986-ja.html?hello=world#%E3%82%84%E3%81%BE%E3%81%82%E3%82%89%E3%81%97"
# cgi_header["location"] = "https://triple-underscore.github.io:8888/rfc-others/RFC3986-ja.html#%E3%82%84%E3%81%BE%E3%81%82%E3%82%89%E3%81%97"
# cgi_header["location"] = "//rfc-others/RFC3986-ja.html?hello=world"

cgi_header.each{ |k, v|
  $stdout.puts "#{k}: #{v}"
}

$stdout.puts

$stdout.write(data)


