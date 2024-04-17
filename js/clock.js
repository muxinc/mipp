const PI = 3.141592653589793238462643383279502884197169399375105820974944592307816406286;
function clock(time) {
    const now = new Date();
    const ctx = new VideoFrame(400, 400, 0)
    ctx.save();
    ctx.translate(200, 200);
    ctx.scale(1, 1);
    ctx.rotate(-PI / 2);
    ctx.strokeStyle = "rgb(255,255,255)";//"#ffffff";
    ctx.fillStyle = "#ffffff";
    ctx.lineWidth = 8;
    ctx.lineCap = "round";

    // Hour marks
    ctx.save();
    for (let i = 0; i < 12; i++) {
        ctx.beginPath();
        ctx.rotate(PI / 6);
        ctx.moveTo(100, 0);
        ctx.lineTo(120, 0);
        ctx.stroke();
    }
    ctx.restore();

    // Minute marks
    ctx.save();
    ctx.lineWidth = 5;
    for (let i = 0; i < 60; i++) {
        if (i % 5 !== 0) {
            ctx.beginPath();
            ctx.moveTo(117, 0);
            ctx.lineTo(120, 0);
            ctx.stroke();
        }
        ctx.rotate(PI / 30);
    }
    ctx.restore();

    // const sec = now.getSeconds();
    // const min = now.getMinutes();
    // const hr = now.getHours() % 12;

    const sec = time;
    const min = 0;
    const hr = 0;
    ctx.fillStyle = "#000000";

    // Write Hours
    ctx.save();
    ctx.rotate(
        (PI / 6) * hr + (PI / 360) * min + (PI / 21600) * sec,
    );
    ctx.lineWidth = 14;
    ctx.beginPath();
    ctx.moveTo(-20, 0);
    ctx.lineTo(80, 0);
    ctx.stroke();
    ctx.restore();

    // Write Minutes
    ctx.save();
    ctx.rotate((PI / 30) * min + (PI / 1800) * sec);
    ctx.lineWidth = 10;
    ctx.beginPath();
    ctx.moveTo(-28, 0);
    ctx.lineTo(112, 0);
    ctx.stroke();
    ctx.restore();

    // Write seconds
    ctx.save();
    ctx.rotate((sec * PI) / 30);
    ctx.strokeStyle = "red";
    ctx.fillStyle = "red";
    ctx.lineWidth = 6;
    ctx.beginPath();
    ctx.moveTo(-30, 0);
    ctx.lineTo(83, 0);
    ctx.stroke();
    ctx.beginPath();
    ctx.arc(0, 0, 10, 0, PI * 2, true);
    ctx.fill();
    ctx.beginPath();
    ctx.arc(95, 0, 10, 0, PI * 2, true);
    ctx.stroke();
    ctx.fillStyle = "#000000";
    ctx.arc(0, 0, 3, 0, PI * 2, true);
    ctx.fill();
    ctx.restore();

    ctx.beginPath();
    ctx.lineWidth = 14;
    ctx.strokeStyle = "#325FA2";
    ctx.arc(0, 0, 142, 0, PI * 2, true);
    ctx.stroke();

    ctx.restore();

    return ctx;
}


function receive_video_frame(frame, pad) {
    c = clock(frame.pts)

    frame.font = "100px Arial";
    frame.fillStyle = "#00000080";
    frame.fillRect(150, 150, 1300, 580);
    frame.moveTo(600, 700);
    frame.strokeText("PTS: " + frame.pts)
    frame.draw(c, 100, 100, 640, 640)
    send_video_frame(frame);
}
