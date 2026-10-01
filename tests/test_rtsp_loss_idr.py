import os
import socket
import struct
import subprocess
import sys
import time
from test_rtsp_profile import Messages, send


def main():
    with socket.socket() as listener:
        listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        listener.bind(("127.0.0.1", 7236))
        listener.listen(1)
        listener.settimeout(5)
        env = os.environ.copy()
        env.update(WFD_LOSS_IDR="1", WFD_VIDEO_ASYNC="1",
                   WFD_AUDIO_ENABLE="1", WFD_AUDIODEV="null")
        process = subprocess.Popen([sys.argv[1], "127.0.0.1"], env=env,
                                   stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
        try:
            peer, _ = listener.accept()
            with peer, socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp:
                peer.settimeout(3)
                messages = Messages(peer)
                send(peer, b"OPTIONS * RTSP/1.0", 0)
                assert messages.read()[0] == b"RTSP/1.0 200 OK"
                assert messages.read()[0].startswith(b"OPTIONS ")
                send(peer, b"RTSP/1.0 200 OK", 1)
                send(peer, b"GET_PARAMETER rtsp://localhost/wfd1.0 RTSP/1.0", 1,
                     b"wfd_video_formats\r\nwfd_audio_codecs\r\n")
                first, _, body = messages.read()
                assert first == b"RTSP/1.0 200 OK"
                assert b"wfd_video_formats: 0a 00 02 04" in body
                send(peer, b"SET_PARAMETER rtsp://localhost/wfd1.0 RTSP/1.0", 2,
                     b"wfd_video_formats: 0a 00 02 04 00000000 00000000 00000002 00 0000 0000 00 none none\r\n")
                assert messages.read()[0] == b"RTSP/1.0 200 OK"
                send(peer, b"SET_PARAMETER rtsp://localhost/wfd1.0 RTSP/1.0", 3,
                     b"wfd_trigger_method: SETUP\r\n")
                assert messages.read()[0] == b"RTSP/1.0 200 OK"
                assert messages.read()[0].startswith(b"SETUP ")
                send(peer, b"RTSP/1.0 200 OK", 5, extra=b"Session: mock-session\r\n")
                assert messages.read()[0].startswith(b"PLAY ")
                send(peer, b"RTSP/1.0 200 OK", 6)

                def packet(sequence):
                    header = struct.pack(">BBHII", 0x80, 33, sequence, 1000, 1)
                    udp.sendto(header + b"\x47\x10\x11\x10" + b"x" * 184,
                               ("127.0.0.1", 1028))

                def idr(expected):
                    first, fields, body = messages.read()
                    assert first.startswith(b"SET_PARAMETER ")
                    assert body == b"wfd_idr_request\r\n"
                    assert fields[b"cseq"] == str(expected).encode()
                    send(peer, b"RTSP/1.0 200 OK", expected)

                for sequence in range(16):
                    packet(sequence)
                idr(103)
                time.sleep(1.1)
                packet(17)
                idr(104)
                packet(19)
                peer.settimeout(0.15)
                try:
                    data = peer.recv(1)
                except socket.timeout:
                    data = None
                assert data is None, "recovery request ignored cooldown"
                peer.settimeout(3)
                time.sleep(1.1)
                packet(20)
                idr(105)
                packet(20)
                packet(19)
                time.sleep(1.1)
                packet(21)
                peer.settimeout(0.15)
                try:
                    data = peer.recv(1)
                except socket.timeout:
                    data = None
                assert data is None, "duplicate/reorder spuriously requested IDR"
                process.terminate()
                _, stderr = process.communicate(timeout=5)
                assert process.returncode == 0, stderr.decode(errors="replace")
                assert b"idr_requests=2" in stderr
            print("Mock800 RTP gap/source200/cooldown/duplicate/reorder IDR recovery passed")
        finally:
            if process.poll() is None:
                process.kill()
                process.communicate()


if __name__ == "__main__":
    main()
