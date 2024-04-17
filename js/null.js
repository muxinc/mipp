
function receive_video_frame(frame, pad) {
    log('JS received video frame: ' + frame.width + 'x' + frame.height)
    send_video_frame(frame, 0)
}

// function receive_audio_frame(frame, pad) {
//     log('JS received audio frame: ' + frame.data.length)
//     send_audio_frame(frame, 0)
// }
