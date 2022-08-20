#!/usr/bin/python3

from http import cookies
import os
import sys

HEADER = "Content-type: text/html"
APP_PAGE = """
<html>
    <head>
        <title>app</title>
    </head>
    <script>
        function buttonClick() {
            document.cookie=`password=`
            document.location="__LOCATION__"
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
            document.location="__LOCATION__"
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
    if "password" in cookie:
        password = cookie["password"].value
        return password == "1231"
    return False


if __name__ == '__main__':
    # for key, val in os.environ.items():
    #     print(f"{key}={val}", file=sys.stderr)
    print(HEADER)
    page = ""
    if is_logged_in():
        page = APP_PAGE
    else:
        page = LOGIN_PAGE
    print(page.replace("__LOCATION__", os.environ["SCRIPT_NAME"]))
