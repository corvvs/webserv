#!/usr/bin/ruby

time = rand(1..30)
sleep(time)

$stdout.puts [
  "Status: 200 OK",
  "Content-Type: text/plain",
  "",
  "done: #{time}",
]
