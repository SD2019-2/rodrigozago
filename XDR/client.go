package main

// Import packages
import (
	"os"
	"log"
	"net"
	"strconv"
	"strings"
	"bytes"
	"bufio"
	"github.com/stellar/go/xdr"
)

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

	for 1 < 2 {
		if textInput == "q" {
			log.Println(textInput)
			break;
		}
		// var h = "Hello World"
		// Use marshal to automatically determine the appropriate underlying XDR
		// types and encode.
		var w bytes.Buffer
		_, err := xdr.Marshal(&w, &textInput)
		
		log.Print(w);

		// Continues only if has no errors
		if err != nil {
			log.Println(err)
			return
		}
	
		encodedData := w.Bytes()
		// log.Printf("bytes written: %s", bytesWritten)
		// log.Printf("encoded data: %s", encodedData)
	
	
		conn.Write([]byte(encodedData))
		log.Printf("Send: %s", textInput)

		// Read input from terminal
		textInput, _ = reader.ReadString('\n')
	}

}

func main() {

	var (
		ip   = "127.0.0.1"
		port = 4649
	)

	SocketClient(ip, port)

}