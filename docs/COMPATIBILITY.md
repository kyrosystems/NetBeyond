# Compatibility

| Feature | XP | Vista | Windows 7 | Status |
|---|---|---|---|---|
| Native C parser/layout | source target | source target | source target | unit tested on CI host only |
| WinHTTP HTTP/HTTPS transport | source target | source target | source target | certificate validation enabled; not run on target OS |
| Modern TLS | unverified | unverified | unverified | system SChannel capability decides |
| IE/ActiveX renderer | no | no | no | no longer part of planned runtime path |
| CSS/JS/images/forms | no | no | no | not implemented |
| Wasm/WebGL/WebGPU/WebRTC/SW | no | no | no | not implemented |
| Chrome MV2/MV3 | no | no | no | not implemented |
