var [x, xdir, y, ydir] = [0, 1, 0, 1];
var [w, h] = [320, 240]
var speed = 5;

make_pads(2)
let overlay = new VideoFrame();
function receive_video_frame(frame, pad) {
    if (pad == 1) {
        overlay = frame
        return 0;
    }

    x += xdir * speed; y += ydir * speed;
    if (x < 0) { x = 0; xdir = 1; }
    if (y < 0) { y = 0; ydir = 1; }
    if (x > frame.width - w) { x = frame.width - w; xdir = -1; }
    if (y > frame.height - h) { y = frame.height - h; ydir = -1; }
    frame.draw(overlay, x, y, w, h)
    send_video_frame(frame);
}
