
function receive_video_frame(frame) {
    var out = new VideoFrame(frame.width, frame.height, frame.pts)

    for (y = 0; y < out.height / 2; y++) {
        for (x = 0; x < out.width / 2; x++) {
            p2 = (x * 2) + (y * out.width * 2);
            pixel = frame.data[p2];

            out.data[x + (y * out.width)] = pixel
            out.data[(x + out.width / 2) + (y * out.width)] = pixel & 0xff0000ff
            out.data[x + ((y + out.height / 2) * out.width)] = pixel & 0xff00ff00
            out.data[(x + out.width / 2) + ((y + out.height / 2) * out.width)] = pixel & 0xffff0000
        }
    }

    send_video_frame(out)
}
