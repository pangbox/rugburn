package main

import (
	"flag"
	"fmt"
	"log"
	"os"
	"time"

	"github.com/pangbox/rugburn/slipstrm/patcher"
)

func main() {
	flag.Parse()

	if flag.NArg() != 3 {
		fmt.Fprintf(os.Stderr, "Usage: slipstrm <original ijl15.dll> <rugburn dll> <new ijl15.dll>\n")
		os.Exit(1)
	}

	start := time.Now()
	defer func() {
		duration := time.Since(start)
		log.Printf("Execution complete. (%s elapsed.)", duration)
	}()

	ijl15Filename := flag.Arg(0)
	rugburnFilename := flag.Arg(1)
	outputFilename := flag.Arg(2)

	// Load goat.
	goatBin, err := os.ReadFile(ijl15Filename)
	if err != nil {
		log.Fatalf("Failed to read %q: %v", ijl15Filename, err)
	}

	// Load payload.
	payloadBin, err := os.ReadFile(rugburnFilename)
	if err != nil {
		log.Fatalf("Failed to read %q: %v", rugburnFilename, err)
	}

	// Perform patch.
	out, err := patcher.Patch(log.Default(), goatBin, payloadBin)
	if err != nil {
		log.Fatalf("Unable to patch: %v", err)
	}

	// Write output.
	if err := os.WriteFile(outputFilename, out, 0644); err != nil {
		log.Fatalf("Unable to write patched DLL to %q: %v", outputFilename, err)
	}
}
