puts "Content-type: text/html"
puts

File.open("../sample.html", "r") do |f|
  print f.read
end