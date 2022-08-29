#!/usr/bin/ruby
# exit(1)する

$stderr.puts "closing..."
$stdout.close
$stderr.puts "closed."
sleep(10)
exit(1)
