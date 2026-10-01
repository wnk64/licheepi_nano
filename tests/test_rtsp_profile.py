#!/usr/bin/env python3
import socket
import subprocess
import sys


class Messages:
    def __init__(self, peer):
        self.peer = peer
        self.pending = bytearray()

    def read(self):
        while b"\r\n\r\n" not in self.pending:
            data = self.peer.recv(4096)
            assert data, "peer closed before header"
            self.pending.extend(data)
        header, _, _ = bytes(self.pending).partition(b"\r\n\r\n")
        fields = {}
        for line in header.split(b"\r\n")[1:]:
            name, separator, value = line.partition(b":")
            if separator:
                fields[name.lower()] = value.strip()
        length = int(fields.get(b"content-length", b"0"))
        end = len(header) + 4 + length
        while len(self.pending) < end:
            data = self.peer.recv(4096)
            assert data, "peer closed before body"
            self.pending.extend(data)
        body = bytes(self.pending[len(header) + 4:end])
        del self.pending[:end]
        return header.split(b"\r\n")[0], fields, body


def send(peer, first, cseq, body=b"", extra=b""):
    header = first + b"\r\nCSeq: " + str(cseq).encode() + b"\r\n" + extra
    if body:
        header += b"Content-Type: text/parameters\r\nContent-Length: "
        header += str(len(body)).encode() + b"\r\n"
    peer.sendall(header + b"\r\n" + body)


def main():
    expected = (b"00", b"00000000") if sys.argv[2] == "640" else (b"0a", b"00000002")
    with socket.socket() as listener:
        listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        listener.bind(("127.0.0.1", 7236))
        listener.listen(1)
        listener.settimeout(5)
        process = subprocess.Popen([sys.argv[1], "127.0.0.1"],
                                   stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
        try:
            peer, _ = listener.accept()
            with peer:
                peer.settimeout(5)
                messages = Messages(peer)
                send(peer, b"OPTIONS * RTSP/1.0", 0)
                assert messages.read()[0] == b"RTSP/1.0 200 OK"
                assert messages.read()[0].startswith(b"OPTIONS ")
                send(peer, b"RTSP/1.0 200 OK", 1)
                send(peer, b"GET_PARAMETER rtsp://localhost/wfd1.0 RTSP/1.0", 1,
                     b"wfd_video_formats\r\nwfd_audio_codecs\r\n")
                first, fields, body = messages.read()
                assert first == b"RTSP/1.0 200 OK" and fields[b"cseq"] == b"1"
                parameters = dict(line.split(b": ", 1) for line in body.split(b"\r\n") if line)
                video = parameters[b"wfd_video_formats"].split()
                assert len(video) == 13 and video[0] == expected[0]
                assert video[2:4] == [b"02", b"04"]
                assert video[4:7] == [b"00000001", b"00000000", expected[1]]
                assert parameters[b"wfd_audio_codecs"] == b"LPCM 00000002 00"
                selected = b"wfd_video_formats: " + parameters[b"wfd_video_formats"] + b"\r\n"
                send(peer, b"SET_PARAMETER rtsp://localhost/wfd1.0 RTSP/1.0", 2, selected)
                assert messages.read()[0] == b"RTSP/1.0 200 OK"
                send(peer, b"SET_PARAMETER rtsp://localhost/wfd1.0 RTSP/1.0", 3,
                     b"wfd_trigger_method: SETUP\r\n")
                assert messages.read()[0] == b"RTSP/1.0 200 OK"
                assert messages.read()[0].startswith(b"SETUP ")
                send(peer, b"RTSP/1.0 200 OK", 5, extra=b"Session: mock-session\r\n")
                assert messages.read()[0].startswith(b"PLAY ")
                send(peer, b"RTSP/1.0 200 OK", 6)
                process.terminate()
                _, stderr = process.communicate(timeout=5)
                assert process.returncode == 0, stderr.decode(errors="replace")
            print("Mock M1-M7 profile %s passed; no real-phone or video acceptance" % sys.argv[2])
        finally:
            if process.poll() is None:
                process.kill()
                process.communicate()


if __name__ == "__main__":
    main()
