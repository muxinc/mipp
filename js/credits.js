function fade_credit(frame, text, start_time, duration, fade_duration) {
    if (frame.pts < start_time) {
        return;
    }
    if (frame.pts > start_time + duration) {
        return;
    }
    if (fade_duration === undefined || fade_duration > duration / 2) {
        fade_duration = duration / 2;
    }

    let alpha = (frame.pts - start_time) / fade_duration;
    // More that half way through the fade, start fading out
    if (frame.pts > start_time + duration - fade_duration) {
        alpha = 1 - (frame.pts - (start_time + duration - fade_duration)) / fade_duration;
    }
    if (alpha > 1) {
        alpha = 1;
    }

    frame.font = "100px Arial";
    frame.setStrokeStyle(255, 255, 255, 255 * alpha);
    frame.moveTo(200, 200);
    frame.strokeText(text);
    frame.globalAlpha = 1;
}


function receive_video_frame(frame, pad) {
    fade_credit(frame, "Mipp Presents", 2, 5, 1);
    fade_credit(frame, "In association with Mipp", 8, 5, 1);
    fade_credit(frame, "A Mipp film", 14, 5, 1);
    send_video_frame(frame);
}
