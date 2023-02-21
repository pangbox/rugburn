package main

import (
	"embed"
	"flag"
	"io/fs"
	"log"
	"net/http"
)

//go:embed dist
var distFiles embed.FS

func main() {
	listen := flag.String("listen", ":1234", "address to listen on")
	flag.Parse()
	var embedFS = fs.FS(distFiles)
	dist, err := fs.Sub(embedFS, "dist")
	if err != nil {
		log.Fatal(err)
	}
	fs := http.FileServer(http.FS(dist))
	http.Handle("/", fs)
	log.Printf("Listening on %s", *listen)
	log.Fatal(http.ListenAndServe(*listen, nil))
}
