#!/usr/bin/env python

import urllib2, re, sys

user_agent = "Mozilla/5.0"
url = "http://www.phpbb.com/phpBB/"

r = urllib2.Request(url + "profile.php?mode=register&agreed=true")
r.add_header('User-Agent', user_agent)
f = urllib2.build_opener().open(r)
info = f.info()
if info.has_key('set-cookie'):
    cookies = info['set-cookie'].split(";")
cookiestr = ""
for c in cookies:
    m = re.compile(".*(phpbb[^=]*=[^ ]*).*").match(c)
    if m:
        cookiestr += m.group(1) + "; "
while True:
    l = f.readline()
    if not l:
        break
    m = re.compile(".*\"(profile[^\"]*confirm[^\"]*)\".*").match(l)
    if m:
        pic = m.group(1).replace("&amp;", "&")
r = urllib2.Request(url + pic)
r.add_header('User-Agent', user_agent)
r.add_header('Referer', url + "profile.php?mode=register&agreed=true")
r.add_header('Cookie', cookiestr)
r.add_header('Accept', "image/png,*/*;q=0.5")
f = urllib2.build_opener().open(r)
while True:
    l = f.readline()
    if not l:
        break
    sys.stdout.write(l)
f.close()

