"use strict";
(function() {
    var patcherResolve, patcherReject;
    var patcherLoaded = new Promise((resolve, reject) => {
        patcherResolve = resolve;
        patcherReject = reject;
    })

    var goPatcher = {
        patch: function () { alert("Error loading patcher."); },
        unpackOrig: function () { alert("Error loading patcher."); },
        checkOrig: function () { alert("Error loading patcher."); },
        getRugburnVersion: function () { alert("Error loading patcher."); },
        getEmbeddedVersion: function () { alert("Error loading patcher."); },
    }

    function deferred(fnName) {
        return function() {
            var args = arguments;
            loadPatcher();
            return patcherLoaded.then(function() {
                return goPatcher[fnName].apply(this, args);
            }).catch(function(error) {
                alert("Patcher did not load successfully: " + String(error));
            });
        }
    }

    var patcher = {
        patch: deferred("patch"),
        unpackOrig: deferred("unpackOrig"),
        checkOrig: deferred("checkOrig"),
        getRugburnVersion: deferred("getRugburnVersion"),
        getEmbeddedVersion: deferred("getEmbeddedVersion"),
    };

    window["patcherLoaded"] = function (patch, unpackOrig, checkOrig, getRugburnVersion, getEmbeddedVersion) {
        goPatcher.patch = patch;
        goPatcher.unpackOrig = unpackOrig;
        goPatcher.checkOrig = checkOrig;
        goPatcher.getRugburnVersion = getRugburnVersion;
        goPatcher.getEmbeddedVersion = getEmbeddedVersion;
        patcherResolve();
    };

    function logToScreen(text) {
        console.log("PATCHER LOG:", text);
        var div = document.createElement("div");
        div.textContent = text;
        document.getElementById("log-output").appendChild(div);
    }

    function loadPatcher() {
        if (window["startedLoadingPatcher"]) {
            return;
        }

        logToScreen("Loading patcher...");

        window["startedLoadingPatcher"] = true;

        WebAssembly.instantiateStreaming(fetch("patcher.wasm"), go.importObject)
            .then(result => {
                go.run(result.instance);
            }).catch(err => {
                alert("An error occurred while loading the WASM bundle: "+err);
                patcherReject(err);
            });
    }

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
            logToScreen(line);
        }
    }

    document.querySelector("#file-input").addEventListener("change", function() {
        var reader = new FileReader();
        reader.onload = function() {
            var input = new Uint8Array(this.result);
            patcher.unpackOrig(input).then(orig => {
                Promise.all([
                    patcher.getEmbeddedVersion(),
                    patcher.getRugburnVersion(input),
                    patcher.checkOrig(orig)
                ]).then(([embedded, inputVer, haveOrig]) => {
                    const confirmOrig = haveOrig || confirm("This does not appear to contain an original ijl15.dll. The rugburn patcher may not work correctly with this file. Proceed?");
                    const confirmReplace = inputVer === "original" || confirm("Current rugburn version: " + inputVer + "; replace with " + embedded + "?");
                    if (confirmOrig && confirmReplace) {
                        patcher.patch(orig, log).then(output => {
                            downloadBlob(output, "ijl15.dll", "application/octet-stream")
                            alert("Success");
                        }).catch(error => {
                            alert("Patching failed: " + error);
                        });
                    } else {
                        logToScreen("Aborted.");
                    }
                });
            }).catch(error => {
                alert("Parsing failed: " + error);
            })
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

    function updateNav(hash) {
        let setSelected = false;
        document.querySelectorAll(".nav a").forEach(link => {
            const linkHash = new URL(link).hash;
            if (linkHash == hash) {
                setSelected = true;
                link.classList.add("selected");
            } else {
                link.classList.remove("selected");
            }
        });
        if (!setSelected) {
            document.querySelectorAll(".nav a").forEach(link => {
                const linkHash = new URL(link).hash;
                if (linkHash == "#home") {
                    link.classList.add("selected");
                }
            });
        }
    }

    logToScreen("Patcher log output will appear here.");

    window.addEventListener("hashchange", event => {
        updateNav(new URL(event.newURL).hash);
    });
    updateNav(location.hash);
})();
