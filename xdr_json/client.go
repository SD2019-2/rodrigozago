package main

// Import packages
import (
	"os"
	"log"
	"net"
	"strconv"
	"strings"
	"bufio"
	"encoding/json"
)


type LogMessage struct {
	Log string
}

func SocketClient(ip string, port int) {

	// Create i/o reader
	reader := bufio.NewReader(os.Stdin)
	
	// Create address string ex ip:port
	addr := strings.Join([]string{ip, strconv.Itoa(port)}, ":")
	
	// The Dial function connects to a server
	conn, err := net.Dial("tcp", addr)

	// Continues only if has no errors
	if err != nil {
		log.Fatalln(err)
		os.Exit(1)
	}
	
	// A defer statement defers the execution of a function until the surrounding function returns.
	// Executed when func SocketClient has ben done
	defer conn.Close()

	// Read input from terminal
	var textInput, _ = reader.ReadString('\n')

	for textInput != "q" {
		aLogMessage := LogMessage{
			Log: textInput,
		}

		res, err := json.Marshal(aLogMessage)

		if err != nil {
			log.Println("Erro")
		}


		conn.Write([]byte(string(res)))
		log.Printf("Send: %s", string(res))

		// Read input from terminal
		textInput, _ = reader.ReadString('\n')
	}

}

func main() {

	var (
		ip   = "104.198.50.212"
		port = 8080
	)

	SocketClient(ip, port)

}