package pe

import "io"

type CountedWriter struct {
	Writer       io.Writer
	BytesWritten int
}

func NewCountedWriter(w io.Writer) *CountedWriter {
	return &CountedWriter{
		Writer:       w,
		BytesWritten: 0,
	}
}

func (w *CountedWriter) Write(b []byte) (int, error) {
	n, err := w.Writer.Write(b)
	w.BytesWritten += n
	return n, err
}
