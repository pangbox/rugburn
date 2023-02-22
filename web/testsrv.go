package main

import (
	"embed"
	"flag"
	"io/fs"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/fsnotify/fsnotify"
)

//go:embed dist
var distFiles embed.FS

func watch(dir string) {
	watcher, err := fsnotify.NewWatcher()
	if err != nil {
		log.Fatal(err)
	}

	go func() {
		for {
			select {
			case event, ok := <-watcher.Events:
				if !ok {
					return
				}
				if !event.Has(fsnotify.Write) {
					continue
				}
				goFile := strings.HasSuffix(event.Name, ".go")
				jsFile := strings.HasSuffix(event.Name, ".js")
				htmlFile := strings.HasSuffix(event.Name, ".html")
				if goFile || jsFile || htmlFile {
					log.Printf("%s changed. Exiting...", event.Name)
					time.Sleep(time.Second / 2)
					os.Exit(0)
				}
			case err, ok := <-watcher.Errors:
				if !ok {
					return
				}
				log.Println("error:", err)
			}
		}
	}()

	log.Printf("Watching directory %s for changes.", dir)
	err = filepath.Walk(dir, func(walkPath string, fi os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		if fi.IsDir() {
			if err = watcher.Add(walkPath); err != nil {
				return err
			}
		}
		return nil
	})
	if err != nil {
		log.Fatal(err)
	}
}

func main() {
	listen := flag.String("listen", ":1234", "address to listen on")
	watchdir := flag.String("watch", "", "directory to watch (exits on changes, run in a loop)")
	flag.Parse()

	if *watchdir != "" {
		watch(*watchdir)
	}

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
