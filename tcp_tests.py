import socket
import subprocess
import os
import time
import signal

devnull = open('/dev/null', 'w')
server = subprocess.Popen(["./ipkcpd -h 127.0.0.1 -p 2024 -m tcp"], stdout=devnull, shell=True)

time.sleep(5)

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("127.0.0.1", 2024))
sock.sendall(b"HELLO\n")
sock.sendall(b"SOLVE (+ 1 2)\n")
sock.sendall(b"BYE\n")
response = b""
while True:
  data = sock.recv(1024)
  if not data:
    break
  response += data
sock.close()
if response == b"HELLO\nRESULT 3\nBYE\n":
  print("simple query: ok")
else:
  print ("simple query: not ok")

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("127.0.0.1", 2024))
sock.sendall(b"HELLO\nSOLVE (+ 1 2)\nBYE\n")
response = b""
while True:
  data = sock.recv(1024)
  if not data:
    break
  response += data
sock.close()
if response == b"HELLO\nRESULT 3\nBYE\n":
  print("more commands in one query: ok")
else:
  print ("more commands in one query: not ok")

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("127.0.0.1", 2024))
sock.sendall(b"HELLO\n")
sock.sendall(b"SOL")
sock.sendall(b"VE (+")
sock.sendall(b" 1 2)\n")
sock.sendall(b"BYE\n")
response = b""
while True:
  data = sock.recv(1024)
  if not data:
    break
  response += data
sock.close()
if response == b"HELLO\nRESULT 3\nBYE\n":
  print("command in multiple queries: ok")
else:
  print ("command in multiple queries: not ok")

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("127.0.0.1", 2024))
sock.sendall(b"HELLO\n")
sock.sendall(b"SOLVE (- 1 2)\n")
response = b""
while True:
  data = sock.recv(1024)
  if not data:
    break
  response += data
sock.close()
if response == b"HELLO\nBYE\n":
  print("negative result: ok")
else:
  print ("negative result: not ok")

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("127.0.0.1", 2024))
sock.sendall(b"SOLVE (+ 1 2)\n")
response = b""
while True:
  data = sock.recv(1024)
  if not data:
    break
  response += data
sock.close()
if response == b"BYE\n":
  print("no hello: ok")
else:
  print ("no hello: not ok")

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("127.0.0.1", 2024))
sock.sendall(b"HELLO\n")
sock.sendall(b"HELLO\n")
response = b""
while True:
  data = sock.recv(1024)
  if not data:
    break
  response += data
sock.close()
if response == b"HELLO\nBYE\n":
  print("double hello: ok")
else:
  print ("double hello: not ok")

# this test flakes :( 
# sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# sock.connect(("127.0.0.1", 2024))

server.send_signal(signal.SIGINT)
# time.sleep(3)
# # os.kill(pid, signal.SIGINT)
# response = b""
# while True:
#   data = sock.recv(1024)
#   if not data:
#     break
#   response += data
# sock.close()
# if response == b"BYE\n":
#   print("sigint: ok")
# else:
#   print ("sigint: not ok")