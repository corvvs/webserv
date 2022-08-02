package main

const (
	webservPort = "8080"
	validHeader = "Host: localhost:8080\r\n" +
		"User-Agent: curl/7.77.0\r\n" +
		"Accept: */*\r\n" +
		"Connection: close\r\n\r\n"
	requestLineGetRootHTTP11 = "GET / HTTP/1.1\r\n"
)

var (
	indexHtml = []byte(`<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Title</title>
</head>
<body>
<h1>hello world</h1>
</body>
</html>`)

	sampleHtml = []byte(`<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Title</title>
</head>
<body>
<h1>sample</h1>
</body>
</html>`)

	dirAHtml = []byte(`<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Title</title>
</head>
<body>
<h1>a</h1>
</body>
</html>`)

	dirBHtml = []byte(`<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Title</title>
</head>
<body>
<h1>b</h1>
</body>
</html>`)

	errorHtml = []byte(`<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Title</title>
</head>
<body>
<div style="text-align: center;"><h1>404 Not Found</h1></div>
<hr>
<div style="text-align: center;">webserv</div>
</body>
</html>`)
	あHtml = []byte(`<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Title</title>
</head>
<body>
<h1>あ</h1>
</body>
</html>`)
)
