#! /usr/bin/env python3

"""
started from free license script posted on stackexchange:
https://softwarerecs.stackexchange.com/questions/28138/converter-that-converts-eml-files-to-plain-text?newreg=48fb884924fd4d909777e5fccc8af2eb

Chris Koeritz: added processing to fix headers and to turn html parts into text 2023-11-08.
"""

#hmmm: would be nice to add the ability to specify a directory; script assumes current dir.

import os
from bs4 import BeautifulSoup
from email import message_from_file, header

def file_exists (f):
    return os.path.exists(os.path.join(path, f).replace("\\","/"))

def save_file (fn, cont):
    file = open(os.path.join(path, fn).replace("\\","/"), "wb")
    file.write(cont)
    file.close()

def construct_name (id, fn):
    id = id.split(".")
    id = id[0]+id[1]
    return id+"."+fn

def disqo (s):
    s = s.strip()
    if s.startswith("'") and s.endswith("'"): return s[1:-1]
    if s.startswith('"') and s.endswith('"'): return s[1:-1]
    return s

def disgra (s):
    s = s.strip()
    if s.startswith("<") and s.endswith(">"): return s[1:-1]
    return s

def pullout (m, key):
    Html = ""
    Text = ""
    Files = {}
    Parts = 0
    if not m.is_multipart():
        if m.get_filename():
            fn = m.get_filename()
            cfn = construct_name(key, fn)
            Files[fn] = (cfn, None)
            if file_exists(cfn): return Text, Html, Files, 1
            save_file(cfn, m.get_payload(decode=True))
            return Text, Html, Files, 1
        cp = m.get_content_type()
        if cp=="text/plain":
            Text += m.get_payload(decode=True).decode("utf-8")
        elif cp=="text/html":
            soup = BeautifulSoup(m.get_payload(decode=True).decode("utf-8"), features="html.parser")
            Html += soup.get_text('\n', strip=True)
        else:
            cp = m.get("content-type")
            try: id = disgra(m.get("content-id"))
            except: id = None
            o = cp.find("name=")
            if o==-1: return Text, Html, Files, 1
            ox = cp.find(";", o)
            if ox==-1: ox = None
            o += 5; fn = cp[o:ox]
            fn = disqo(fn)
            cfn = construct_name(key, fn)
            Files[fn] = (cfn, id)
            if file_exists(cfn): return Text, Html, Files, 1
            save_file(cfn, m.get_payload(decode=True))
        return Text, Html, Files, 1
    y = 0
    while 1:
        try:
            pl = m.get_payload(y)
        except: break
        t, h, f, p = pullout(pl, key)
        Text += t; Html += h; Files.update(f); Parts += p
        y += 1
    return Text, Html, Files, Parts

def extract (msgfile, key): 
    m = message_from_file(msgfile)
    From, To, Subject, Date = caption(m)
    Text, Html, Files, Parts = pullout(m, key)
    Text = Text.strip(); Html = Html.strip()
    msg = {"subject": Subject, "from": From, "to": To, "date": Date,
        "text": Text, "html": Html, "parts": Parts}
    if Files: msg["files"] = Files
    return msg

def clean_header(h):
    return str(header.make_header(header.decode_header(h)))

def caption (origin):
    Date = ""
    if "date" in origin: Date = clean_header(origin["date"]).strip()
    From = ""
    if "from" in origin: From = clean_header(origin["from"]).strip()
    To = ""
    if "to" in origin: To = clean_header(origin["to"]).strip()
    Subject = ""
    if "subject" in origin: Subject = clean_header(origin["subject"]).strip()
    return From, To, Subject, Date

if __name__ == "__main__":
    global path

    startdirname = "Email"
    num = 1
    for i in range(10000000):
        if os.path.exists(startdirname + str(num)) == False:
            os.makedirs("Email" + str(num))
            break
        else:
            num += 1


    for i in os.listdir("."):
        if i.endswith(".eml") == True:
            nam = i[:-4]
            path = "./" + startdirname + str(num) + "/" + nam

            os.makedirs("./" + startdirname + str(num) + "/" + nam)

            f = open(i, "r")
            emailDict = extract(f, f.name)
            f.close()

            textFile = ""

            froms = emailDict["from"]
            tos = emailDict["to"]
            subject = emailDict["subject"]
            parts = emailDict["parts"]
            date = emailDict["date"]
            txt = emailDict["text"]
            html = emailDict["html"]

            files = []
            if "files" in emailDict:
                for i in emailDict["files"]:
                    files.append(i)

            textFile += "From: " + froms + "\n"
            textFile += "To: " + tos + "\n"
            textFile += "Subject: " + subject + "\n"
            textFile += "Date: " + date + "\n\n"
            textFile += "Files: " + ", ".join(files) + "\n"
            textFile += "Parts: " + str(parts) + "\n\n"
            textFile += "Text:\n\n" + txt + "\n\n" 
            textFile += "HTML:\n\n" + html


            wf = open("./" + startdirname + str(num) + "/" + nam + "/" + "txt_" + nam + ".txt", "w")
            wf.write(textFile)
            wf.close()

