#!/usr/bin/python3

from http import cookies
import os

HEADER = "Content-type: text/html"
APP_PAGE = """
<html>
    <head>
        <title>app</title>
    </head>
    <script>
        function buttonClick() {
            document.cookie=`password=`
            document.location="/cgi-bin/app.py"
        }
    </script>
    <body>
        <h1>hello app</h1>
        <input type="button" value="logout" onclick="buttonClick()">
    </body>
</html>
"""

LOGIN_PAGE = """
<html>
    <head>
        <title>app</title>
    </head>
    <script>
        function submitOnClick() {
            document.cookie=`password=${document.getElementById("pass").value}`
            document.location="/cgi-bin/app.py"
        }
    </script>
    <body>
    <h1>Please enter your password</h1>
    <input id="pass" type="password" value="" size="10">
    <input type="submit" value="submit" onclick="submitOnClick()">
    </body>
</html>
"""


def is_logged_in():
    cookie = cookies.SimpleCookie()
    cookie.load(os.environ["HTTP_COOKIE"])
    password = cookie["password"].value
    return password == "1231"


if __name__ == '__main__':
    if is_logged_in():
        print(HEADER)
        print(APP_PAGE)
    else:
        print(HEADER)
        print(LOGIN_PAGE)
