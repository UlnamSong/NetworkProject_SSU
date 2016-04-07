from socket import *

ip = 'localhost'
port = 9090


s = socket(AF_INET, SOCK_STREAM)
s.connect((ip,port))
s.send("\x00\x00\x00\x00\x00\x00\x00\x00\x41\x41\x41\x41")
s.close()
