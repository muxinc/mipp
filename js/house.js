function drawEllipse(frame, x, y, w, h) {

    var kappa = .55228483
    ox = (w / 2) * kappa, // control point offset horizontal
        oy = (h / 2) * kappa, // control point offset vertical
        xe = x + w,           // x-end
        ye = y + h,           // y-end
        xm = x + w / 2,       // x-middle
        ym = y + h / 2;       // y-middle

    frame.beginPath();
    frame.moveTo(x, ym);
    frame.bezierCurveTo(x, ym - oy, xm - ox, y, xm, y);
    frame.bezierCurveTo(xm + ox, y, xe, ym - oy, xe, ym);
    frame.bezierCurveTo(xe, ym + oy, xm + ox, ye, xm, ye);
    frame.bezierCurveTo(xm - ox, ye, x, ym + oy, x, ym);
    //frame.closePath(); // not used correctly, see comments (use to close off open path)

    frame.fill();
    frame.stroke();
}

function receive_frame(frame) {
    // frame.rotate(ts / 100);
    // frame.fillStyle = "#975B5B";
    // frame.strokeStyle = "#ffffff";
    // frame.lineWidth = 3;

    frame.beginPath();
    frame.moveTo(100, 260);
    frame.lineTo(300, 10);
    frame.lineTo(500, 260);
    frame.closePath();
    frame.fill();
    frame.stroke();
    frame.flush();


    frame.fillRect(381, 60, 45, 120);
    frame.strokeRect(381, 60, 45, 140);
    // drawEllipse(frame. 380, 55, 47, 14);
    frame.fillRect(378, 198, 55, 5);

    // // house walls
    frame.fillRect(100, 260, 400, 300);
    frame.strokeRect(100, 260, 400, 300);

    // // windows
    // frame.fillStyle = "white";
    frame.fillRect(130, 300, 70, 45);
    frame.fillRect(205, 300, 70, 45);
    frame.fillRect(325, 300, 70, 45);
    frame.fillRect(400, 300, 70, 45);
    frame.fillRect(130, 350, 70, 45);
    frame.fillRect(205, 350, 70, 45);
    frame.fillRect(325, 350, 70, 45);
    frame.fillRect(400, 350, 70, 45);
    frame.fillRect(325, 425, 70, 45);
    frame.fillRect(400, 425, 70, 45);
    frame.fillRect(325, 475, 70, 45);
    frame.fillRect(400, 475, 70, 45);

    send_frame(frame);

    // frame.beginPath();
    // frame.moveTo(200, 423);
    // frame.lineTo(200, 560);
    // frame.moveTo(140, 433);
    // frame.lineTo(140, 560);
    // frame.moveTo(260, 434);
    // frame.lineTo(260, 560);
    // frame.stroke();
    // frame.beginPath();
    // frame.arc(199, 562, 140, 1.355 * Math.PI, 1.65 * Math.PI); // door arc
    // frame.stroke();

    // // door handles
    // frame.beginPath();
    // frame.arc(185, 510, 5, 0, 2 * Math.PI);
    // frame.stroke();
    // frame.beginPath();
    // frame.arc(215, 510, 5, 0, 2 * Math.PI);
    // frame.stroke();

    // frame.font = "20px fantasy"
    // frame.fillStyle = "HotPink";
    // frame.fillText("X" + name, 100, 100);
}
