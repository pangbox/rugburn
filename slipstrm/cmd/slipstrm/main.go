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

	if flag.NArg() != 4 {
		fmt.Fprintf(os.Stderr, "Usage: slipstrm <original ijl15.dll> <rugburn dll> <new ijl15.dll> <version>\n")
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
	version := flag.Arg(3)

	// Load goat.
	goatBin, err := os.ReadFile(ijl15Filename)
	if err != nil {
		log.Fatalf("Failed to read %q: %v", ijl15Filename, err)
	}

	goatBin, err = patcher.UnpackOriginal(goatBin)
	if err != nil {
		log.Fatalf("Failed to parse %q: %v", ijl15Filename, err)
	}

	if !patcher.CheckOriginalData(goatBin) {
		log.Printf("================================================================================")
		log.Printf("WARNING: input ijl15.dll does not appear to be an original copy!")
		log.Printf("Rugburn slipstream does not support patching arbitrary binaries.")
		log.Printf("================================================================================")
	}

	// Load payload.
	payloadBin, err := os.ReadFile(rugburnFilename)
	if err != nil {
		log.Fatalf("Failed to read %q: %v", rugburnFilename, err)
	}

	// Perform patch.
	out, err := patcher.Patch(log.Default(), goatBin, payloadBin, version)
	if err != nil {
		log.Fatalf("Unable to patch: %v", err)
	}

	// Write output.
	if err := os.WriteFile(outputFilename, out, 0644); err != nil {
		log.Fatalf("Unable to write patched DLL to %q: %v", outputFilename, err)
	}
}
