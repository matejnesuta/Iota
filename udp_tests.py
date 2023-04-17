import socket
import subprocess
import os
import time
import signal

devnull = open('/dev/null', 'w')
server = subprocess.Popen(["./ipkcpd -h 127.0.0.1 -p 2024 -m udp"], stdout=devnull, shell=True)

time.sleep(5)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.settimeout(2)

addr = ("127.0.0.1", 2024)

sock.sendto(b"\x00\x07(* 4 5)",addr)

try:
    data, _= sock.recvfrom(1024)

except socket.timeout:
    print("REQUEST TIMED OUT")

if data == b"\x01\x00\x0220":
  print("normal request: ok")
else:
  print ("normal request: not ok")

sock.sendto(b"\x00\x07(- 4 5)",addr)

try:
    data, _= sock.recvfrom(1024)

except socket.timeout:
    print("REQUEST TIMED OUT")

if data == b"\x01\x01\x32IPKCP does not support negative numbers as results":
  print("negative result request: ok")
else:
  print ("negative result request: not ok")

sock.sendto(b"\x01\x07(* 4 5)",addr)

try:
    data, _= sock.recvfrom(1024)

except socket.timeout:
    print("REQUEST TIMED OUT")

if data == b"\x01\x01\x0cWrong opcode":
  print("malformed opcode request: ok")
else:
  print ("malformed opcode request: not ok")

server.send_signal(signal.SIGINT)
