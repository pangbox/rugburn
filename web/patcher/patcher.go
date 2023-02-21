//go:build js
// +build js

package main

import (
	"bufio"
	"io"
	"log"
	"syscall/js"

	"github.com/pangbox/rugburn/out"
	"github.com/pangbox/rugburn/slipstrm/patcher"
)

func main() {
	c := make(chan struct{}, 0)
	js.Global().Call("patcherLoaded", js.FuncOf(Patch))
	<-c
}

// JS arguments:
// - input: Uint8Array
// - logCallback: (error: Error, line: string) => void
// JS return value:
// - Promise<Uint8Array>
func Patch(this js.Value, p []js.Value) interface{} {
	jsinput := p[0]
	logcb := p[1]

	handler := js.FuncOf(func(this js.Value, args []js.Value) interface{} {
		resolve := args[0]
		reject := args[1]

		go func() {
			// Setup logging.
			logr, logw := io.Pipe()
			go func() {
				scanner := bufio.NewScanner(logr)
				for scanner.Scan() {
					logcb.Invoke(nil, scanner.Text())
				}
				if err := scanner.Err(); err != nil {
					logcb.Invoke(err.Error(), nil)
				}
			}()
			logger := log.New(logw, "", log.LstdFlags)

			input := make([]byte, jsinput.Get("length").Int())
			js.CopyBytesToGo(input, jsinput)

			output, err := patcher.Patch(logger, input, out.RugburnDLL)

			if err != nil {

			}

			jsoutput := js.Global().Get("Uint8Array").New(len(output))
			js.CopyBytesToJS(jsoutput, output)

			if err != nil {
				errorConstructor := js.Global().Get("Error")
				errorObject := errorConstructor.New(err.Error())
				reject.Invoke(errorObject)
			} else {
				resolve.Invoke(js.ValueOf(jsoutput))
			}
		}()

		return nil
	})

	promiseConstructor := js.Global().Get("Promise")
	return promiseConstructor.New(handler)
}
