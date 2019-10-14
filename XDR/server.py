import socket
import xdrlib

HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
PORT = 4649         # Port to listen on

# Use socket with name s
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    # Bind the socket to ip and port
    s.bind((HOST, PORT))
    # Configure as a server socket, max 10 connections
    s.listen(10)
    while True:
        # Start accepting connections
        conn, addr = s.accept()
        print('Connected by', addr)
        while True:
            data = conn.recv(1024)
            unpacker = xdrlib.Unpacker(data)
            if not data:
                break
            print('Recived data: ')
            print(data)
            print('Unpacked data: ' + unpacker.unpack_string().decode('utf-8') + '\n a')
