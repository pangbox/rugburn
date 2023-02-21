"use strict";
(function() {
    var patch = () => { alert("Patcher is not loaded yet."); };
    window["patcherLoaded"] = function (fn) { patch = fn; };

    function downloadURL(data, fileName) {
        var a;
        a = document.createElement('a');
        a.href = data;
        a.download = fileName;
        document.body.appendChild(a);
        a.style = "display: none";
        a.click();
        a.remove();
    };

    function downloadBlob(data, fileName, mimeType) {
        var blob, url;
        blob = new Blob([data], { type: mimeType });
        url = window.URL.createObjectURL(blob);
        downloadURL(url, fileName);
        setTimeout(function() {
            return window.URL.revokeObjectURL(url);
        }, 1000);
    };

    function log(error, line) {
        if (error) {
            alert("An error occurred: " + error);
        } else {
            console.log(line);
        }
    }

    document.querySelector("#file-input").addEventListener("change", function() {
        var reader = new FileReader();
        reader.onload = function() {
            var input = new Uint8Array(this.result);
            patch(input, log).then(output => {
                downloadBlob(output, "ijl15.dll", "application/octet-stream")
                alert("Success");
            }).catch(error => {
                alert("Patching failed: " + error);
            });
        }
        reader.readAsArrayBuffer(this.files[0]);
    }, false);

    const go = new Go();

    go.exit = (code) => {
        alert("WASM bundle unexpectedly exited! (exit code "+code+") - check console for more details");
    };

    // Just in case someone is using older Safari.
    if (!WebAssembly.instantiateStreaming) {
        WebAssembly.instantiateStreaming = async (resp, importObject) => {
            const source = await (await resp).arrayBuffer();
            return await WebAssembly.instantiate(source, importObject);
        };
    }

    WebAssembly.instantiateStreaming(fetch("patcher.wasm"), go.importObject)
        .then(result => {
            go.run(result.instance);
        }).catch(err => {
            alert("An error occurred while loading the WASM bundle: "+err);
        });
})();
