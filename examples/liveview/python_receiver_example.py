"""ZeroMQ subscriber that receives JPEG frames published by ImageProcessorPythonStream."""

import cv2
import numpy as np
import zmq


def main() -> None:
    endpoint = "tcp://127.0.0.1:5555"

    context = zmq.Context.instance()
    socket = context.socket(zmq.SUB)
    socket.connect(endpoint)
    socket.setsockopt(zmq.SUBSCRIBE, b"")

    try:
        while True:
            header = socket.recv()
            if len(header) != 4:
                continue
            frame_size = int.from_bytes(header, byteorder="little", signed=False)
            payload = socket.recv()
            if len(payload) != frame_size:
                continue

            buffer = np.frombuffer(payload, dtype=np.uint8)
            frame = cv2.imdecode(buffer, cv2.IMREAD_COLOR)
            if frame is None:
                continue

            cv2.imshow("Liveview from Edge-SDK", frame)
            if cv2.waitKey(1) == 27:
                break
    finally:
        cv2.destroyAllWindows()
        socket.close()

if __name__ == "__main__":
    main()
