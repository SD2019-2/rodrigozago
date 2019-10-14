import socket
import mysql.connector
import json

HOST = '0.0.0.0'
PORT = 8080

mydb = mysql.connector.connect(
  host="localhost",
  user="root",
  passwd="sd-2019-2",
  database="log"
)

def main():
    # Use socket with name s
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        # Bind the socket to ip and port
        s.bind((HOST, PORT))
        # Configure as a server socket
        s.listen(10)
        while True:
            # Start accepting connections
            conn, addr = s.accept()
            print('Conected by', addr)
            while True:
                data = conn.recv(1024)
                parsedData = json.loads(data.decode('utf-8'))
                if not data:
                    break
                print('Saved data: ')
                print("Log: {}\nAddr: {}".format(parsedData['Log'], addr[0]))
                saveLogToDatabase(addr[0], parsedData['Log'])
                



def saveLogToDatabase(address, log): 

    mycursor = mydb.cursor()

    sql = "INSERT INTO log (ip_address, log_message) VALUES (%s, %s)"
    val = (address, log)
    mycursor.execute(sql, val)

    mydb.commit()

if __name__ == "__main__":
    main()