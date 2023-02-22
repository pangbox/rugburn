package main

import (
	"bytes"
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

var autoreloadScript = []byte(`<script>;(function(){function retry(){fetch("/__ready").then(function(){window.location.reload()}).catch(function(){setTimeout(retry,1000)})}function check(){fetch("/__wait").then(check).catch(retry)}check();})();</script>`)

type autoreloadInjectorFS struct {
	next fs.FS
}

func (f autoreloadInjectorFS) Open(name string) (fs.File, error) {
	if strings.HasSuffix(name, ".html") {
		data, err := fs.ReadFile(f.next, name)
		if err != nil {
			return nil, err
		}
		file, err := f.next.Open(name)
		if err != nil {
			return nil, err
		}
		defer file.Close()
		stat, err := file.Stat()
		if err != nil {
			return nil, err
		}
		data = append(data, autoreloadScript...)
		name := stat.Name()
		size := int64(len(data))
		mode := stat.Mode()
		modTime := stat.ModTime()
		return &memoryFile{name, size, mode, modTime, *bytes.NewReader(data)}, nil
	}
	return f.next.Open(name)
}

type memoryFile struct {
	name    string
	size    int64
	mode    fs.FileMode
	modTime time.Time
	bytes.Reader
}

func (f memoryFile) Name() string               { return f.name }
func (f memoryFile) Size() int64                { return f.size }
func (f memoryFile) Mode() fs.FileMode          { return f.mode }
func (f memoryFile) ModTime() time.Time         { return f.modTime }
func (memoryFile) IsDir() bool                  { return false }
func (memoryFile) Sys() any                     { return nil }
func (f memoryFile) Stat() (fs.FileInfo, error) { return f, nil }
func (memoryFile) Close() error                 { return nil }

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
				cssFile := strings.HasSuffix(event.Name, ".css")
				htmlFile := strings.HasSuffix(event.Name, ".html")
				if goFile || jsFile || cssFile || htmlFile {
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
	fs := http.FileServer(http.FS(autoreloadInjectorFS{dist}))
	http.Handle("/", fs)
	http.Handle("/__wait", http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) { time.Sleep(time.Minute) }))
	http.Handle("/__ready", http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {}))
	log.Printf("Listening on %s", *listen)
	log.Fatal(http.ListenAndServe(*listen, nil))
}
